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

#include "KrautVKCommon.cpp"

//FUNCTION HEADERS
namespace KrautVK {

    //KRAUTVK VERSION
    uint32_t major = 0;
    uint32_t minor = 4;
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
    PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR getPhysicalDeviceSurfaceCapabilitiesKHR;
    PFN_vkGetPhysicalDeviceSurfaceFormatsKHR getPhysicalDeviceSurfaceFormatsKHR;
    PFN_vkGetPhysicalDeviceSurfacePresentModesKHR getPhysicalDeviceSurfacePresentModesKHR;

    PFN_vkGetDeviceProcAddr getDeviceProcAddr;
    PFN_vkGetDeviceQueue getDeviceQueue;
    PFN_vkDeviceWaitIdle deviceWaitIdle;
    PFN_vkDestroyDevice destroyDevice;
    PFN_vkDestroyImageView destroyImageView;
    PFN_vkCreateSemaphore createSemaphore;
    PFN_vkCreateSwapchainKHR createSwapchainKHR;
    PFN_vkCreateImageView createImageView;
    PFN_vkCreateRenderPass createRenderPass;
    PFN_vkCreateFramebuffer createFramebuffer;
    PFN_vkCreateShaderModule createShaderModule;
    PFN_vkDestroySwapchainKHR destroySwapchainKHR;
    PFN_vkDestroyShaderModule destroyShaderModule;
    PFN_vkGetSwapchainImagesKHR getSwapchainImagesKHR;
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
    PFN_vkEndCommandBuffer endCommandBuffer;
    PFN_vkCmdBeginRenderPass cmdBeginRenderPass;
    PFN_vkCmdBindPipeline cmdBindPipeline;
    PFN_vkCmdDraw cmdDraw;
    PFN_vkCmdEndRenderPass cmdEndRenderPass;
    PFN_vkDestroyPipeline destroyPipeline;
    PFN_vkDestroyRenderPass destroyRenderPass;
    PFN_vkDestroyFramebuffer destroyFramebuffer;
    PFN_vkCreateGraphicsPipelines createGraphicsPipelines;
    PFN_vkCreatePipelineLayout createPipelineLayout;
    PFN_vkDestroyPipelineLayout destroyPipelineLayout;

    class KrautVK {

    private:

        static int kvkInitGLFW(const int &width, const int &height, const char* title,const int &fullScreen);

        static int kvkCheckExtensionAvailability(const char* extensionName, const std::vector<VkExtensionProperties> &availableExtensions);

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

        static bool kvkCreateCommandBuffers(const uint32_t &width, const uint32_t &height);

        static int kvkInitVulkan(const char* &title, const uint32_t &width, const uint32_t &height);

        static int kvkCreateRenderPass();

        static int kvkCreateFrameBuffers(const uint32_t &width, const uint32_t &height);

        static void kvkClear();

        static bool kvkOnWindowSizeChanged(const int &width, const int &height);

        static bool kvkRecordCommandBuffers(const uint32_t &width, const uint32_t &height);

        static int kvkCreatePipelines(const uint32_t &width, const uint32_t &height);

        static GarbageCollector<VkPipelineLayout, PFN_vkDestroyPipelineLayout> kvkLoadPipelineLayout();

        static GarbageCollector<VkShaderModule, PFN_vkDestroyShaderModule> kvkLoadShader(std::string const &filename);

    public:

        static KrautCommon kraut;

        static int kvkInit(const int &w, const int &h, const char* title, const int &f);

        static int kvkWindowShouldClose();

        static bool kvkRenderUpdate();

        static void kvkPollEvents();

        static void kvkTerminate();
    };

    EXPORT int KrautInit(int w, int h, char* title, int f, char* dllPath);

    EXPORT int KrautWindowShouldClose();

    EXPORT void KrautPollEvents();

    EXPORT void KrautTerminate();

    EXPORT void KrautDraw();
}

#endif