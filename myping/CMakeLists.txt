# CMakeList.txt: myping 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

# 文件写出位置
set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/BinOutput/${CMAKE_BUILD_TYPE}/${ANDROID_ABI}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")

string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_UP)
if(CMAKE_BUILD_TYPE_UP STREQUAL "DEBUG")
    message(STATUS "=== Add debug information and symbols ===")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -g")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -g")
else()
    message(STATUS "=== Remove debug information and symbols ===")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -s")
endif()

# 添加头文件
include_directories(
    "${PROJECT_SOURCE_DIR}/${CMAKE_PROJECT_NAME}"
)

# 将源代码添加到此项目的可执行文件。
aux_source_directory(. program_src)
aux_source_directory(ping ping_src)
aux_source_directory(misc misc_src)

add_executable (myping
    ${program_src}
    ${ping_src}
    ${misc_src}
)

# 添加静态库
if(LINUX)
    message(STATUS "Platform: Linux")
elseif(WIN32)
    message(STATUS "Platform: Windows")
    target_link_libraries(myping ws2_32)
else()
    message(STATUS "Platform: Unknown")
endif()
# TODO: 如有需要，请添加测试并安装目标。