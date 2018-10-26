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

//SETTINGS
//__SHADERS & RASTER
#define KVK_CLEAR_COLOR             { 0.4f, 0.6f, 1.0f, 0.0f }
#define KVK_PRIMITIVE_TOPOLOGY      VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN
#define KVK_VERTEX_COUNT            (7)
#define KVK_INSTANCE_COUNT          (2)
#define KVK_CULL_MODE               VK_CULL_MODE_BACK_BIT
#define KVK_CULL_FRONT_FACE         VK_FRONT_FACE_COUNTER_CLOCKWISE

namespace KrautVK {

    class Tools{
    public :
        static std::string rootPath;

        static void findAndReplace(std::string& str, const std::string& find, const std::string& replace);

        static std::vector<char> getBinaryData(std::string const &filename);

        static std::vector<char> getImageData( std::string const &filename, int requestedComponents, int *width, int *height, int *components, int *dataSize );

        static std::array<float, 16> getProjMatrixPerspective(float const aspectRatio, float const fieldOfView, float const nearClip, float const farClip);

        static std::array<float, 16> getProjMatrixOrtho( float const leftPlane, float const rightPlane, float const topPlane, float const bottomPlane, float const nearPlane, float const farPlane );

    };

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
        std::vector<VkFramebuffer>    FrameBuffers;
        VkPhysicalDevice              PhysicalDevice;
        VkDevice                      Device;
        VkSurfaceKHR                  ApplicationSurface;
        VkRenderPass                  RenderPass;
        VkPipeline                    GraphicsPipeline;

        VulkanParameters() :
                Instance(VK_NULL_HANDLE),
                FrameBuffers(0),
                PhysicalDevice(VK_NULL_HANDLE),
                Device(VK_NULL_HANDLE),
                ApplicationSurface(VK_NULL_HANDLE),
                RenderPass(VK_NULL_HANDLE),
                GraphicsPipeline(VK_NULL_HANDLE){
        }
    };

    struct KrautCommon {
        GLFWParameters GLFW;
        VulkanParameters Vulkan;
        SemaphoreParameters Semaphores;
        QueueParameters GraphicsQueue;
        QueueParameters PresentQueue;
        CommandBufferParameters GraphicsBuffer;
        SwapChainParameters SwapChain;

        KrautCommon() :
                GLFW(),
                Vulkan(),
                Semaphores(),
                GraphicsQueue(),
                PresentQueue(),
                SwapChain(),
                GraphicsBuffer(){

        }

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
}

#endif