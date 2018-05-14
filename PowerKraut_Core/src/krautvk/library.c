
#define GLFW_INCLUDE_VULKAN
#include "library.h"
#include <GLFW/glfw3.h>
#include <stdio.h>

#define EXPORT __declspec(dllexport)

GLFWwindow* window;
GLFWmonitor* monitor;

EXPORT int Init(int width, int height, char *title, int fullScreen){
    printf("***Initializing KrautVK***\nVersion 0.0.2\n");

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

    if (!glfwVulkanSupported())
    {
        return -3;
    }
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