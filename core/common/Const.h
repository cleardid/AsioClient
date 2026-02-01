
#ifndef CONST_H
#define CONST_H

#include <string>
#include <cstddef>

// 服务枚举
enum SERVICE_TYPE
{
    SERVICE_HEART = 0,         // 心跳服务
    SERVICE_HELLO = 1,         // hello服务
    SERVICE_DB = 2,            // 数据库服务
    SERVICE_COMMUNICATION = 3, // 通信服务
};

// 心跳检测服务命令枚举
enum HEART_CMD
{
    HEART_RECV = 1,
    HEART_ACK = 2,
};

// hello 服务命令枚举
enum HELLO_CMD
{
    HELLO_CMD_TEST = 1, // 测试命令
};

// 数据库服务命令枚举
enum DB_CMD
{
    DB_EXECUTE = 1, // 执行命令
    DB_CLOSE = 2,   // 关闭连接
};

// 通信服务命令枚举
enum COMMUNICATION_CMD
{
    COMMUINICATION_REGISTER = 1, // 创建连接
    COMMUINICATION_CLOSE = 2,    // 关闭连接
    COMMUINICATION_SEND = 3,     // 发送数据
    COMMUINICATION_RECV = 4,     // 接收数据
    COMMUINICATION_SHOW = 5,     // 显示连接信息
};

#pragma region 日志相关枚举及方法

// 日志等级枚举
enum class LogLevel
{
    UNKNOWN = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    MYERROR = 4,
    FATAL = 5
};

// 级别转字符串（输出用）
inline std::string LogLevelToString(LogLevel level)
{
    switch (level)
    {
    case LogLevel::UNKNOWN:
        return "UNKNOWN";
    case LogLevel::DEBUG:
        return "DEBUG";
    case LogLevel::INFO:
        return "INFO ";
    case LogLevel::WARN:
        return "WARN ";
    case LogLevel::MYERROR:
        return "ERROR";
    case LogLevel::FATAL:
        return "FATAL";
    default:
        return "UNKNOWN";
    }
}

#pragma endregion

#endif // CONST_H