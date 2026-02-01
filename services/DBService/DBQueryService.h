#ifndef DBQUERYSERVICE_H
#define DBQUERYSERVICE_H

#include "../IService.h"
#include "../../core/common/Const.h" // 包含 SERVICE_DB, DB_CMD 定义
#include <string>

class DBQueryService : public IService
{
public:
    DBQueryService();
    ~DBQueryService();

    // 实现接口
    int GetServiceId() const override;

    // 处理回包
    // 返回 false，表示我们希望 CClient 继续把消息转发给 Unity (让 Unity 解析 JSON 结果)
    bool Handle(std::shared_ptr<MsgNode> msg) override;

    // 发送 SQL 查询请求
    void ExecuteQuery(const std::string &req);

    // 关闭数据库连接
    void CloseDB();
};

#endif