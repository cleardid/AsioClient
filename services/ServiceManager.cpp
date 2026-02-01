#include "ServiceManager.h"

ServiceManager &ServiceManager::GetInstance()
{
    static ServiceManager instance;
    return instance;
}

void ServiceManager::RegisterService(std::shared_ptr<IService> service)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _services[service->GetServiceId()] = service;
}

bool ServiceManager::Dispatch(std::shared_ptr<MsgNode> msg)
{
    int id = msg->GetHeader().serviceId;
    std::shared_ptr<IService> service = nullptr;

    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _services.find(id);
        if (it != _services.end())
        {
            service = it->second;
        }
    }

    // 如果找到了对应的 C++ 服务，就交给它处理
    if (service)
    {
        return service->Handle(msg);
    }

    // 没找到服务，说明这是业务消息，返回 false 让 CClient 转发给 Unity
    return false;
}