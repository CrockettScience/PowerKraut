/*
Copyright 2018 Jonathan Crockett

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#ifndef KRAUTVK_H_
#define KRAUTVK_H_

#include <cstdio>
#include <vector>
#include <iostream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

//MACROS
#define EXPORT extern "C" __declspec(dllexport)
#define SUCCESS (0)
#define INIT_FAILED (-1)
#define WINDOW_CREATION_FAILED (-2)
#define VULKAN_NOT_SUPPORTED (-3)
#define VULKAN_INSTANCE_CREATION_FAILED (-4)
#define VULKAN_DEVICE_CREATION_FAILED (-5)
#define VULKAN_SURFACE_CREATION_FAILED (-6)

//FUNCTION HEADERS
namespace KrautVK {

    //KRAUTVK VERSION
    uint32_t major = 0;
    uint32_t minor = 2;
    uint32_t patch = 0;
    uint32_t version = VK_MAKE_VERSION(major, minor, patch);

    //VULKAN FUNCTION POINTERS
    PFN_vkCreateInstance createInstance;

    PFN_vkCreateDevice createDevice;
    PFN_vkEnumeratePhysicalDevices enumeratePhysicalDevices;
    PFN_vkGetPhysicalDeviceProperties getPhysicalDeviceProperties;
    PFN_vkGetPhysicalDeviceFeatures getPhysicalDeviceFeatures;
    PFN_vkGetPhysicalDeviceQueueFamilyProperties getPhysicalDeviceQueueFamilyProperties;
    PFN_vkDestroyInstance destroyInstance;

    PFN_vkGetDeviceProcAddr getDeviceProcAddr;
    PFN_vkGetDeviceQueue getDeviceQueue;
    PFN_vkDeviceWaitIdle deviceWaitIdle;
    PFN_vkDestroyDevice destroyDevice;
    PFN_vkDestroySurfaceKHR destroySurfaceKHR;

    class KrautVK {

    private:

        static GLFWwindow *window;
        static GLFWmonitor *monitor;
        static VkInstance instance;
        static VkDevice device;
        static uint32_t queueFamilyIndex;
        static VkQueue commandBuffer;
        static VkSurfaceKHR applicationSurface;

        static int initGLFW(int width, int height, char *title, int fullScreen);

        static int checkDeviceProperties(VkPhysicalDevice physicalDevice, uint32_t &selectedFamilyIndex);

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