#ifndef COMMUNICATIONSERVICE_H
#define COMMUNICATIONSERVICE_H

#include "../IService.h"
#include "../../core/common/Const.h" // 包含 SERVICE_COMMUNICATION, COMMUNICATION_CMD 定义
#include <string>

class CommunicationService : public IService
{
public:
    CommunicationService();
    ~CommunicationService();

    // 1. 实现接口
    int GetServiceId() const override;

    // 2. 处理回包 (接收别人的消息)
    // 返回 false 表示继续转发给 Unity (让 Unity 显示聊天内容)
    bool Handle(std::shared_ptr<MsgNode> msg) override;

    // 3. 主动发送 - 登录/注册
    void Login(const std::string &name);

    // 4. 主动发送 - 发送消息
    void SendMsg(const std::string &toUser, const std::string &msg);

private:
    std::string _myName;
};

#endif