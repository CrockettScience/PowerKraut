cmake_minimum_required(VERSION 3.10)
project(krautvk C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_SHARED_LIBRARY_PREFIX  "")

add_library(krautvk SHARED KrautVK.cpp KrautVK.h)

target_link_libraries(krautvk ${PROJECT_SOURCE_DIR}/lib/glfw3.dll)

include_directories(./include C:/VulkanSDK/1.1.73.0/Include)