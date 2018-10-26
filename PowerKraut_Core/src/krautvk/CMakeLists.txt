cmake_minimum_required(VERSION 3.10)
project(krautvk C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_SHARED_LIBRARY_PREFIX  "")
set(STDLIB_DIR "C:/Program\ Files/mingw-w64/x86_64-8.1.0-posix-seh-rt_v6-rev0/mingw64/bin")

add_library(krautvk SHARED src/KrautVK.cpp src/KrautVK.h src/KrautVKCommon.h)

target_link_libraries(krautvk ${PROJECT_SOURCE_DIR}/lib/glfw3.dll)

file(COPY ${PROJECT_SOURCE_DIR}/lib/glfw3.dll DESTINATION ${PROJECT_BINARY_DIR})
file(COPY ${STDLIB_DIR}/libstdc++-6.dll DESTINATION ${PROJECT_BINARY_DIR})
file(COPY ${STDLIB_DIR}/libgcc_s_seh-1.dll DESTINATION ${PROJECT_BINARY_DIR})
file(COPY ${STDLIB_DIR}/libwinpthread-1.dll DESTINATION ${PROJECT_BINARY_DIR})

file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/data)

add_custom_target(shaderbuilder COMMAND cmd /c ${PROJECT_SOURCE_DIR}/lib/spirv/shaderbuilder.bat)

file(GLOB SPIRV_SHADERS ${PROJECT_SOURCE_DIR}/lib/spirv/*.spv)

foreach(SPIRV_SHADER ${SPIRV_SHADERS})
    add_custom_command(TARGET shaderbuilder COMMAND ${CMAKE_COMMAND} -E copy ${SPIRV_SHADER} ${PROJECT_BINARY_DIR}/data)
endforeach()

add_dependencies(krautvk shaderbuilder)

include_directories(./include C:/VulkanSDK/1.1.73.0/Include)