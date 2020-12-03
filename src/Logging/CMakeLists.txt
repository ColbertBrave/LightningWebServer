# cmake最低版本要求
cmake_minimum_required(VERSION 3.1)

# 使用glob方式组织源码
FILE(GLOB_RECURSE Logging "*.cpp" "*.hpp" "*.h" "*.c")

# 输出文件列表
# message(STATUS "file list: ${Logging}")

# 将源代码添加到此项目的可执行文件。
add_library (Logging  STATIC Logging.cpp)

# 设置属性
set_target_properties(Logging
PROPERTIES
  # 语言标准
  CXX_STANDARD 11
  # 编译器特定扩展是否开启
  CXX_EXTENSIONS OFF
  # 指定所选标准的版本。如果这个版本不可用，CMake将停止配置并出现错误。
  # 当这个属性被设置为OFF时，CMake将寻找下一个标准的最新版本，直到一个合适的标志。
  CXX_STANDARD_REQUIRED ON

  # 设置生成位置无关代码
  POSITION_INDEPENDENT_CODE ON
)