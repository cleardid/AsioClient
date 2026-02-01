cmake_minimum_required(VERSION 3.10)
project(AsioClient)

# 使用 C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =============================================================
# [MOVED UP & FIXED] 输出目录配置 (必须在 add_library 之前)
# =============================================================
# Windows DLL 文件属于 RUNTIME，必须设置这个变量
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# Windows 导入库(.dll.a) 和 Linux .so 属于 LIBRARY
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# 静态库属于 ARCHIVE
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 优先查找静态库
set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_STATIC_RUNTIME ON)

# 设置静态链接标志 (将 gcc, stdc++, pthread 全部打入 DLL)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -static-libgcc -static-libstdc++")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")

# =============================================================
# 1. 包含路径 (Include Paths)
# =============================================================
# 将项目根目录加入包含路径，这样代码中类似 #include "core/..." 的写法也能工作
# 同时这也能确保相对路径 ../../infra 能够正确寻址
include_directories(${CMAKE_SOURCE_DIR})

# =============================================================
# 2. 查找依赖 (Boost)
# =============================================================
# Asio 需要 system 和 thread 库
# date_time 有时在旧版本 boost 处理定时器时需要，建议加上
cmake_policy(SET CMP0167 NEW)
find_package(Boost REQUIRED COMPONENTS system thread date_time)

# =============================================================
# 3. 收集源文件 (Source Files)
# =============================================================
# 因为你已经把文件都复制过来了，我们可以直接扫描 client 目录下的所有源文件
# 扫描 core, infra, services, app 目录下的所有 .cpp 文件
file(GLOB_RECURSE CLIENT_SOURCES
    "core/*.cpp"
    "infra/*.cpp"
    "services/*.cpp"
    "app/dll/*.cpp"  # 这里包含你的 dll_main.cpp
)

# 如果你的目录里还残留了 .c 文件，也可以加上
file(GLOB_RECURSE C_SOURCES "core/*.c" "infra/*.c")

# =============================================================
# 4. 生成动态链接库 (DLL)
# =============================================================
add_library(${PROJECT_NAME} SHARED ${CLIENT_SOURCES} ${C_SOURCES})

# =============================================================
# 5. 平台宏定义与链接配置
# =============================================================
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE 
        ASIOCLIENT_EXPORTS 
        # _WIN32_WINNT=0x0601 
        _CRT_SECURE_NO_WARNINGS
        BOOST_THREAD_USE_LIB # 告诉 Boost 我们在使用静态库
    )
    # 链接 Windows 基础库
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32 mswsock iphlpapi)
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE ASIOCLIENT_EXPORTS)
    target_link_libraries(${PROJECT_NAME} PRIVATE pthread)
endif()

# 链接 Boost
target_link_libraries(${PROJECT_NAME} PRIVATE Boost::system Boost::thread Boost::date_time)

# 输出目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)