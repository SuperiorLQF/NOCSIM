cmake_minimum_required(VERSION 3.10)
project(NocSim LANGUAGES CXX)

# 设定 C++ 标准（SystemC 3.0.1 需要至少 C++17）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 源文件所在路径
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../src")

# 自动收集所有 .cpp 源文件
file(GLOB_RECURSE SRC_FILES "${SRC_DIR}/*.cpp")

# SystemC 安装路径
set(SYSTEMC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/systemc-3.0.1")

# 头文件和库路径
include_directories("${SRC_DIR}")
include_directories("${SYSTEMC_DIR}/include")

link_directories("${SYSTEMC_DIR}/lib")

# 生成可执行文件
add_executable(test ${SRC_FILES})

# 链接 SystemC
target_link_libraries(test systemc m)

# 编译完成后自动执行可执行文件
add_custom_command(TARGET test
    POST_BUILD
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/test
    COMMENT "✅ 编译完成，正在自动运行 test..."
)