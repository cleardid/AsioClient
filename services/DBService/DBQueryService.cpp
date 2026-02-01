#include "DBQueryService.h"
#include "../../core/client/CClient.h"
#include "../../infra/util/json.hpp" // 引用 nlohmann/json

#include "../../infra/log/Logger.h"

using json = nlohmann::json;

DBQueryService::DBQueryService()
{
}

DBQueryService::~DBQueryService()
{
}

int DBQueryService::GetServiceId() const
{
    return SERVICE_DB;
}

bool DBQueryService::Handle(std::shared_ptr<MsgNode> msg)
{
    // 返回 false !!!
    // 这是一个关键点：客户端 C++ 层通常只负责转发数据。
    // 我们返回 false，让 CClient::OnMessage 继续执行，
    // 把这个 JSON 数据通过回调函数传给 Unity/C# 层去解析和显示 UI。
    return false;
}

void DBQueryService::ExecuteQuery(const std::string &req)
{
    try
    {
        // 发送
        CClient::GetInstance().Send(SERVICE_DB, DB_EXECUTE, req);
    }
    catch (std::exception &e)
    {
        LOG_ERROR << "DBQueryService::ExecuteQuery exception: " << e.what() << std::endl;
    }
}

void DBQueryService::CloseDB()
{
    try
    {
        json j;
        j["cmd"] = DB_CLOSE; // DB_CLOSE = 2

        CClient::GetInstance().Send(SERVICE_DB, DB_CLOSE, j.dump());
    }
    catch (...)
    {
        LOG_ERROR << "DBQueryService::CloseDB exception" << std::endl;
    }
}