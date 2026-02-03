cmake_minimum_required(VERSION 3.15)
project(AsioClient LANGUAGES CXX)

# 1. 全局设置
set(CMAKE_CXX_STANDARD 20) # 统一升级为 C++20，如需 C++17 请修改此处
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 2. 依赖查找与平台策略
if(WIN32)
    # MSYS2 + MinGW64 或 MSVC
    cmake_policy(SET CMP0167 NEW)
    
    # Windows 下通常由包管理器或环境变量处理路径
    find_package(Boost CONFIG REQUIRED COMPONENTS system thread date_time)
    
elseif(APPLE)
    cmake_policy(SET CMP0144 NEW)
    cmake_policy(SET CMP0167 NEW)

    # MacOS Homebrew 路径指定 (仿照 Server 配置)
    set(BOOST_ROOT "/opt/homebrew/opt/boost@1.85")
    
    find_package(Boost REQUIRED COMPONENTS system thread date_time)
endif()

# 3. 源文件管理
# 注意：Server端采用了手动列出文件的方式(推荐)，但因不知道你具体文件名，
# 这里保留 GLOB_RECURSE，但建议后期改为手动 add_library(name src1 src2...) 以避免构建缓存问题
file(GLOB_RECURSE ALL_SRCS
    "core/*.cpp"
    "infra/*.cpp"
    "services/*.cpp"
    "app/dll/*.cpp"
)

add_library(${PROJECT_NAME} SHARED
    ${ALL_SRCS}
)

# 4. 包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${BOOST_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    
    # 显式列出子模块目录，结构更清晰
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/infra
    ${CMAKE_CURRENT_SOURCE_DIR}/services
    ${CMAKE_CURRENT_SOURCE_DIR}/app/dll
)

if(APPLE)
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${BOOST_INCLUDE_DIR}
    )
endif()

# 5. 编译定义 (宏)
target_compile_definitions(${PROJECT_NAME} PRIVATE 
    ASIOCLIENT_EXPORTS
    _CRT_SECURE_NO_WARNINGS
)

if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE 
        BOOST_THREAD_USE_LIB # 告诉 Boost 使用静态库
    )
endif()

# 6. 链接库
if(WIN32)
    # 静态链接系统库 (解决 MSYS2/MinGW 运行时依赖)
    target_link_options(${PROJECT_NAME} PRIVATE 
        $<$<CXX_COMPILER_ID:GNU>:-static;-static-libgcc;-static-libstdc++>
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE
        Boost::system 
        Boost::thread 
        Boost::date_time
        ws2_32 
        mswsock 
        iphlpapi
    )
elseif(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Boost::system 
        Boost::thread 
        Boost::date_time
        pthread
    )
else()
    # Linux 或其他 Unix
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Boost::system 
        Boost::thread 
        Boost::date_time
        pthread
    )
endif()

# 7. 输出目录配置
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin") # DLL 输出位置
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib") # LIB 输出位置

if(WIN32)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/lib/Debug")

    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib/Release")
endif()