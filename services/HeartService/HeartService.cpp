#include "HeartService.h"
#include "../../core/client/CClient.h"
#include "../../core/common/Const.h"

HeartService::HeartService(boost::asio::io_context &ioc)
    : _timer(ioc)
{
}

HeartService::~HeartService()
{
    Stop();
}

int HeartService::GetServiceId() const
{
    return SERVICE_HEART;
}

void HeartService::Start()
{
    _isStopped = false;
    // 启动定时器：5秒后触发
    _timer.expires_after(std::chrono::seconds(5));
    _timer.async_wait(std::bind(&HeartService::OnTimer, shared_from_this(), std::placeholders::_1));
}

void HeartService::Stop()
{
    _isStopped = true;
    boost::system::error_code ec;
    _timer.cancel();
}

void HeartService::OnTimer(const boost::system::error_code &ec)
{
    if (ec || _isStopped)
        return;

    // 1. 发送心跳包
    // 构造一个空的心跳包 body，或者包含时间戳
    std::string heartData = "ping";
    CClient::GetInstance().Send(SERVICE_HEART, HEART_RECV, heartData);

    // 2. 再次启动定时器 (循环)
    _timer.expires_after(std::chrono::seconds(5));
    _timer.async_wait(std::bind(&HeartService::OnTimer, shared_from_this(), std::placeholders::_1));
}

bool HeartService::Handle(std::shared_ptr<MsgNode> msg)
{
    // 返回 true 表示我们处理完了，不用发给 Unity 了
    return true;
}