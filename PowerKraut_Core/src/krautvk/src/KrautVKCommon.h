//
// Created by John Crockett on 6/3/2018.
//
#ifndef KRAUTVKCOMMON_H_
#define KRAUTVKCOMMON_H_

#include <cstdio>
#include <vector>
#include <iostream>
#include <cstring>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


struct QueueParameters {
    VkQueue                       Handle;
    uint32_t                      FamilyIndex;

    QueueParameters() :
            Handle( VK_NULL_HANDLE ),
            FamilyIndex( 0 ) {
    }
};

struct ImageParameters {
    VkImage                       Handle;
    VkImageView                   View;
    VkSampler                     Sampler;
    VkDeviceMemory                Memory;

    ImageParameters() :
            Handle( VK_NULL_HANDLE ),
            View( VK_NULL_HANDLE ),
            Sampler( VK_NULL_HANDLE ),
            Memory( VK_NULL_HANDLE ) {
    }
};

struct BufferParameters {
    VkBuffer                        Handle;
    VkDeviceMemory                  Memory;
    uint32_t                        Size;

    BufferParameters() :
            Handle( VK_NULL_HANDLE ),
            Memory( VK_NULL_HANDLE ),
            Size( 0 ) {
    }
};

struct DescriptorSetParameters {
    VkDescriptorPool                Pool;
    VkDescriptorSetLayout           Layout;
    VkDescriptorSet                 Handle;

    DescriptorSetParameters() :
            Pool( VK_NULL_HANDLE ),
            Layout( VK_NULL_HANDLE ),
            Handle( VK_NULL_HANDLE ) {
    }
};

struct SemaphoreParameters{
    VkSemaphore ImageAvailable;
    VkSemaphore RenderingFinished;

    SemaphoreParameters() :
            ImageAvailable(VK_NULL_HANDLE),
            RenderingFinished(VK_NULL_HANDLE){
    }
};

struct SwapChainParameters {
    VkSwapchainKHR                Handle;
    VkFormat                      Format;
    std::vector<ImageParameters>  Images;
    VkExtent2D                    Extent;

    SwapChainParameters() :
            Handle( VK_NULL_HANDLE ),
            Format( VK_FORMAT_UNDEFINED ),
            Images(),
            Extent() {
    }
};

struct CommandBufferParameters {
    VkCommandPool CommandPool;
    std::vector<VkCommandBuffer> CommandBuffers;

    CommandBufferParameters() :
            CommandPool(VK_NULL_HANDLE),
            CommandBuffers(0) {

    }

};

struct GLFWParameters {
    GLFWmonitor *Monitor;
    GLFWwindow *Window;
};

struct VulkanParameters {
    VkInstance                    Instance;
    VkPhysicalDevice              PhysicalDevice;
    VkDevice                      Device;
    VkSurfaceKHR                  ApplicationSurface;

    VulkanParameters() :
            Instance( VK_NULL_HANDLE ),
            PhysicalDevice( VK_NULL_HANDLE ),
            Device( VK_NULL_HANDLE ),
            ApplicationSurface( VK_NULL_HANDLE ) {
    }
};

struct KrautCommon {
    GLFWParameters GLFW;
    VulkanParameters Vulkan;
    SemaphoreParameters Semaphores;
    QueueParameters GraphicsQueue;
    QueueParameters PresentQueue;
    CommandBufferParameters PresentBuffer;
    SwapChainParameters SwapChain;

    KrautCommon() :
            GLFW(),
            Vulkan(),
            Semaphores(),
            GraphicsQueue(),
            PresentQueue(),
            SwapChain(),
            PresentBuffer() {

    }

};

#endif