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

#ifndef KRAUTVKCOMMON_H_
#define KRAUTVKCOMMON_H_

#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <fstream>
#include <cstring>
#include <array>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "STB/stb_image.h"

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
#define VULKAN_RENDERPASS_CREATION_FAILED (-8)
#define VULKAN_FRAMEBUFFERS_CREATION_FAILED (-9)
#define VULKAN_PIPELINES_CREATION_FAILED (-10)
#define VULKAN_VERTEX_CREATION_FAILED (-11)
#define VULKAN_FENCE_CREATION_FAILED (-12)
#define VULKAN_COMMAND_BUFFER_CREATION_FAILED (-13)

//SETTINGS
//__SHADERS & RASTER
#define KVK_CLEAR_COLOR             {1.0f, 0.6f, 1.0f, 0.0f}
#define KVK_PRIMITIVE_TOPOLOGY      VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP
#define KVK_VERTEX_COUNT            (4)
#define KVK_INSTANCE_COUNT          (1)
#define KVK_CULL_MODE               VK_CULL_MODE_BACK_BIT
#define KVK_CULL_FRONT_FACE         VK_FRONT_FACE_COUNTER_CLOCKWISE

//__RENDERING RESOURCES
#define KVK_RESOURCE_COUNT          (3)

namespace KrautVK {


    //KRAUTVK VERSION
    uint32_t major = 0;
    uint32_t minor = 5;
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
    PFN_vkCreateBuffer createBuffer;
    PFN_vkBindBufferMemory bindBufferMemory;
    PFN_vkMapMemory mapMemory;
    PFN_vkFlushMappedMemoryRanges flushMappedMemoryRanges;
    PFN_vkUnmapMemory unmapMemory;
    PFN_vkGetBufferMemoryRequirements getBufferMemoryRequirements;
    PFN_vkGetPhysicalDeviceMemoryProperties getPhysicalDeviceMemoryProperties;
    PFN_vkAllocateMemory allocateMemory;
    PFN_vkCreateFence createFence;
    PFN_vkDestroyFence destroyFence;
    PFN_vkDestroyBuffer destroyBuffer;
    PFN_vkFreeMemory freeMemory;
    PFN_vkResetFences resetFences;
    PFN_vkWaitForFences waitForFences;
    PFN_vkCmdSetViewport cmdSetViewport;
    PFN_vkCmdSetScissor cmdSetScissor;
    PFN_vkCmdBindVertexBuffers cmdBindVertexBuffers;

    class Tools{
    public :
        static std::string rootPath;

        static void findAndReplace(std::string& str, const std::string& find, const std::string& replace);

        static std::vector<char> getBinaryData(std::string const &filename);

        static std::vector<char> getImageData( std::string const &filename, int requestedComponents, int *width, int *height, int *components, int *dataSize );

        static std::array<float, 16> getProjMatrixPerspective(float const aspectRatio, float const fieldOfView, float const nearClip, float const farClip);

        static std::array<float, 16> getProjMatrixOrtho( float const leftPlane, float const rightPlane, float const topPlane, float const bottomPlane, float const nearPlane, float const farPlane );

    };

    template<class T, class F>
    class GarbageCollector {
    public:
        GarbageCollector() :
                Object(VK_NULL_HANDLE),
                Deleter(nullptr),
                Device(VK_NULL_HANDLE) {
        }

        GarbageCollector(T object, F deleter, VkDevice device) :
                Object(object),
                Deleter(deleter),
                Device(device) {
        }

        GarbageCollector(GarbageCollector&& other) {
            *this = std::move(other);
        }

        ~GarbageCollector() {
            if((Object != VK_NULL_HANDLE) && (Deleter != nullptr) && (Device != VK_NULL_HANDLE)) {
                Deleter(Device, Object, nullptr);
            }
        }

        GarbageCollector& operator=(GarbageCollector&& other) {
            if(this != &other) {
                Object = other.Object;
                Deleter = other.Deleter;
                Device = other.Device;
                other.Object = VK_NULL_HANDLE;
            }
            return *this;
        }

        T get() {
            return Object;
        }

        bool operator !() const {
            return Object == VK_NULL_HANDLE;
        }

    private:
        GarbageCollector(const GarbageCollector&);

        GarbageCollector& operator=(const GarbageCollector&);
        T         Object;
        F         Deleter;
        VkDevice  Device{};
    };

    //Struct to keep vertex attribute data to be passed into the shaders
    struct VertexData {
        float   x, y, z, w;             //Position
        float   r, g, b, a;             //Color
    };

    //Important structures to keep Vulkan Data
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
                Handle(VK_NULL_HANDLE),
                Memory(VK_NULL_HANDLE),
                Size(0) {
        }
    };

    struct DescriptorSetParameters {
        VkDescriptorPool                Pool;
        VkDescriptorSetLayout           Layout;
        VkDescriptorSet                 Handle;

        DescriptorSetParameters() :
                Pool(VK_NULL_HANDLE),
                Layout(VK_NULL_HANDLE),
                Handle(VK_NULL_HANDLE) {
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

    struct RenderingResourcesData {
        VkFramebuffer                         Framebuffer;
        VkCommandBuffer                       CommandBuffer;
        VkSemaphore                           ImageAvailableSemaphore;
        VkSemaphore                           FinishedRenderingSemaphore;
        VkFence                               Fence;

        RenderingResourcesData() :
                Framebuffer(VK_NULL_HANDLE),
                CommandBuffer(VK_NULL_HANDLE),
                ImageAvailableSemaphore(VK_NULL_HANDLE),
                FinishedRenderingSemaphore(VK_NULL_HANDLE),
                Fence(VK_NULL_HANDLE){
        }

        void DestroyRecources(const VkDevice &device, const VkCommandPool &pool);
    };

    struct GLFWParameters {
        GLFWmonitor *Monitor;
        GLFWwindow *Window;
    };

    struct VulkanParameters {
        VkInstance                              Instance;
        VkPhysicalDevice                        PhysicalDevice;
        VkDevice                                Device;
        VkSurfaceKHR                            ApplicationSurface;
        VkRenderPass                            RenderPass;
        VkPipeline                              GraphicsPipeline;
        SwapChainParameters                     SwapChain;
        std::vector<RenderingResourcesData>     RenderingResources;
        VkCommandPool                           CommandPool;

        static const size_t                     ResourceCount = KVK_RESOURCE_COUNT;

        VulkanParameters() :
                Instance(VK_NULL_HANDLE),
                PhysicalDevice(VK_NULL_HANDLE),
                Device(VK_NULL_HANDLE),
                ApplicationSurface(VK_NULL_HANDLE),
                RenderPass(VK_NULL_HANDLE),
                GraphicsPipeline(VK_NULL_HANDLE),
                SwapChain(),
                RenderingResources(ResourceCount),
                CommandPool(){
        }
    };

    struct KrautCommon {
        GLFWParameters              GLFW;
        VulkanParameters            Vulkan;
        QueueParameters             GraphicsQueue;
        QueueParameters             PresentQueue;
        BufferParameters            VertexBuffer;

        KrautCommon() :
                GLFW(),
                Vulkan(),
                GraphicsQueue(),
                PresentQueue(),
                VertexBuffer(){

        }

    };
}

#endif