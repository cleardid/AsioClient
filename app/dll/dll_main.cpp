#include "../../core/client/AsioClientExport.h"
#include "../../core/client/CClient.h"
#include "../../core/session/ClientSession.h"
#include <iostream>
#include <string>
#include <cstring>

#include "../../infra/log/Logger.h"

// 用于 Unity/TestApp 调用
typedef void (*ExternalCallbackType)(int, int, const char *, int);

// 保存外部传入的函数指针
static ExternalCallbackType g_ExternalCallback = nullptr;

// 这是一个中间转换函数：CClient(C++) -> InternalCallback -> g_ExternalCallback(C#)
void InternalCallback(int serviceId, int cmdId, const char *data, int len)
{
    // 检查外部指针是否有效
    if (g_ExternalCallback)
    {
        try
        {
            // 调用外部的 C 函数指针
            g_ExternalCallback(serviceId, cmdId, data, len);
        }
        catch (...)
        {
            // 吞掉所有异常，防止崩坏 C++ 核心
            LOG_ERROR << "InternalCallback Exception" << std::endl;
        }
    }
}

/**
 * @brief 初始化客户端
 * @param logFile 日志文件路径，必须传入有效路径
 * @return void
 * @note 初始化客户端，设置日志文件路径，内部会进行判断日志路径是否合理，如果不合理则会使用默认路径。
 */
ASIO_C_API void Init(const char *logFile)
{
    try
    {
        // 获取单例并初始化线程
        auto &client = CClient::GetInstance();
        client.Init(logFile);

        // 设置内部回调，当 CClient 收到消息时，会调用 InternalCallback
        client.SetCallback(InternalCallback);

        LOG_INFO << "[DLL] Client Initialized." << std::endl;
    }
    catch (std::exception &e)
    {
        LOG_ERROR << "[DLL] Init Exception: " << e.what() << std::endl;
    }
}

/**
 * @brief 连接服务器
 * @param ip 服务器 IP
 * @param port 服务器端口
 * @param timeout 超时时间 （秒）
 * @param logFile 日志文件路径，与 Init 一致必须传递有效路径 ？？ 似乎不需要？每次启动都会执行 Init 方法?
 * @return true: 成功，false: 失败
 * @note 内部处理为使用超时机制，尝试连接，外部需通过对外接口 IsConnected 持续进行检测，例如 Unity 使用协程在规定时间内进行判断
 */
ASIO_C_API bool Connect(const char *ip, int port, int timeout, const char *logFile)
{
    if (ip == nullptr)
        return false;

    try
    {
        // 获取连接信息
        CClient::GetInstance().Connect(ip, static_cast<uint16_t>(port), timeout, logFile);

        return true;
    }
    catch (std::exception &e)
    {
        LOG_ERROR << "[DLL] Connect Exception: " << e.what() << std::endl;
        return false;
    }

    return false;
}

/**
 * @brief 关闭客户端
 * @return void
 * @note 关闭客户端，释放资源。用于 Unity 关闭时调用
 */
ASIO_C_API void Close()
{
    try
    {
        CClient::GetInstance().Close();
        LOG_INFO << "[DLL] Client Closed." << std::endl;
    }
    catch (...)
    {
        // 忽略关闭时的错误
    }
}

/**
 * @brief 设置消息回调
 * @param callback C# 端的委托 (delegate)
 * @return void
 * @note 设置消息回调，当 CClient 收到消息时，会调用 g_ExternalCallback，实际中间还需要一层进行转换，无法直接调用
 */
ASIO_C_API void SetOnMessageCallback(ExternalCallbackType callback)
{
    g_ExternalCallback = callback;
    // 同时更新 CClient 中的回调指向
    CClient::GetInstance().SetCallback(InternalCallback);
}

/**
 * @brief 发送消息
 * @param serviceId 服务 ID
 * @param cmdId 命令 ID
 * @param data 数据
 * @param len 数据长度
 * @return void
 * @note 通用发送数据接口，目前用不上，服务器端不会处理非格式化数据，仅处理格式化后的数据库服务和通信服务信息
 */
ASIO_C_API void Send(int serviceId, int cmdId, const char *data, int len)
{
    if (data == nullptr || len <= 0)
        return;

    // 构造 string (发生一次内存拷贝，保证线程安全)
    std::string strData(data, len);
    CClient::GetInstance().Send(serviceId, cmdId, strData);
}

/**
 * @brief 检查是否连接
 * @return true: 已连接，false: 未连接
 * @note 检查是否连接，通过唯一会话 GetSession()->IsConnected() 获取连接状态
 */
ASIO_C_API bool IsConnected()
{
    // 通过 GetSession()->IsConnected() 获取连接状态
    auto session = CClient::GetInstance().GetSession();
    if (session)
    {
        return session->IsConnected();
    }
    return false;
}

/**
 * @brief 执行 SQL
 * @param req 格式化后的 SQL 语句，详见服务器端的 Request
 * @return void
 * @note 执行 SQL 语句，会根据执行结果返回不同的数据，详见服务器端的 Response
 */
ASIO_C_API void ExecuteSql(const char *req)
{
    if (req == nullptr)
        return;
    CClient::GetInstance().ExecuteSql(std::string(req));
}

/**
 * @brief 登录
 * @param name 用户名
 * @return void
 * @note 通信服务的登陆接口，必须登录，其他连接才能向此连接发送消息，建议定义唯一标识符
 */
ASIO_C_API void Login(const char *name)
{
    if (name)
    {
        CClient::GetInstance().Login(std::string(name));
    }
}

/**
 * @brief 发送聊天消息
 * @param toUser 接收者用户名
 * @param msg 消息内容
 * @return void
 * @note 发送聊天消息，必须登录，此连接才能向其他连接发送信息
 */
ASIO_C_API void SendChat(const char *toUser, const char *msg)
{
    if (msg)
    {
        CClient::GetInstance().SendChat(std::string(toUser), std::string(msg));
    }
}

/**
 * @brief 获取在线用户
 * @return void
 * @note 获取在线用户，必须登录，此连接才能获取在线用户
 */
ASIO_C_API void GetOnLineUser()
{
    CClient::GetInstance().GetOnLineUser();
}