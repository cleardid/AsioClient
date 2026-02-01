cmake_minimum_required(VERSION 3.10)
project(AsioClient)

# 使用 C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# 方便 VS Code 插件提供代码补全
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 统一设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # DLL 在 Windows 视为 RUNTIME, 但统一设为 bin
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Asio 需要 system 和 thread 库
# date_time 有时在旧版本 boost 处理定时器时需要
cmake_policy(SET CMP0167 NEW)
find_package(Boost REQUIRED COMPONENTS system thread date_time)

# 定义目标
file(GLOB_RECURSE ALL_SRCS
    "core/*.cpp"
    "infra/*.cpp"
    "services/*.cpp"
    "app/dll/*.cpp"
)

# 生成动态链接库
add_library(${PROJECT_NAME} SHARED ${ALL_SRCS})

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE 
    ASIOCLIENT_EXPORTS
    _CRT_SECURE_NO_WARNINGS
)

# =============================================================
# 5. 平台宏定义与链接配置
# =============================================================
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE 
        # 告诉 Boost 我们在使用静态库
        BOOST_THREAD_USE_LIB
    )
    # 静态链接系统库 (解决 MSYS2 运行时依赖)
    target_link_options(${PROJECT_NAME} PRIVATE 
        $<$<CXX_COMPILER_ID:GNU>:-static;-static-libgcc;-static-libstdc++>
    )
    # 链接 Windows 基础库
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32 mswsock iphlpapi)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE pthread)
endif()

# 链接依赖库
target_link_libraries(${PROJECT_NAME} PRIVATE 
    Boost::system 
    Boost::thread 
    Boost::date_time
)