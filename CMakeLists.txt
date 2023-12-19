# 指定CMake最低版本
cmake_minimum_required(VERSION 3.0)

# 设置项目名称
project(yolocap)

# 设置编译选项
# 使用方法
# cmake -DCMAKE_BUILD_TYPE=Debug ..
# cmake -DCMAKE_BUILD_TYPE=Release ..
if(NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "Release")
    # 在Release模式下使用O3优化选项
    message(STATUS "Release mode enabled")
    set(CMAKE_BUILD_TYPE "Release")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3") # 添加-O3选项
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # 在Debug模式下启用符号信息调试
    message(STATUS "Debug mode enabled")
    set(CMAKE_BUILD_TYPE "Debug")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g") # 添加-g选项
else()
    message(FATAL_ERROR "Unknown build type: ${CMAKE_BUILD_TYPE}")
endif()

# 查找所有源文件
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# 设置中间文件输出目录为 build 目录
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

# 设置最终文件输出目录为 bin 目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# 查找OpenCV
find_package(OpenCV REQUIRED)

# 使用 C++11 标准编译
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

# 添加头文件目录
include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/spdlog-1.12.0/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/json-3.11.3/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/csv-parser"
)

# 添加可执行文件，并链接OpenCV、pthread库
add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-pthread" LINK_FLAGS "-pthread")
