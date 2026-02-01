#ifndef ISERVICE_H
#define ISERVICE_H

#include <memory>
#include "../core/message/MsgNode.h"

// 客户端服务接口基类
class IService
{
public:
    IService() = default;
    virtual ~IService() = default;

    // 获取服务ID
    virtual int GetServiceId() const = 0;

    // 处理服务器发回来的消息
    // 返回值: true 表示已处理(拦截)，false 表示需要继续传递给 Unity
    virtual bool Handle(std::shared_ptr<MsgNode> msg) = 0;
};

#endif