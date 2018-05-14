
#define GLFW_INCLUDE_VULKAN
#include "library.h"
#include <GLFW/glfw3.h>
#include <stdio.h>

#define EXPORT __declspec(dllexport)

GLFWwindow* window;
GLFWmonitor* monitor;

int InitGLFW(int width, int height, char *title, int fullScreen){
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    monitor = glfwGetPrimaryMonitor();


    if(fullScreen)
        window = glfwCreateWindow(width, height, title, monitor, NULL);
    else
        window = glfwCreateWindow(width, height, title, NULL, NULL);


    if (!window)
    {
        return -2;
    }

    return 0;
}

int InitVulkan(){

    if (!glfwVulkanSupported())
        return -3;

    return 0;
}

EXPORT int Init(int width, int height, char *title, int fullScreen){
    printf("***Initializing KrautVK***\nVersion 0.0.3\n");

    int status = InitGLFW(width, height, title, fullScreen);

    if(status != 0)
        return status;

    status = InitVulkan();

    if(status != 0)
        return status;


    return 0;
}

EXPORT int WindowShouldClose(){
    return glfwWindowShouldClose(window);

}

EXPORT void PollEvents(){
    glfwPollEvents();
}

EXPORT void Terminate(){
    glfwTerminate();
}