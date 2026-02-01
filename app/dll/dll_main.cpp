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

// 1. 初始化客户端
ASIO_C_API void Client_Init()
{
    try
    {
        // 获取单例并初始化线程
        auto &client = CClient::GetInstance();
        client.Init();

        // 设置内部回调，当 CClient 收到消息时，会调用 InternalCallback
        client.SetCallback(InternalCallback);

        LOG_INFO << "[DLL] Client Initialized." << std::endl;
    }
    catch (std::exception &e)
    {
        LOG_ERROR << "[DLL] Init Exception: " << e.what() << std::endl;
    }
}

// 2. 连接服务器
ASIO_C_API bool Client_Connect(const char *ip, int port, int timeout)
{
    if (ip == nullptr)
        return false;

    try
    {
        CClient::GetInstance().Connect(ip, static_cast<uint16_t>(port), timeout);

        return true;
    }
    catch (std::exception &e)
    {
        LOG_ERROR << "[DLL] Connect Exception: " << e.what() << std::endl;
        return false;
    }
}

// 3. 关闭连接
ASIO_C_API void Client_Close()
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

// 4. 设置消息回调 (供 C# 调用)
// callback: C# 端的委托 (delegate)
ASIO_C_API void Client_SetOnMessageCallback(ExternalCallbackType callback)
{
    g_ExternalCallback = callback;
    // 同时更新 CClient 中的回调指向
    CClient::GetInstance().SetCallback(InternalCallback);
}

// 5. 通用发送接口
ASIO_C_API void Client_Send(int serviceId, int cmdId, const char *data, int len)
{
    if (data == nullptr || len <= 0)
        return;

    // 构造 string (发生一次内存拷贝，保证线程安全)
    std::string strData(data, len);
    CClient::GetInstance().Send(serviceId, cmdId, strData);
}

// 6. 检查是否已连接
ASIO_C_API bool Client_IsConnected()
{
    // 通过 GetSession()->IsConnected() 获取连接状态
    auto session = CClient::GetInstance().GetSession();
    if (session)
    {
        return session->IsConnected();
    }
    return false;
}

// 7. 执行 SQL
ASIO_C_API void Client_ExecuteSql(const char *req)
{
    if (req == nullptr)
        return;
    CClient::GetInstance().ExecuteSql(std::string(req));
}

// 8. 登录/注册名字
ASIO_C_API void Client_Login(const char *name)
{
    if (name)
    {
        CClient::GetInstance().Login(std::string(name));
    }
}

// 9. 发送聊天
ASIO_C_API void Client_SendChat(const char *toUser, const char *msg)
{
    if (msg)
    {
        CClient::GetInstance().SendChat(std::string(toUser), std::string(msg));
    }
}