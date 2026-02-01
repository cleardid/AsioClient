# 项目简介
这是一个使用 asio 搭建的客户端 DLL 项目，用于导出 DLL 在 Unity 或 QT 或 WPF 中使用。

# 环境搭建
下载安装 msys2，在其中的 MinGW 中安装 boost 库 和 cmake 库

# 构建过程
cd build
rm -rf *
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
在 build/bin 中会生成 dll
