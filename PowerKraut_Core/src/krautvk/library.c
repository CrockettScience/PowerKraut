#include "library.h"
#include <GLFW/glfw3.h>
#include <stdio.h>

#define EXPORT __declspec(dllexport)

GLFWwindow* window;

EXPORT int Init(){
    printf("***Initializing KrautVK***\nVersion 0.0.1\n");

    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(640, 480, "PowerKraut", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    return 0;
}

EXPORT int WindowShouldClose(){
    return glfwWindowShouldClose(window);

}

EXPORT void PollEvents(){
    glfwPollEvents();
}