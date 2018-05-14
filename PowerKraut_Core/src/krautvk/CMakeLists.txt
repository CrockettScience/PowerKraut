cmake_minimum_required(VERSION 3.10)
project(krautvk C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_SHARED_LIBRARY_PREFIX  "")

add_library(krautvk SHARED library.c library.h)

target_link_libraries(krautvk ${PROJECT_SOURCE_DIR}/lib/glfw3.dll)

include_directories(./include C:/VulkanSDK/1.1.73.0/Include)