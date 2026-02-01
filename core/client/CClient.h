#ifndef CCLIENT_H
#define CCLIENT_H

#include "AsioClientExport.h"
#include <boost/asio.hpp>
#include <thread>
#include <memory>
#include <functional>

class ClientSession;
class MsgNode;
class HeartService;
class DBQueryService;
class CommunicationService;

// 回调函数定义：ServiceId, CmdId, Data, Len
using MessageCallback = std::function<void(int, int, const char *, int)>;

class ASIO_API CClient
{
public:
    static CClient &GetInstance();

    void Init();
    void Connect(const std::string &ip, const uint16_t port, const int timeout);
    void Send(const uint16_t serviceId, const uint16_t cmdId, const std::string &data);
    void Close();

    // 启动心跳服务
    void StartHeartbeat();
    // 为 DLL 调用的数据库操作接口
    void ExecuteSql(const std::string &req);
    // 为 DLL 调用的通信接口
    void Login(const std::string &name);
    void SendChat(const std::string &toUser, const std::string &msg);

    // 给 Session 用的
    void OnMessage(std::shared_ptr<MsgNode> msg);

    // 给 DLL 用的
    void SetCallback(MessageCallback cb) { _callback = cb; }
    std::shared_ptr<ClientSession> GetSession() { return _session; }

private:
    CClient();
    ~CClient();
    void RunContext();

    boost::asio::io_context _ioc;
    std::unique_ptr<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> _work;
    std::thread _thread;
    std::shared_ptr<ClientSession> _session;
    MessageCallback _callback;

    // 心跳服务
    std::shared_ptr<HeartService> _heartService;
    // 数据库服务
    std::shared_ptr<DBQueryService> _dbService;
    // 通信服务
    std::shared_ptr<CommunicationService> _chatService;

    // 消息序列
    inline static uint32_t _msgSeq = 0;
};

#endif