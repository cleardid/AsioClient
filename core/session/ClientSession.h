#ifndef CLIENTSESSION_H
#define CLIENTSESSION_H

#include <boost/asio.hpp>
#include <memory>
#include <queue>
#include <mutex>
#include <functional>
#include <iostream>

// 复用的头文件 (请确保文件存在)
#include "../message/MsgNode.h"
#include "../protocol/MessageHeader.h"

class CClient; // 前置声明

class ClientSession : public std::enable_shared_from_this<ClientSession>
{
public:
    ClientSession(boost::asio::io_context &ioc, CClient *client);
    ~ClientSession();

    void Connect(const std::string &ip, uint16_t port, const int timeout);
    void Close();
    void Send(const MessageHeader &header, const std::string &body);
    bool IsConnected() const { return _isConnected; }

private:
    void DoReadHeader();
    void DoReadBody();
    void DoWrite();
    void OnConnectTimeout(const boost::system::error_code &ec);
    void OnConnect(const boost::system::error_code &ec);

private:
    boost::asio::ip::tcp::socket _socket;
    boost::asio::ip::tcp::endpoint _endpoint;
    CClient *_client;

    // 超时机制
    boost::asio::steady_timer _connectTimer;

    std::shared_ptr<MsgNode> _recvNode;
    std::queue<std::shared_ptr<MsgNode>> _sendQue;
    std::mutex _mutex;

    bool _isConnected = false;
    bool _isClosed = false;
};

#endif