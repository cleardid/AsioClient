#ifndef HEARTSERVICE_H
#define HEARTSERVICE_H

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include "../IService.h"

class HeartService : public IService, public std::enable_shared_from_this<HeartService>
{
public:
    // 初始化时需要传入 io_context 以创建定时器
    HeartService(boost::asio::io_context &ioc);
    ~HeartService();

    // 启动心跳
    void Start();
    // 停止心跳
    void Stop();

    // IService 接口实现
    int GetServiceId() const override;
    bool Handle(std::shared_ptr<MsgNode> msg) override;

private:
    // 定时器回调
    void OnTimer(const boost::system::error_code &ec);

private:
    boost::asio::steady_timer _timer;
    bool _isStopped = false;
};

#endif