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

#include "KrautVKCommon.h"

//MACROS
#define EXPORT extern "C" __declspec(dllexport)
#define SUCCESS (0)
#define GLFW_INIT_FAILED (-1)
#define GLFW_WINDOW_CREATION_FAILED (-2)
#define VULKAN_NOT_SUPPORTED (-3)
#define VULKAN_INSTANCE_CREATION_FAILED (-4)
#define VULKAN_DEVICE_CREATION_FAILED (-5)
#define VULKAN_SURFACE_CREATION_FAILED (-6)
#define VULKAN_SEMAPHORE_CREATION_FAILED (-7)

//FUNCTION HEADERS
namespace KrautVK {

    //KRAUTVK VERSION
    uint32_t major = 0;
    uint32_t minor = 3;
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
    PFN_vkEnumerateDeviceExtensionProperties enumerateDeviceExtensionProperties;
    PFN_vkDestroySurfaceKHR destroySurfaceKHR;

    PFN_vkGetDeviceProcAddr getDeviceProcAddr;
    PFN_vkGetDeviceQueue getDeviceQueue;
    PFN_vkDeviceWaitIdle deviceWaitIdle;
    PFN_vkDestroyDevice destroyDevice;
    PFN_vkCreateSemaphore createSemaphore;
    PFN_vkCreateSwapchainKHR createSwapchainKHR;
    PFN_vkDestroySwapchainKHR destroySwapchainKHR;
    PFN_vkGetSwapchainImagesKHR getSwapchainImagesKHR;
    PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR getPhysicalDeviceSurfaceCapabilitiesKHR;
    PFN_vkGetPhysicalDeviceSurfaceFormatsKHR getPhysicalDeviceSurfaceFormatsKHR;
    PFN_vkGetPhysicalDeviceSurfacePresentModesKHR getPhysicalDeviceSurfacePresentModesKHR;
    PFN_vkAcquireNextImageKHR acquireNextImageKHR;
    PFN_vkQueuePresentKHR queuePresentKHR;
    PFN_vkQueueSubmit queueSubmit;
    PFN_vkCreateCommandPool createCommandPool;
    PFN_vkAllocateCommandBuffers allocateCommandBuffers;
    PFN_vkFreeCommandBuffers freeCommandBuffers;
    PFN_vkDestroyCommandPool destroyCommandPool;
    PFN_vkDestroySemaphore destroySemaphore;
    PFN_vkBeginCommandBuffer beginCommandBuffer;
    PFN_vkCmdPipelineBarrier cmdPipelineBarrier;
    PFN_vkCmdClearColorImage cmdClearColorImage;
    PFN_vkEndCommandBuffer endCommandBuffer;

    class KrautVK {

    private:

        static KrautCommon kraut;

        static int kvkInitGLFW(int width, int height, char *title, int fullScreen);

        static int kvkCheckExtensionAvailability(const char *extensionName, const std::vector<VkExtensionProperties> &availableExtensions);

        static void kvkGetRequiredDeviceExtensions(std::vector<const char*> &deviceExtensions);

        static int kvkCheckDeviceProperties(VkPhysicalDevice physicalDevice, uint32_t &selectedFamilyIndex, uint32_t &selectedPresentationCommandBuffer);

        static int kvkCreateInstance(const char *title);

        static int kvkCreateDevice();

        static int kvkCreateSemaphores();

        static bool kvkCreateSwapChain();

        static uint32_t kvkGetSwapChainNumImages(VkSurfaceCapabilitiesKHR surfaceCapabilities);

        static VkSurfaceFormatKHR kvkGetSwapChainFormat(std::vector<VkSurfaceFormatKHR> surfaceFormats);

        static VkExtent2D kvkGetSwapChainExtent(VkSurfaceCapabilitiesKHR surfaceCapabilities);

        static VkImageUsageFlags kvkGetSwapChainUsageFlags(VkSurfaceCapabilitiesKHR surfaceCapabilities);

        static VkSurfaceTransformFlagBitsKHR kvkGetSwapChainTransform(VkSurfaceCapabilitiesKHR surfaceCapabilities);

        static VkPresentModeKHR kvkGetSwapChainPresentMode(std::vector<VkPresentModeKHR> presentModes);

        static bool kvkCreateCommandBuffers();

        static int kvkInitVulkan(const char *title);

        static int kvkClear();

        static bool kvkOnWindowSizeChanged();

        static bool kvkRecordCommandBuffers();

    public:

        static int kvkInit(int w, int h, char *title, int f);

        static int kvkWindowShouldClose();

        static bool kvkRenderUpdate();

        static void kvkPollEvents();

        static void kvkTerminate();
    };

    EXPORT int init(int w, int h, char *title, int f);

    EXPORT int windowShouldClose();

    EXPORT void pollEvents();

    EXPORT void terminate();

    EXPORT void draw();
}

#endif