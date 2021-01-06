cmake_minimum_required(VERSION 3.10)
project(krautvk
        VERSION 0.1
        DESCRIPTION "A simple 2D renderer built with using Vulkan and GLFW."
        HOMEPAGE_URL "https://github.com/CrockettScience/PowerKraut"
        LANGUAGES C CXX)

configure_file(src/KrautVKConfig.h.in src/KrautVKConfig.h)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_SHARED_LIBRARY_PREFIX  "")
set(STDLIB_DIR "C:/Program Files (x86)/Microsoft Visual Studio/2017/Community/VC/Tools/MSVC/14.16.27023/bin/Hostx64/x64")
add_library(krautvk SHARED src/KrautVKExport.cpp)

file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

target_link_libraries( krautvk PRIVATE ${PROJECT_SOURCE_DIR}/lib/bin/glfw3.lib )

file(COPY ${PROJECT_SOURCE_DIR}/lib/res DESTINATION ${PROJECT_BINARY_DIR})
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/data)

add_custom_target(shaderbuilder COMMAND cmd /c ${PROJECT_SOURCE_DIR}/lib/spirv/shaderbuilder.bat)

file(GLOB SPIRV_SHADERS ${PROJECT_SOURCE_DIR}/lib/spirv/*.spv)

foreach(SPIRV_SHADER ${SPIRV_SHADERS})
    add_custom_command(TARGET shaderbuilder COMMAND ${CMAKE_COMMAND} -E copy ${SPIRV_SHADER} ${PROJECT_BINARY_DIR}/data)
endforeach()

add_dependencies(krautvk shaderbuilder)

include_directories(./include P:/glfw/glfw-3.3.2/include)
include_directories(./include P:/glfw/stb-master)
include_directories(./include C:/VulkanSDK/1.2.162.0/Include)
include_directories(./include ${PROJECT_BINARY_DIR}/src)