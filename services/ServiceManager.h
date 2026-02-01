#ifndef SERVICEMANAGER_H
#define SERVICEMANAGER_H

#include <unordered_map>
#include <memory>
#include <mutex>
#include "IService.h"

class ServiceManager
{
public:
    static ServiceManager &GetInstance();

    // 注册服务
    void RegisterService(std::shared_ptr<IService> service);

    // 分发消息
    // 返回 true 表示消息被内部服务处理了，不需要转发给 Unity
    bool Dispatch(std::shared_ptr<MsgNode> msg);

private:
    ServiceManager() = default;

    std::mutex _mutex;
    std::unordered_map<int, std::shared_ptr<IService>> _services;
};

#endif