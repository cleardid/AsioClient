#include "ClientSession.h"
#include "../client/CClient.h"

#include "../../infra/log/Logger.h"

#include <chrono>

ClientSession::ClientSession(boost::asio::io_context &ioc, CClient *client)
    : _socket(ioc), _client(client), _connectTimer(ioc)
{
    _recvNode = std::make_shared<MsgNode>();
}

ClientSession::~ClientSession()
{
    Close();
}

void ClientSession::Connect(const std::string &ip, uint16_t port, const int timeout)
{
    try
    {
        _endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(ip), port);

        // 设置连接超时定时器（5秒）
        _connectTimer.expires_after(std::chrono::seconds(timeout));
        _connectTimer.async_wait(
            std::bind(&ClientSession::OnConnectTimeout, shared_from_this(), std::placeholders::_1));

        _socket.async_connect(_endpoint,
                              std::bind(&ClientSession::OnConnect, shared_from_this(), std::placeholders::_1));
    }
    catch (std::exception &e)
    {
    }
}

void ClientSession::OnConnectTimeout(const boost::system::error_code &ec)
{
    if (!ec) // 定时器未被取消
    {
        // 关闭socket，这将导致async_connect操作失败
        boost::system::error_code ignored_ec;
        _socket.close(ignored_ec);
    }
}

void ClientSession::OnConnect(const boost::system::error_code &ec)
{
    // 取消定时器
    _connectTimer.cancel();

    if (!ec)
    {
        // 连接成功
        LOG_INFO << "Connected to " << _endpoint << std::endl;

        _isConnected = true;
        _isClosed = false;
        DoReadHeader(); // 开始读取

        // 启动心跳检测
        _client->StartHeartbeat();
    }
    else
    {
        Close();
    }
}

void ClientSession::Close()
{
    if (!_isClosed)
    {
        _isClosed = true;
        _isConnected = false;
        boost::system::error_code ec;
        _socket.close(ec);
    }
}

void ClientSession::DoReadHeader()
{
    auto self = shared_from_this();
    _recvNode->Clear();
    boost::asio::async_read(_socket,
                            boost::asio::buffer(_recvNode->GetHeaderData(), MsgNode::GetHeaderSize()),
                            [this, self](boost::system::error_code ec, std::size_t)
                            {
                                if (!ec)
                                {
                                    _recvNode->GetHeader().ToHost();
                                    _recvNode->Allocate(_recvNode->GetHeader().length);
                                    DoReadBody();
                                }
                                else
                                {
                                    Close();
                                }
                            });
}

void ClientSession::DoReadBody()
{
    auto self = shared_from_this();
    size_t len = _recvNode->GetBodyLen();
    if (len == 0)
    {
        _client->OnMessage(_recvNode); // 转发给 Client
        DoReadHeader();
        return;
    }
    boost::asio::async_read(_socket,
                            boost::asio::buffer(_recvNode->GetBody(), len),
                            [this, self](boost::system::error_code ec, std::size_t)
                            {
                                if (!ec)
                                {
                                    _client->OnMessage(_recvNode); // 转发给 Client
                                    DoReadHeader();
                                }
                                else
                                {
                                    Close();
                                }
                            });
}

void ClientSession::Send(const MessageHeader &header, const std::string &body)
{
    if (!_isConnected)
        return;
    // 构建消息节点
    auto node = std::make_shared<MsgNode>(body.size());
    // 设置消息头
    node->GetHeader() = header;
    // 设置消息体长度
    node->GetHeader().length = body.size();
    // 消息体不空，则拷贝消息体
    if (!body.empty())
        memcpy(node->GetBody(), body.data(), body.size());
    // 转为网络字节序
    node->GetHeader().ToNetwork();
    // 构建发送缓冲区
    node->BuildSendBuffer();

    std::lock_guard<std::mutex> lock(_mutex);
    _sendQue.push(node);
    if (_sendQue.size() == 1)
        DoWrite();
}

void ClientSession::DoWrite()
{
    auto self = shared_from_this();
    std::shared_ptr<MsgNode> node = _sendQue.front();
    boost::asio::async_write(_socket,
                             boost::asio::buffer(node->GetSendData(), node->GetSendSize()),
                             [this, self](boost::system::error_code ec, std::size_t)
                             {
                                 if (!ec)
                                 {
                                     std::lock_guard<std::mutex> lock(_mutex);
                                     _sendQue.pop();
                                     if (!_sendQue.empty())
                                         DoWrite();
                                 }
                                 else
                                 {
                                     Close();
                                 }
                             });
}