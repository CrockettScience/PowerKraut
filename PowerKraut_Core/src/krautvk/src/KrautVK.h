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


    class KrautVK {

    private:

        static int kvkInitGLFW(const int &width, const int &height, const char* title,const int &fullScreen);

        static int kvkCheckExtensionAvailability(const char* extensionName, const std::vector<VkExtensionProperties> &availableExtensions);

        static void kvkGetRequiredDeviceExtensions(std::vector<const char*> &deviceExtensions);

        static int kvkCheckDeviceProperties(VkPhysicalDevice physicalDevice, uint32_t &selectedFamilyIndex, uint32_t &selectedPresentationCommandBuffer);

        static int kvkCreateInstance(const char *title);

        static int kvkCreateDevice();

        static int kvkCreateSemaphores();

        static int kvkCreateFences();

        static bool kvkCreateSwapChain();

        static uint32_t kvkGetSwapChainNumImages(VkSurfaceCapabilitiesKHR surfaceCapabilities);

        static VkSurfaceFormatKHR kvkGetSwapChainFormat(std::vector<VkSurfaceFormatKHR> surfaceFormats);

        static VkExtent2D kvkGetSwapChainExtent(VkSurfaceCapabilitiesKHR surfaceCapabilities);

        static VkImageUsageFlags kvkGetSwapChainUsageFlags(VkSurfaceCapabilitiesKHR surfaceCapabilities);

        static VkSurfaceTransformFlagBitsKHR kvkGetSwapChainTransform(VkSurfaceCapabilitiesKHR surfaceCapabilities);

        static VkPresentModeKHR kvkGetSwapChainPresentMode(std::vector<VkPresentModeKHR> presentModes);

        static int kvkCreateCommandBuffers();

        static int kvkCreateRenderPass();

        static bool kvkCreateFrameBuffers(VkFramebuffer &framebuffer, VkImageView imageView);

        static bool kvkOnWindowSizeChanged();

        static bool kvkRecordCommandBuffers(VkCommandBuffer commandBuffer, const ImageParameters &imageParameters, VkFramebuffer &framebuffer);

        static int kvkCreatePipelines();

        static GarbageCollector<VkPipelineLayout, PFN_vkDestroyPipelineLayout> kvkLoadPipelineLayout();

        static GarbageCollector<VkShaderModule, PFN_vkDestroyShaderModule> kvkLoadShader(std::string const &filename);

        static int kvkCreateVertexBuffer();

        static bool kvkAllocateBufferMemory(VkBuffer buffer, VkDeviceMemory *memory);

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