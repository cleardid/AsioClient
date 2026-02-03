#include "CClient.h"
#include "../session/ClientSession.h"
#include "../message/MsgNode.h"

#include "../../services/ServiceManager.h"
#include "../../services/HeartService/HeartService.h"
#include "../../services/DBService/DBQueryService.h"
#include "../../services/CommunicationService/CommunicationService.h"

#include "../../infra/log/Logger.h"

#include <chrono>

CClient &CClient::GetInstance()
{
    static CClient instance;
    return instance;
}

CClient::CClient()
{
    // 初始化日志，仅添加文件输出
    std::string logFile = "E:/client.log";
    Logger::GetInstance().AddAppender(std::make_unique<FileAppender>(logFile));
}

CClient::~CClient()
{
    Close();
}

void CClient::Init(const char *logFile)
{
    // 如果已经在执行，则直接返回
    if (_work)
        return;

    // 设置日志路径
    if (logFile)
    {
        // 首先删除所有的日志输出，避免与之前的冲突
        Logger::GetInstance().ClearAppender();
        // 添加新的文件输出
        Logger::GetInstance().AddAppender(std::make_unique<FileAppender>(std::string(logFile)));
    }

    // 确保 ioc 状态正确
    if (_ioc.stopped())
        _ioc.restart();

    // 创建心跳服务
    _heartService = std::make_shared<HeartService>(_ioc);
    // 注册到管理器
    ServiceManager::GetInstance().RegisterService(_heartService);
    // 创建数据库服务
    _dbService = std::make_shared<DBQueryService>();
    ServiceManager::GetInstance().RegisterService(_dbService);
    // 注册通信服务
    _chatService = std::make_shared<CommunicationService>();
    ServiceManager::GetInstance().RegisterService(_chatService);

    // 启动IO上下文
    _work = std::make_unique<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>(
        boost::asio::make_work_guard(_ioc));
    _thread = std::thread(&CClient::RunContext, this);

    LOG_INFO << "Client init success" << std::endl;
}

void CClient::RunContext()
{
    _ioc.run();
}

void CClient::Connect(const std::string &ip, const uint16_t port, const int timeout, const char *logFile)
{
    // 初始化
    if (!_work)
        Init(logFile);

    // 先关闭旧的心跳，防止上次连接遗留的定时器还在跑
    if (_heartService)
        _heartService->Stop();

    // 关闭旧会话
    if (_session)
        _session->Close();

    // 创建新会话
    _session = std::make_shared<ClientSession>(_ioc, this);
    _session->Connect(ip, port, timeout);

    LOG_INFO << "Client connect to " << ip << ":" << port << std::endl;
}

void CClient::Send(const uint16_t serviceId, const uint16_t cmdId, const std::string &data)
{
    if (_session && _session->IsConnected())
    {
        MessageHeader header;
        header.serviceId = serviceId;
        header.cmdId = cmdId;
        header.seq = ++_msgSeq;
        _session->Send(header, data);
    }
}

void CClient::Close()
{
    // 关闭心跳服务
    if (_heartService)
    {
        _heartService->Stop();
        _heartService.reset();
    }
    // 关闭数据库服务
    if (_dbService)
    {
        _dbService.reset();
    }
    // 关闭通信服务
    if (_chatService)
    {
        _chatService.reset();
    }

    if (_work)
    {
        _work->reset();
        _ioc.stop();
        if (_thread.joinable())
            _thread.join();
        _work = nullptr;
    }
    if (_session)
    {
        _session->Close();
        _session = nullptr;
    }

    LOG_INFO << "Client close." << std::endl;
}

// 启动心跳服务
void CClient::StartHeartbeat()
{
    // 如果心跳服务存在，则启动
    if (_heartService)
        _heartService->Start();
}

void CClient::ExecuteSql(const std::string &req)
{
    if (_dbService)
    {
        _dbService->ExecuteQuery(req);
    }
    else
    {
        LOG_ERROR << "DBService not init" << std::endl;
    }
}

void CClient::Login(const std::string &name)
{
    if (_chatService)
    {
        _chatService->Login(name);
    }
    else
    {
        LOG_ERROR << "CommunicationService not init" << std::endl;
    }
}
void CClient::SendChat(const std::string &toUser, const std::string &msg)
{
    if (_chatService)
    {
        _chatService->SendMsg(toUser, msg);
    }
    else
    {
        LOG_ERROR << "CommunicationService not init" << std::endl;
    }
}

void CClient::GetOnLineUser()
{
    if (_chatService)
    {
        _chatService->GetOnLineUser();
    }
    else
    {
        LOG_ERROR << "CommunicationService not init" << std::endl;
    }
}

void CClient::OnMessage(std::shared_ptr<MsgNode> msg)
{
    // 优先交给 ServiceManager 处理
    bool handled = ServiceManager::GetInstance().Dispatch(msg);

    // 如果内部服务处理了 (比如心跳)，就直接返回，不打扰 Unity
    if (handled)
        return;

    if (_callback)
    {
        _callback(msg->GetHeader().serviceId, msg->GetHeader().cmdId, msg->GetBody(), msg->GetBodyLen());
    }
}