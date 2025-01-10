cmake_minimum_required(VERSION 3.10)
project(monitor)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# 设置编译类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# 设置编译选项
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -DDEBUG")


# 添加源文件
add_executable(monitor 
    src/main.cpp
    src/network/http_server.cpp
    src/network/tcp_server.cpp
    src/database/database.cpp
    src/scoring/environment_scorer.cpp
    src/device/device_manager.cpp
    src/services/environment_service.cpp
    src/tasks/data_maintenance.cpp
)

# 包含目录
target_include_directories(monitor PRIVATE 
    /usr/include/mysql
    /usr/local/include
)

# 链接库
target_link_libraries(monitor PRIVATE
    boost_system
    mysqlclient
    jsoncpp
    pthread
)

# 添加设备模拟器可执行文件
add_executable(device_simulator
    tools/device_simulator.cpp
)

# 设置包含目录
target_include_directories(device_simulator PRIVATE
    /usr/local/include
)

# 链接库
target_link_libraries(device_simulator PRIVATE
    boost_system
    jsoncpp
    pthread
)

# 为调试版本添加预处理器定义
target_compile_definitions(monitor PRIVATE
    $<$<CONFIG:Debug>:DEBUG_MODE>
)

target_compile_definitions(device_simulator PRIVATE
    $<$<CONFIG:Debug>:DEBUG_MODE>
)

# find_package(Boost REQUIRED COMPONENTS system)
# find_package(MySQL REQUIRED)

# target_link_libraries(monitor
#     ${Boost_LIBRARIES}
#     ${MYSQL_LIBRARIES}
#     jsoncpp
# )