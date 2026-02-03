#include "CommunicationService.h"
#include "../../core/client/CClient.h"
#include "../../infra/util/json.hpp" // 使用 json 库

#include "../../infra/log/Logger.h"

using json = nlohmann::json;

CommunicationService::CommunicationService()
{
}

CommunicationService::~CommunicationService()
{
}

int CommunicationService::GetServiceId() const
{
    // 对应 Const.h 中的 SERVICE_COMMUNICATION = 3
    return SERVICE_COMMUNICATION;
}

bool CommunicationService::Handle(std::shared_ptr<MsgNode> msg)
{
    // 收到通信服务的回包
    // 通常是服务器转发过来的聊天消息，格式可能是 {"cmd": COMMUINICATION_RECV, "from": "Tom", "msg": "Hello"}

    // 我们选择不拦截 (return false)，
    // 这样 CClient 会通过 OnMessageCallback 把这个 JSON 字符串传给 Unity。
    // Unity 解析 JSON 后显示在 UI 上。
    return false;
}

void CommunicationService::Login(const std::string &name)
{
    _myName = name;
    try
    {
        json j;
        j["target"] = {{"name", name}};

        // 发送给服务器
        CClient::GetInstance().Send(SERVICE_COMMUNICATION, COMMUINICATION_REGISTER, j.dump());
    }
    catch (std::exception &e)
    {
        LOG_ERROR << "CommunicationService::Login exception: " << e.what() << std::endl;
    }
}

void CommunicationService::SendMsg(const std::string &toUser, const std::string &msg)
{
    try
    {
        json j;
        j["target"] = {
            {"name", _myName},
            {"client", toUser},
            {"message", msg}};

        CClient::GetInstance().Send(SERVICE_COMMUNICATION, COMMUINICATION_SEND, j.dump());
    }
    catch (std::exception &e)
    {
        LOG_ERROR << "CommunicationService::SendMsg exception: " << e.what() << std::endl;
    }
}

void CommunicationService::GetOnLineUser()
{
    CClient::GetInstance().Send(SERVICE_COMMUNICATION, COMMUINICATION_SHOW, "Get OnLine Clients.");
}
