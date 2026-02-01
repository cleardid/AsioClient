#ifndef ASIOCLIENT_EXPORT_H
#define ASIOCLIENT_EXPORT_H

// 定义 DLL 导出宏
#if defined(_WIN32)
// Windows 平台（包括 MSVC 和 MinGW-w64）
#ifdef ASIOCLIENT_EXPORTS
// MinGW-w64 和 MSVC 都支持 __declspec(dllexport)
#define ASIO_API __declspec(dllexport)
#else
#define ASIO_API __declspec(dllimport)
#endif
#else
// Linux / Mac 平台（GCC/Clang）
#define ASIO_API __attribute__((visibility("default")))
#endif

// 定义 C 接口导出 (供 Unity/C# 使用)
// 注意：extern "C" 要放在最外层，避免和 __declspec 顺序问题
#define ASIO_C_API extern "C" ASIO_API

#endif // ASIOCLIENT_EXPORT_H