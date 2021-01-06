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

#include "KrautVKConfig.h"
#include "KrautVKCommon.cpp"

//FUNCTION HEADERS
namespace KVKBase {

    class KrautVK {

    private:

        static int kvkInitGLFW(const int &width, const int &height, const char* title,const int &fullScreen);

        static int kvkCheckExtensionAvailability(const char* extensionName, const std::vector<VkExtensionProperties> &availableExtensions);

        static void kvkGetRequiredDeviceExtensions(std::vector<const char*> &deviceExtensions);

        static int kvkCheckDeviceProperties(VkPhysicalDevice physicalDevice, uint32_t &selectedFamilyIndex, uint32_t &selectedPresentationCommandBuffer);

        static int kvkCreateInstance(const char *title);

        static int kvkCreateDevice();

        static bool kvkCreateSwapChain();

        static uint32_t kvkGetSwapChainNumImages(VkSurfaceCapabilitiesKHR surfaceCapabilities);

        static VkSurfaceFormatKHR kvkGetSwapChainFormat(std::vector<VkSurfaceFormatKHR> surfaceFormats);

        static VkExtent2D kvkGetSwapChainExtent(VkSurfaceCapabilitiesKHR surfaceCapabilities);

        static VkImageUsageFlags kvkGetSwapChainUsageFlags(VkSurfaceCapabilitiesKHR surfaceCapabilities);

        static VkSurfaceTransformFlagBitsKHR kvkGetSwapChainTransform(VkSurfaceCapabilitiesKHR surfaceCapabilities);

        static VkPresentModeKHR kvkGetSwapChainPresentMode(std::vector<VkPresentModeKHR> presentModes);

        static int kvkCreateCommandPool();

        static int kvkCreateRenderPass();

        static bool kvkCreateFrameBuffers(VkFramebuffer &framebuffer, VkImageView imageView);

        static bool kvkOnWindowSizeChanged();

        static bool kvkRecordCommandBuffers(VkCommandBuffer commandBuffer, const Com::ImageParameters &imageParameters, VkFramebuffer &framebuffer);

        static int kvkCreatePipelines();

        static bool kvkCreatePipelineLayout();

        static bool kvkCreateBuffer(Com::BufferParameters &buffer, VkBufferCreateFlags usage, VkMemoryPropertyFlagBits memoryProperty);

        static int kvkCreateStagingBuffer();

        static int kvkCreateVertexBuffer();

        static int kvkCopyBufferToGPU();

        static bool kvkAllocateBufferMemory(VkBuffer buffer, VkMemoryPropertyFlagBits memoryProperty, VkDeviceMemory *memory);

        static int kvkAllocateCommandBuffer(VkCommandPool pool, uint32_t count, VkCommandBuffer *commandBuffer);

        static int kvkCreateSemaphore(VkSemaphore *semaphore);

        static int kvkCreateFence(VkFence *fence);

        static bool kvkCreateImage(const uint32_t &width, const uint32_t &height, VkImage *image);

        static bool kvkAllocateImageMemory(VkImage image, VkMemoryPropertyFlagBits property, VkDeviceMemory *memory);

        static int kvkCreateTexture(std::string relPath, Com::ImageParameters &image);

        static bool kvkCreateImageView(Com::ImageParameters &image, const VkFormat &format);

        static bool kvkCopyTextureToGPU(Com::ImageParameters &image, char* textureData, uint32_t dataSize, uint32_t width, uint32_t height);

        static bool kvkCreateSampler(VkSampler *sampler);

        static int kvkCreateDescriptorSet();

        static bool kvkLayoutDescriptorSet();

        static bool kvkCreateDescriptorPool();

        static bool kvkAllocateDescriptorSet();

        static void kvkUpdateDescriptorSet();

    public:

        static int kvkInit(const int &w, const int &h, const char* title, const int &f);

        static int kvkWindowShouldClose();

        static bool kvkRenderUpdate();

        static void kvkPollEvents();

        static void kvkTerminate();
    };
}

#endif