cmake_minimum_required(VERSION 3.1)

# 项目名称: LightningWebServer 语言: C++
project("LightningWebServer" LANGUAGES CXX)

# 添加子目录并构建该目录
add_subdirectory(Timer Timer) #将输出文件放在Timer目录下
add_subdirectory(Logging Logging) #将输出文件放在Logging目录下

# # 设置生成静态库文件位置
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "/build")

# # 设置生成动态库文件位置
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "/build")

# # 设置生成运行文件位置
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "/build")

# 链接库


# 可执行程序
add_executable(server Main.cpp WebServer.cpp ThreadPool.cpp HttpRequest.cpp Epoll.cpp Utils.cpp)
# target_sources(server WebServer.cpp)
# target_sources(server ThreadPool.cpp)
# target_sources(server HttpRequest.cpp)
# target_sources(server Epoll.cpp)
# target_sources(server Utils.cpp)

# 后跟库的具体位置
target_link_libraries(server /Logging/libLogging.a  /Timer/libTimer.a)