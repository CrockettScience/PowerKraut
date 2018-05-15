#ifndef KRAUTVK_H
#define KRAUTVK_H
#define GLFW_INCLUDE_VULKAN

using namespace std;
#include <GLFW/glfw3.h>
#include <cstdio>

//KRAUTVK VERSION
uint32_t version = VK_MAKE_VERSION(0, 1, 0);

//MACROS
#define EXPORT extern "C" __declspec(dllexport)
#define SUCCESS (0)
#define INIT_FAILED (-1)
#define WINDOW_CREATION_FAILED (-2)
#define VULKAN_NOT_SUPPORTED (-3)

//VULKAN FUNCTION POINTERS
PFN_vkCreateInstance createInstance;
PFN_vkCreateDevice createDevice;

//FUNCTION HEADERS
namespace KrautVK {
    class KrautVK {

    private:

        static GLFWwindow* window;
        static GLFWmonitor* monitor;

        static int initGLFW(int width, int height, char *title, int fullScreen);

        static int initVulkan(const char *title);

    public:

        static int init(int w, int h, char *title, int f);

        static int windowShouldClose();

        static void pollEvents();

        static void terminate();
    };

    EXPORT int init(int w, int h, char *title, int f);

    EXPORT int windowShouldClose();

    EXPORT void pollEvents();

    EXPORT void terminate();
}



#endif