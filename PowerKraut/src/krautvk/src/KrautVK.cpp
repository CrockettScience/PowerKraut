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

#include "KrautVK.h"

//Easier access to kraut resources
#define kraut Com::kraut

//Temporary hard-coded vertex data for alpha developement and testing
const std::vector<float> GlobalVertexData = {
        -1.0f, -1.0f, 0.0f, 1.0f,
        0.0f, 0.0f,
        //
        -1.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 1.0f,
        //
        1.0f, -1.0f, 0.0f, 1.0f,
        1.0f, 0.0f,
        //
        1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f,
};

namespace KVKBase {

    int KrautVK::kvkInitGLFW(const int &width, const int &height, const char* title, const int &fullScreen) {

        if (!glfwInit())
            return GLFW_INIT_FAILED;

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, false);

        kraut.GLFW.Monitor = glfwGetPrimaryMonitor();

        if (fullScreen)
            kraut.GLFW.Window = glfwCreateWindow(width, height, title, kraut.GLFW.Monitor, nullptr);
        else
            kraut.GLFW.Window = glfwCreateWindow(width, height, title, nullptr, nullptr);


        if (!kraut.GLFW.Window) {
            return GLFW_WINDOW_CREATION_FAILED;
        }

        glfwSetWindowSizeCallback(kraut.GLFW.Window, [](GLFWwindow *unusedWindow, int width, int height) {
            kvkOnWindowSizeChanged();
        });

        return SUCCESS;
    }

    int KrautVK::kvkCheckExtensionAvailability(const char* extensionName, const std::vector<VkExtensionProperties> &availableExtensions) {
        for (size_t i = 0; i < availableExtensions.size(); ++i) {
            if (strcmp(availableExtensions[i].extensionName, extensionName) == 0) {
                return true;
            }
        }
        return false;
    }

    void KrautVK::kvkGetRequiredDeviceExtensions(std::vector<const char *> &deviceExtensions) {
        deviceExtensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };
    }

    //This is where we set up our command buffers and queue families and select our device.
    //When updating system requirments, start here.
    int KrautVK::kvkCheckDeviceProperties(VkPhysicalDevice physicalDevice, uint32_t &selectedGraphicsCommandBuffer, uint32_t &selectedPresentationCommandBuffer) {
        uint32_t extensionsCount = 0;
        if ((enumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionsCount, nullptr) != VK_SUCCESS) ||
            (extensionsCount == 0)) {
            return false;
        }

        std::vector<VkExtensionProperties> availableExtensions(extensionsCount);
        if (enumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionsCount, &availableExtensions[0]) !=
            VK_SUCCESS) {
            return false;
        }

        std::vector<const char *> deviceExtensions;
        kvkGetRequiredDeviceExtensions(deviceExtensions);

        for (size_t i = 0; i < deviceExtensions.size(); ++i) {
            if (!kvkCheckExtensionAvailability(deviceExtensions[i], availableExtensions)) {
                return false;
            }
        }

        getPhysicalDeviceProperties(physicalDevice, &kraut.Vulkan.Device.Properties);
        getPhysicalDeviceFeatures(physicalDevice, &kraut.Vulkan.Device.Features);
        getPhysicalDeviceMemoryProperties(physicalDevice, &kraut.Vulkan.Device.MemoryProperties);

        uint32_t majorVersion = VK_VERSION_MAJOR(kraut.Vulkan.Device.Properties.apiVersion);
        uint32_t minorVersion = VK_VERSION_MINOR(kraut.Vulkan.Device.Properties.apiVersion);  //Reserved for when it's time to expect finer version requirements
        uint32_t patchVersion = VK_VERSION_PATCH(kraut.Vulkan.Device.Properties.apiVersion);

        if ((majorVersion < 1) || (kraut.Vulkan.Device.Properties.limits.maxImageDimension2D < 4096)) {
            return false;
        }

        uint32_t qFamilyCount = 0;
        getPhysicalDeviceQueueFamilyProperties(physicalDevice, &qFamilyCount, nullptr);
        if (qFamilyCount == 0) {
            return false;
        }

        std::vector<VkQueueFamilyProperties> qFamilyProperties(qFamilyCount);
        getPhysicalDeviceQueueFamilyProperties(physicalDevice, &qFamilyCount, qFamilyProperties.data());

        uint32_t currentGraphicsQueueFamilyIndex = UINT32_MAX;
        uint32_t currentPresentationQueueFamilyIndex = UINT32_MAX;

        //If a queue supports both graphics and presentation then use it. otherwise use separate queues.
        // If theres not a compatible buffer for either feature, the device is not compatible
        for (uint32_t i = 0; i < qFamilyCount; ++i) {
            if ((qFamilyProperties[i].queueCount > 0) && (qFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
                currentGraphicsQueueFamilyIndex == UINT32_MAX)
                currentGraphicsQueueFamilyIndex = i;

            if (glfwGetPhysicalDevicePresentationSupport(kraut.Vulkan.Instance, physicalDevice, i) &&
                currentPresentationQueueFamilyIndex == UINT32_MAX) {
                currentPresentationQueueFamilyIndex = i;
            }

            if ((currentGraphicsQueueFamilyIndex != UINT32_MAX && currentPresentationQueueFamilyIndex != UINT32_MAX)) {
                selectedGraphicsCommandBuffer = currentGraphicsQueueFamilyIndex;
                selectedPresentationCommandBuffer = currentPresentationQueueFamilyIndex;

                std::cout << "Selected Device: " << kraut.Vulkan.Device.Properties.deviceName << std::endl;
                return true;
            }
        }

        return false;
    }

    int KrautVK::kvkCreateInstance(const char *title) {

        //CREATE VULKAN INSTANCE
        if (!glfwVulkanSupported())
            return VULKAN_NOT_SUPPORTED;

        //initialize function pointers and create Vulkan instance
        createInstance = (PFN_vkCreateInstance) glfwGetInstanceProcAddress(nullptr, "vkCreateInstance");


        VkApplicationInfo applicationInfo = {
                VK_STRUCTURE_TYPE_APPLICATION_INFO,             // VkStructureType            sType
                nullptr,                                        // const void                *pNext
                title,                                          // const char                *pApplicationName
                VK_MAKE_VERSION(0, 0, 0),                       // uint32_t                   applicationVersion
                "KrautVK",                                      // const char                *pEngineName
                version,                                        // uint32_t                   engineVersion
                VK_API_VERSION_1_1                              // uint32_t                   apiVersion
        };

        //Ask GLFW what extensions are needed for Vulkan to operate, then load that list into the creation info
        //For now, i'm just going to assume glfw is gving me valid extensions without double checking
        //If anything wierd happens during instance creation, check this first.
        uint32_t count;
        const char **reqdExtensions = glfwGetRequiredInstanceExtensions(&count);

        VkInstanceCreateInfo instanceCreateInfo = {
                VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,         // VkStructureType            sType
                nullptr,                                        // const void*                pNext
                0,                                              // VkInstanceCreateFlags      flags
                &applicationInfo,                               // const VkApplicationInfo   *pApplicationInfo
                0,                                              // uint32_t                   enabledLayerCount
                nullptr,                                        // const char * const        *ppEnabledLayerNames
                count,                                          // uint32_t                   enabledExtensionCount
                reqdExtensions                                  // const char * const        *ppEnabledExtensionNames
        };

        if (createInstance(&instanceCreateInfo, nullptr, &kraut.Vulkan.Instance) != SUCCESS)
            return VULKAN_INSTANCE_CREATION_FAILED;

        createDevice = (PFN_vkCreateDevice)                                                         glfwGetInstanceProcAddress(kraut.Vulkan.Instance, "vkCreateDevice");
        enumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices)                                 glfwGetInstanceProcAddress(kraut.Vulkan.Instance, "vkEnumeratePhysicalDevices");
        getPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties)                           glfwGetInstanceProcAddress(kraut.Vulkan.Instance, "vkGetPhysicalDeviceProperties");
        getPhysicalDeviceFeatures = (PFN_vkGetPhysicalDeviceFeatures)                               glfwGetInstanceProcAddress(kraut.Vulkan.Instance, "vkGetPhysicalDeviceFeatures");
        getPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties)     glfwGetInstanceProcAddress(kraut.Vulkan.Instance, "vkGetPhysicalDeviceQueueFamilyProperties");
        destroyInstance = (PFN_vkDestroyInstance)                                                   glfwGetInstanceProcAddress(kraut.Vulkan.Instance, "vkDestroyInstance");
        destroySurfaceKHR = (PFN_vkDestroySurfaceKHR)                                               glfwGetInstanceProcAddress(kraut.Vulkan.Instance, "vkDestroySurfaceKHR");
        enumerateDeviceExtensionProperties = (PFN_vkEnumerateDeviceExtensionProperties)             glfwGetInstanceProcAddress(kraut.Vulkan.Instance, "vkEnumerateDeviceExtensionProperties");
        getPhysicalDeviceSurfaceCapabilitiesKHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR)   glfwGetInstanceProcAddress(kraut.Vulkan.Instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
        getPhysicalDeviceSurfaceFormatsKHR = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR)             glfwGetInstanceProcAddress(kraut.Vulkan.Instance, "vkGetPhysicalDeviceSurfaceFormatsKHR");
        getPhysicalDeviceSurfacePresentModesKHR = (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR)   glfwGetInstanceProcAddress(kraut.Vulkan.Instance, "vkGetPhysicalDeviceSurfacePresentModesKHR");
        getPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties)               glfwGetInstanceProcAddress(kraut.Vulkan.Instance, "vkGetPhysicalDeviceMemoryProperties");


        return SUCCESS;
    }

    int KrautVK::kvkCreateDevice() {
        if (glfwCreateWindowSurface(kraut.Vulkan.Instance, kraut.GLFW.Window, nullptr, &kraut.Vulkan.ApplicationSurface))
            return VULKAN_SURFACE_CREATION_FAILED;

        //INITIALIZE PHYSICAL DEVICES
        printf("Enumerating physical devices...\n");
        uint32_t deviceCount;
        if (enumeratePhysicalDevices(kraut.Vulkan.Instance, &deviceCount, nullptr) != SUCCESS || deviceCount == 0)
            return VULKAN_NOT_SUPPORTED;

        std::vector<VkPhysicalDevice> devices(deviceCount);
        if (enumeratePhysicalDevices(kraut.Vulkan.Instance, &deviceCount, &devices[0]) != SUCCESS)
            return VULKAN_NOT_SUPPORTED;

        uint32_t selectedGraphicsQueueFamilyIndex = UINT32_MAX;
        uint32_t selectedPresentationQueueFamilyIndex = UINT32_MAX;

        for (uint32_t i = 0; i < deviceCount; i++) {
            if (kvkCheckDeviceProperties(devices[i], selectedGraphicsQueueFamilyIndex,
                                         selectedPresentationQueueFamilyIndex)) {
                kraut.Vulkan.Device.PhysicalDevice = devices[i];
                break;
            }
        }

        if (kraut.Vulkan.Device.PhysicalDevice == nullptr)
            return VULKAN_NOT_SUPPORTED;

        std::vector<VkDeviceQueueCreateInfo> qCreateInfos;
        std::vector<float> queuePriorities = {1.0f};

        qCreateInfos.push_back({
                                       VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,     // VkStructureType              sType
                                       nullptr,                                        // const void                  *pNext
                                       0,                                              // VkDeviceQueueCreateFlags     flags
                                       selectedGraphicsQueueFamilyIndex,                  // uint32_t                     queueFamilyIndex
                                       static_cast<uint32_t>(queuePriorities.size()),  // uint32_t                     queueCount
                                       &queuePriorities[0]                             // const float                 *pQueuePriorities
                               });

        if (selectedGraphicsQueueFamilyIndex != selectedPresentationQueueFamilyIndex) {
            qCreateInfos.push_back({
                                           VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,     // VkStructureType              sType
                                           nullptr,                                        // const void                  *pNext
                                           0,                                              // VkDeviceQueueCreateFlags     flags
                                           selectedGraphicsQueueFamilyIndex,               // uint32_t                     queueFamilyIndex
                                           static_cast<uint32_t>(queuePriorities.size()),  // uint32_t                     queueCount
                                           &queuePriorities[0]                             // const float                 *pQueuePriorities
                                   });
        }

        std::vector<const char *> extensions;
        kvkGetRequiredDeviceExtensions(extensions);

        VkDeviceCreateInfo deviceCreateInfo = {
                VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,           // VkStructureType                    sType
                nullptr,                                        // const void                        *pNext
                0,                                              // VkDeviceCreateFlags                flags
                static_cast<uint32_t>(qCreateInfos.size()),     // uint32_t                           queueCreateInfoCount
                &qCreateInfos[0],                               // const VkDeviceQueueCreateInfo     *pQueueCreateInfos
                0,                                              // uint32_t                           enabledLayerCount
                nullptr,                                        // const char * const                *ppEnabledLayerNames
                static_cast<uint32_t>(extensions.size()),       // uint32_t                           enabledExtensionCount
                &extensions[0],                                 // const char * const                *ppEnabledExtensionNames
                nullptr                                         // const VkPhysicalDeviceFeatures    *pEnabledFeatures
        };

        if (createDevice(kraut.Vulkan.Device.PhysicalDevice, &deviceCreateInfo, nullptr, &kraut.Vulkan.Device.Handle) != SUCCESS)
            return VULKAN_DEVICE_CREATION_FAILED;

        getDeviceProcAddr = (PFN_vkGetDeviceProcAddr) glfwGetInstanceProcAddress(kraut.Vulkan.Instance, "vkGetDeviceProcAddr");

        getDeviceQueue = (PFN_vkGetDeviceQueue)                                         getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkGetDeviceQueue");
        deviceWaitIdle = (PFN_vkDeviceWaitIdle)                                         getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkDeviceWaitIdle");
        destroyDevice = (PFN_vkDestroyDevice)                                           getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkDestroyDevice");
        createSemaphore = (PFN_vkCreateSemaphore)                                       getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkCreateSemaphore");
        createSwapchainKHR = (PFN_vkCreateSwapchainKHR)                                 getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkCreateSwapchainKHR");
        destroySwapchainKHR = (PFN_vkDestroySwapchainKHR)                               getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkDestroySwapchainKHR");
        getSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR)                           getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkGetSwapchainImagesKHR");
        acquireNextImageKHR = (PFN_vkAcquireNextImageKHR)                               getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkAcquireNextImageKHR");
        queuePresentKHR = (PFN_vkQueuePresentKHR)                                       getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkQueuePresentKHR");
        queueSubmit = (PFN_vkQueueSubmit)                                               getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkQueueSubmit");
        createCommandPool = (PFN_vkCreateCommandPool)                                   getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkCreateCommandPool");
        allocateCommandBuffers = (PFN_vkAllocateCommandBuffers)                         getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkAllocateCommandBuffers");
        freeCommandBuffers = (PFN_vkFreeCommandBuffers)                                 getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkFreeCommandBuffers");
        destroyCommandPool = (PFN_vkDestroyCommandPool)                                 getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkDestroyCommandPool");
        destroySemaphore = (PFN_vkDestroySemaphore)                                     getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkDestroySemaphore");
        beginCommandBuffer = (PFN_vkBeginCommandBuffer)                                 getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkBeginCommandBuffer");
        cmdPipelineBarrier = (PFN_vkCmdPipelineBarrier)                                 getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkCmdPipelineBarrier");
        endCommandBuffer = (PFN_vkEndCommandBuffer)                                     getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkEndCommandBuffer");
        destroyImageView = (PFN_vkDestroyImageView)                                     getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkDestroyImageView");
        createImageView = (PFN_vkCreateImageView)                                       getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkCreateImageView");
        createRenderPass = (PFN_vkCreateRenderPass)                                     getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkCreateRenderPass");
        createFramebuffer = (PFN_vkCreateFramebuffer)                                   getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkCreateFramebuffer");
        createShaderModule = (PFN_vkCreateShaderModule)                                 getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkCreateShaderModule");
        destroyShaderModule = (PFN_vkDestroyShaderModule)                               getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkDestroyShaderModule");
        cmdBeginRenderPass = (PFN_vkCmdBeginRenderPass)                                 getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkCmdBeginRenderPass");
        cmdBindPipeline = (PFN_vkCmdBindPipeline)                                       getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkCmdBindPipeline");
        cmdDraw = (PFN_vkCmdDraw)                                                       getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkCmdDraw");
        cmdEndRenderPass = (PFN_vkCmdEndRenderPass)                                     getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkCmdEndRenderPass");
        destroyPipeline = (PFN_vkDestroyPipeline)                                       getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkDestroyPipeline");
        destroyRenderPass = (PFN_vkDestroyRenderPass)                                   getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkDestroyRenderPass");
        destroyFramebuffer = (PFN_vkDestroyFramebuffer)                                 getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkDestroyFramebuffer");
        createGraphicsPipelines = (PFN_vkCreateGraphicsPipelines)                       getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkCreateGraphicsPipelines");
        createPipelineLayout = (PFN_vkCreatePipelineLayout)                             getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkCreatePipelineLayout");
        destroyPipelineLayout = (PFN_vkDestroyPipelineLayout)                           getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkDestroyPipelineLayout");
        createBuffer = (PFN_vkCreateBuffer)                                             getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkCreateBuffer");
        bindBufferMemory = (PFN_vkBindBufferMemory)                                     getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkBindBufferMemory");
        mapMemory = (PFN_vkMapMemory)                                                   getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkMapMemory");
        flushMappedMemoryRanges = (PFN_vkFlushMappedMemoryRanges)                       getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkFlushMappedMemoryRanges");
        unmapMemory = (PFN_vkUnmapMemory)                                               getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkUnmapMemory");
        getBufferMemoryRequirements = (PFN_vkGetBufferMemoryRequirements)               getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkGetBufferMemoryRequirements");
        allocateMemory = (PFN_vkAllocateMemory)                                         getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkAllocateMemory");
        createFence = (PFN_vkCreateFence)                                               getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkCreateFence");
        destroyFence = (PFN_vkDestroyFence)                                             getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkDestroyFence");
        destroyBuffer = (PFN_vkDestroyBuffer)                                           getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkDestroyBuffer");
        freeMemory = (PFN_vkFreeMemory)                                                 getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkFreeMemory");
        resetFences = (PFN_vkResetFences)                                               getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkResetFences");
        waitForFences = (PFN_vkWaitForFences)                                           getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkWaitForFences");
        cmdSetViewport = (PFN_vkCmdSetViewport)                                         getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkCmdSetViewport");
        cmdSetScissor = (PFN_vkCmdSetScissor)                                           getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkCmdSetScissor");
        cmdBindVertexBuffers = (PFN_vkCmdBindVertexBuffers)                             getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkCmdBindVertexBuffers");
        cmdCopyBuffer = (PFN_vkCmdCopyBuffer)                                           getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkCmdCopyBuffer");
        bindImageMemory = (PFN_vkBindImageMemory)                                       getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkBindImageMemory");
        createSampler = (PFN_vkCreateSampler)                                           getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkCreateSampler");
        cmdCopyBufferToImage = (PFN_vkCmdCopyBufferToImage)                             getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkCmdCopyBufferToImage");
        createImage = (PFN_vkCreateImage)                                               getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkCreateImage");
        getImageMemoryRequirements = (PFN_vkGetImageMemoryRequirements)                 getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkGetImageMemoryRequirements");
        createDescriptorSetLayout = (PFN_vkCreateDescriptorSetLayout)                   getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkCreateDescriptorSetLayout");
        createDescriptorPool = (PFN_vkCreateDescriptorPool)                             getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkCreateDescriptorPool");
        allocateDescriptorSets = (PFN_vkAllocateDescriptorSets)                         getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkAllocateDescriptorSets");
        updateDescriptorSets = (PFN_vkUpdateDescriptorSets)                             getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkUpdateDescriptorSets");
        cmdBindDescriptorSets = (PFN_vkCmdBindDescriptorSets)                           getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkCmdBindDescriptorSets");
        destroyDescriptorPool = (PFN_vkDestroyDescriptorPool)                           getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkDestroyDescriptorPool");
        destroyDescriptorSetLayout = (PFN_vkDestroyDescriptorSetLayout)                 getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkDestroyDescriptorSetLayout");
        destroySampler = (PFN_vkDestroySampler)                                         getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkDestroySampler");
        destroyImage = (PFN_vkDestroyImage)                                             getDeviceProcAddr(kraut.Vulkan.Device.Handle, "vkDestroyImage");

        //INITIALIZE COMMAND BUFFER
        kraut.GraphicsQueue.FamilyIndex = selectedGraphicsQueueFamilyIndex;
        kraut.PresentQueue.FamilyIndex = selectedPresentationQueueFamilyIndex;

        getDeviceQueue(kraut.Vulkan.Device.Handle, kraut.GraphicsQueue.FamilyIndex, 0, &kraut.GraphicsQueue.Handle);
        getDeviceQueue(kraut.Vulkan.Device.Handle, kraut.PresentQueue.FamilyIndex, 0, &kraut.PresentQueue.Handle);

        return SUCCESS;
    }

    bool KrautVK::kvkCreateSwapChain() {

        if (kraut.Vulkan.Device.Handle != VK_NULL_HANDLE) {
            deviceWaitIdle(kraut.Vulkan.Device.Handle);
        }

        for(size_t i = 0; i < kraut.Vulkan.SwapChain.Images.size(); ++i) {
            if(kraut.Vulkan.SwapChain.Images[i].View != VK_NULL_HANDLE) {
                destroyImageView(kraut.Vulkan.Device.Handle, kraut.Vulkan.SwapChain.Images[i].View, nullptr);
                kraut.Vulkan.SwapChain.Images[i].View = VK_NULL_HANDLE;
            }
        }
        kraut.Vulkan.SwapChain.Images.clear();

        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        if (getPhysicalDeviceSurfaceCapabilitiesKHR(kraut.Vulkan.Device.PhysicalDevice, kraut.Vulkan.ApplicationSurface, &surfaceCapabilities) != VK_SUCCESS) {
            return false;
        }

        uint32_t formatsCount;
        if ((getPhysicalDeviceSurfaceFormatsKHR(kraut.Vulkan.Device.PhysicalDevice, kraut.Vulkan.ApplicationSurface, &formatsCount, nullptr) != VK_SUCCESS) ||
            (formatsCount == 0)) {
            return false;
        }

        std::vector<VkSurfaceFormatKHR> surfaceFormats(formatsCount);
        if (getPhysicalDeviceSurfaceFormatsKHR(kraut.Vulkan.Device.PhysicalDevice, kraut.Vulkan.ApplicationSurface, &formatsCount, surfaceFormats.data()) != VK_SUCCESS) {
            return false;
        }

        uint32_t presentModesCount;
        if ((getPhysicalDeviceSurfacePresentModesKHR(kraut.Vulkan.Device.PhysicalDevice, kraut.Vulkan.ApplicationSurface, &presentModesCount, nullptr) != VK_SUCCESS) ||
            (presentModesCount == 0)) {
            return false;
        }

        std::vector<VkPresentModeKHR> presentModes(presentModesCount);
        if (getPhysicalDeviceSurfacePresentModesKHR(kraut.Vulkan.Device.PhysicalDevice, kraut.Vulkan.ApplicationSurface,
                                                    &presentModesCount, presentModes.data()) != VK_SUCCESS) {
            return false;
        }

        uint32_t desiredNumberOfImages = kvkGetSwapChainNumImages(surfaceCapabilities);
        VkSurfaceFormatKHR desiredFormat = kvkGetSwapChainFormat(surfaceFormats);
        VkExtent2D desiredExtent = kvkGetSwapChainExtent(surfaceCapabilities);
        VkImageUsageFlags desiredUsage = kvkGetSwapChainUsageFlags(surfaceCapabilities);
        VkSurfaceTransformFlagBitsKHR desiredTransform = kvkGetSwapChainTransform(surfaceCapabilities);
        VkPresentModeKHR desiredPresentMode = kvkGetSwapChainPresentMode(presentModes);
        VkSwapchainKHR oldSwapChain = kraut.Vulkan.SwapChain.Handle;

        if (static_cast<int>(desiredUsage) == -1) {
            return false;
        }

        if (static_cast<int>(desiredPresentMode) == -1) {
            return false;
        }

        if ((desiredExtent.width == 0) || (desiredExtent.height == 0)) {
            // Some asshole minimized the window, but I guess it's fine
            return SUCCESS;
        }

        VkSwapchainCreateInfoKHR swapChainCreateInfo = {
                VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,    // VkStructureType                sType
                nullptr,                                        // const void                    *pNext
                0,                                              // VkSwapchainCreateFlagsKHR      flags
                kraut.Vulkan.ApplicationSurface,                // VkSurfaceKHR                   surface
                desiredNumberOfImages,                          // uint32_t                       minImageCount
                desiredFormat.format,                           // VkFormat                       imageFormat
                desiredFormat.colorSpace,                       // VkColorSpaceKHR                imageColorSpace
                desiredExtent,                                  // VkExtent2D                     imageExtent
                1,                                              // uint32_t                       imageArrayLayers
                desiredUsage,                                   // VkImageUsageFlags              imageUsage
                VK_SHARING_MODE_EXCLUSIVE,                      // VkSharingMode                  imageSharingMode
                0,                                              // uint32_t                       queueFamilyIndexCount
                nullptr,                                        // const uint32_t                *pQueueFamilyIndices
                desiredTransform,                               // VkSurfaceTransformFlagBitsKHR  preTransform
                VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,              // VkCompositeAlphaFlagBitsKHR    compositeAlpha
                desiredPresentMode,                             // VkPresentModeKHR               presentMode
                VK_TRUE,                                        // VkBool32                       clipped
                oldSwapChain                                    // VkSwapchainKHR                 oldSwapchain
        };

        if (createSwapchainKHR(kraut.Vulkan.Device.Handle, &swapChainCreateInfo, nullptr, &kraut.Vulkan.SwapChain.Handle) !=
            VK_SUCCESS) {
            return false;
        }

        kraut.Vulkan.SwapChain.Extent = desiredExtent;
        kraut.Vulkan.SwapChain.Format = desiredFormat.format;

        if (oldSwapChain != VK_NULL_HANDLE) {
            destroySwapchainKHR(kraut.Vulkan.Device.Handle, oldSwapChain, nullptr);
        }

        uint32_t imageCount = 0;
        if((getSwapchainImagesKHR( kraut.Vulkan.Device.Handle, kraut.Vulkan.SwapChain.Handle, &imageCount, nullptr ) != VK_SUCCESS) ||
            (imageCount == 0) ) {
            std::cout << "Could not get swap chain images!" << std::endl;
            return false;
        }
        kraut.Vulkan.SwapChain.Images.resize( imageCount );

        std::vector<VkImage> images( imageCount );
        if(getSwapchainImagesKHR(kraut.Vulkan.Device.Handle, kraut.Vulkan.SwapChain.Handle, &imageCount, images.data() ) != VK_SUCCESS ) {
            std::cout << "Could not get swap chain images!" << std::endl;
            return false;
        }

        for( size_t i = 0; i < kraut.Vulkan.SwapChain.Images.size(); ++i ) {
            kraut.Vulkan.SwapChain.Images[i].Handle = images[i];
        }

        for (size_t i = 0; i < kraut.Vulkan.SwapChain.Images.size(); ++i) {

            if(!kvkCreateImageView(kraut.Vulkan.SwapChain.Images[i], kraut.Vulkan.SwapChain.Format))
                return false;

        }

        return true;
    }

    uint32_t KrautVK::kvkGetSwapChainNumImages(VkSurfaceCapabilitiesKHR surfaceCapabilities) {
        // Set of images defined in a swap chain may not always be available for application to render to:
        // One may be displayed and one may wait in a queue to be presented
        // If application wants to use more images at the same time it must ask for more images
        uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
        if ((surfaceCapabilities.maxImageCount > 0) &&
            (imageCount > surfaceCapabilities.maxImageCount)) {
            imageCount = surfaceCapabilities.maxImageCount;
        }
        return imageCount;
    }

    VkSurfaceFormatKHR KrautVK::kvkGetSwapChainFormat(std::vector<VkSurfaceFormatKHR> surfaceFormats) {
        // If the list contains only one entry with undefined format
        // it means that there are no preferred surface formats and any can be chosen
        if ((surfaceFormats.size() == 1) &&
            (surfaceFormats[0].format == VK_FORMAT_UNDEFINED)) {
            return {VK_FORMAT_R8G8B8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR};
        }

        // Check if list contains most widely used R8 G8 B8 A8 format
        // with nonlinear color space
        for (VkSurfaceFormatKHR &surfaceFormat : surfaceFormats) {
            if (surfaceFormat.format == VK_FORMAT_R8G8B8A8_UNORM) {
                return surfaceFormat;

            }
        }

        // Return the first format from the list
        return surfaceFormats[0];
    }

    VkExtent2D KrautVK::kvkGetSwapChainExtent(VkSurfaceCapabilitiesKHR surfaceCapabilities) {
        // Special value of surface extent is width == height == -1
        // If this is so we define the size by ourselves but it must fit within defined confines
        if (surfaceCapabilities.currentExtent.width == -1) {
            VkExtent2D swapChainExtent = {640, 480};
            if (swapChainExtent.width < surfaceCapabilities.minImageExtent.width) {
                swapChainExtent.width = surfaceCapabilities.minImageExtent.width;
            }
            if (swapChainExtent.height < surfaceCapabilities.minImageExtent.height) {
                swapChainExtent.height = surfaceCapabilities.minImageExtent.height;
            }
            if (swapChainExtent.width > surfaceCapabilities.maxImageExtent.width) {
                swapChainExtent.width = surfaceCapabilities.maxImageExtent.width;
            }
            if (swapChainExtent.height > surfaceCapabilities.maxImageExtent.height) {
                swapChainExtent.height = surfaceCapabilities.maxImageExtent.height;
            }
            return swapChainExtent;
        }

        // Most of the cases we define size of the swap_chain images equal to current window's size
        return surfaceCapabilities.currentExtent;
    }

    // This is where we will get technical with our surface capabilities. Right now theres a color attachment and a
    // transfer destination attachment, for screen clearing. We'll need to tweak if we go 3D (There's currently no depth attachment)
    VkImageUsageFlags KrautVK::kvkGetSwapChainUsageFlags(VkSurfaceCapabilitiesKHR surfaceCapabilities) {
        // Color attachment flag must always be supported
        // We can define other usage flags but we always need to check if they are supported
        if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
            return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        }

        std::cout << "VK_IMAGE_USAGE_TRANSFER_DST image usage is not supported by the swap chain!" << std::endl
                  << "Supported swap chain's image usages include:" << std::endl
                  << (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT
                      ? "    VK_IMAGE_USAGE_TRANSFER_SRC\n" : "")
                  << (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT
                      ? "    VK_IMAGE_USAGE_TRANSFER_DST\n" : "")
                  << (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT
                      ? "    VK_IMAGE_USAGE_SAMPLED\n" : "")
                  << (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_STORAGE_BIT
                      ? "    VK_IMAGE_USAGE_STORAGE\n" : "")
                  << (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
                      ? "    VK_IMAGE_USAGE_COLOR_ATTACHMENT\n" : "")
                  << (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
                      ? "    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT\n" : "")
                  << (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT
                      ? "    VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT\n" : "")
                  << (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
                      ? "    VK_IMAGE_USAGE_INPUT_ATTACHMENT" : "")
                  << std::endl;

        return static_cast<VkImageUsageFlags> (-1);
    }

    //No need to fuck with this unless we want to add support for "rotateable" devices
    VkSurfaceTransformFlagBitsKHR KrautVK::kvkGetSwapChainTransform(VkSurfaceCapabilitiesKHR surfaceCapabilities) {
        // Sometimes images must be transformed before they are presented (i.e. due to device's orienation
        // being other than default orientation)
        // If the specified transform is other than current transform, presentation engine will transform image
        // during presentation operation; this operation may hit performance on some platforms
        // Here we don't want any transformations to occur so if the identity transform is supported use it
        // otherwise just use the same transform as current transform
        if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
            return VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        } else {
            return surfaceCapabilities.currentTransform;
        }
    }

    VkPresentModeKHR KrautVK::kvkGetSwapChainPresentMode(std::vector<VkPresentModeKHR> presentModes) {
        // FIFO present mode is always available
        // MAILBOX is the lowest latency V-Sync enabled mode (something like triple-buffering) so use it if available
        for (VkPresentModeKHR &presentMode : presentModes) {
            if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return presentMode;
            }
        }
        for (VkPresentModeKHR &presentMode : presentModes) {
            if (presentMode == VK_PRESENT_MODE_FIFO_KHR) {
                return presentMode;
            }
        }

        return static_cast<VkPresentModeKHR>(-1);
    }

    int KrautVK::kvkCreateCommandPool(){
        VkCommandPoolCreateInfo cmdPoolCreateInfo = {
                VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,                                                 // VkStructureType              sType
                nullptr,                                                                                    // const void*                  pNext
                VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,     // VkCommandPoolCreateFlags     flags
                kraut.PresentQueue.FamilyIndex                                                              // uint32_t                     queueFamilyIndex
        };

        if (createCommandPool(kraut.Vulkan.Device.Handle, &cmdPoolCreateInfo, nullptr, &kraut.Vulkan.CommandPool) != VK_SUCCESS) {
            return VULKAN_COMMAND_BUFFER_CREATION_FAILED;
        }

        return SUCCESS;
    }

    bool KrautVK::kvkRecordCommandBuffers(VkCommandBuffer commandBuffer, const Com::ImageParameters &imageParameters, VkFramebuffer &framebuffer) {
        if(!kvkCreateFrameBuffers(framebuffer, imageParameters.View)) {
            return false;
        }

        VkCommandBufferBeginInfo commandBufferBeginInfo = {
                VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,        // VkStructureType                        sType
                nullptr,                                            // const void                            *pNext
                VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,        // VkCommandBufferUsageFlags              flags
                nullptr                                             // const VkCommandBufferInheritanceInfo  *pInheritanceInfo
        };

        beginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

        VkImageSubresourceRange imageSubresourceRange = {
                VK_IMAGE_ASPECT_COLOR_BIT,                          // VkImageAspectFlags                     aspectMask
                0,                                                  // uint32_t                               baseMipLevel
                1,                                                  // uint32_t                               levelCount
                0,                                                  // uint32_t                               baseArrayLayer
                1                                                   // uint32_t                               layerCount
        };

        if(kraut.PresentQueue.Handle != kraut.GraphicsQueue.Handle) {
            VkImageMemoryBarrier barrierFromPresentToDraw = {
                    VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,           // VkStructureType                        sType
                    nullptr,                                          // const void                            *pNext
                    VK_ACCESS_MEMORY_READ_BIT,                        // VkAccessFlags                          srcAccessMask
                    VK_ACCESS_MEMORY_READ_BIT,                        // VkAccessFlags                          dstAccessMask
                    VK_IMAGE_LAYOUT_UNDEFINED,                        // VkImageLayout                          oldLayout
                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,                  // VkImageLayout                          newLayout
                    kraut.PresentQueue.FamilyIndex,                   // uint32_t                               srcQueueFamilyIndex
                    kraut.GraphicsQueue.FamilyIndex,                  // uint32_t                               dstQueueFamilyIndex
                    imageParameters.Handle,                           // VkImage                                image
                    imageSubresourceRange                             // VkImageSubresourceRange                subresourceRange
            };
            cmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrierFromPresentToDraw);
        }

        VkClearValue clearValue = {
                KVK_CLEAR_COLOR,                         // VkClearColorValue                      color
        };

        VkRenderPassBeginInfo renderPassBeginInfo = {
                VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,           // VkStructureType                        sType
                nullptr,                                            // const void                            *pNext
                kraut.Vulkan.RenderPass,                                  // VkRenderPass                           renderPass
                framebuffer,                                        // VkFramebuffer                          framebuffer
                {                                                   // VkRect2D                               renderArea
                        {                                                 // VkOffset2D                             offset
                                0,                                                // int32_t                                x
                                0                                                 // int32_t                                y
                        },
                        kraut.Vulkan.SwapChain.Extent,                            // VkExtent2D                             extent;
                },
                1,                                                  // uint32_t                               clearValueCount
                &clearValue                                        // const VkClearValue                    *pClearValues
        };

        cmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        cmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, kraut.Vulkan.GraphicsPipeline);

        VkViewport viewport = {
                0.0f,                                               // float                                  x
                0.0f,                                               // float                                  y
                static_cast<float>(kraut.Vulkan.SwapChain.Extent.width),    // float                                  width
                static_cast<float>(kraut.Vulkan.SwapChain.Extent.height),   // float                                  height
                0.0f,                                               // float                                  minDepth
                1.0f                                                // float                                  maxDepth
        };

        VkRect2D scissor = {
                {                                                   // VkOffset2D                             offset
                        0,                                                  // int32_t                                x
                        0                                                   // int32_t                                y
                },
                {                                                   // VkExtent2D                             extent
                        kraut.Vulkan.SwapChain.Extent.width,                // uint32_t                               width
                        kraut.Vulkan.SwapChain.Extent.height                // uint32_t                               height
                }
        };

        cmdSetViewport(commandBuffer, 0, 1, &viewport);
        cmdSetScissor(commandBuffer, 0, 1, &scissor);

        VkDeviceSize offset = 0;
        cmdBindVertexBuffers(commandBuffer, 0, 1, &kraut.DemoResources.VertexBuffer.Handle, &offset);

        cmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, kraut.Vulkan.PipelineLayout, 0, 1, &kraut.Vulkan.Descriptor.Handle, 0, nullptr);

        cmdDraw(commandBuffer, KVK_VERTEX_COUNT, KVK_INSTANCE_COUNT, 0, 0);

        cmdEndRenderPass(commandBuffer);

        if(kraut.GraphicsQueue.Handle != kraut.PresentQueue.Handle ) {
            VkImageMemoryBarrier barrierFromDrawToPresent = {
                    VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,           // VkStructureType                        sType
                    nullptr,                                          // const void                            *pNext
                    VK_ACCESS_MEMORY_READ_BIT,                        // VkAccessFlags                          srcAccessMask
                    VK_ACCESS_MEMORY_READ_BIT,                        // VkAccessFlags                          dstAccessMask
                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,                  // VkImageLayout                          oldLayout
                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,                  // VkImageLayout                          newLayout
                    kraut.GraphicsQueue.FamilyIndex,                   // uint32_t                               srcQueueFamilyIndex
                    kraut.PresentQueue.FamilyIndex,                    // uint32_t                               dstQueueFamilyIndex
                    imageParameters.Handle,                          // VkImage                                image
                    imageSubresourceRange                           // VkImageSubresourceRange                subresourceRange
            };
            cmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrierFromDrawToPresent);
        }

        return endCommandBuffer(commandBuffer) == VK_SUCCESS;

    }

    bool KrautVK::kvkOnWindowSizeChanged() {

        return kvkCreateSwapChain();

    }

    int KrautVK::kvkWindowShouldClose() {
        return glfwWindowShouldClose(kraut.GLFW.Window);

    }

    bool KrautVK::kvkRenderUpdate() {

        static size_t           resourceIndex = 0;
        Com::RenderingResourcesData &currentRenderingResource = kraut.Vulkan.RenderingResources[resourceIndex];
        VkSwapchainKHR          swapchain = kraut.Vulkan.SwapChain.Handle;
        uint32_t                imageIndex;

        resourceIndex = (resourceIndex + 1) % Com::VulkanParameters::ResourceCount;

        if(waitForFences(kraut.Vulkan.Device.Handle, 1, &currentRenderingResource.Fence, VK_FALSE, 1000000000) != VK_SUCCESS) {
            std::cout << "Fence Time Out!" << std::endl;
            return false;
        }

        resetFences(kraut.Vulkan.Device.Handle, 1, &currentRenderingResource.Fence);

        VkResult result = acquireNextImageKHR(kraut.Vulkan.Device.Handle, swapchain, UINT64_MAX, currentRenderingResource.ImageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
        switch(result) {
            case VK_SUCCESS:
            case VK_SUBOPTIMAL_KHR:
                break;
            case VK_ERROR_OUT_OF_DATE_KHR:
                return kvkOnWindowSizeChanged();
            default:
                std::cout << "Swap chain image aqcisition failure!" << std::endl;
                return false;
        }

        if(!kvkRecordCommandBuffers(currentRenderingResource.CommandBuffer, kraut.Vulkan.SwapChain.Images[imageIndex], currentRenderingResource.Framebuffer)) {
            return false;
        }

        VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo submitInfo = {
                VK_STRUCTURE_TYPE_SUBMIT_INFO,                          // VkStructureType              sType
                nullptr,                                                // const void                  *pNext
                1,                                                      // uint32_t                     waitSemaphoreCount
                &currentRenderingResource.ImageAvailableSemaphore,      // const VkSemaphore           *pWaitSemaphores
                &waitDstStageMask,                                      // const VkPipelineStageFlags  *pWaitDstStageMask;
                1,                                                      // uint32_t                     commandBufferCount
                &currentRenderingResource.CommandBuffer,                // const VkCommandBuffer       *pCommandBuffers
                1,                                                      // uint32_t                     signalSemaphoreCount
                &currentRenderingResource.FinishedRenderingSemaphore    // const VkSemaphore           *pSignalSemaphores
        };

        if(queueSubmit(kraut.GraphicsQueue.Handle, 1, &submitInfo, currentRenderingResource.Fence) != VK_SUCCESS) {
            return false;
        }

        VkPresentInfoKHR presentInfo = {
                VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,                     // VkStructureType              sType
                nullptr,                                                // const void                  *pNext
                1,                                                      // uint32_t                     waitSemaphoreCount
                &currentRenderingResource.FinishedRenderingSemaphore,   // const VkSemaphore           *pWaitSemaphores
                1,                                                      // uint32_t                     swapchainCount
                &swapchain,                                             // const VkSwapchainKHR        *pSwapchains
                &imageIndex,                                            // const uint32_t              *pImageIndices
                nullptr                                                 // VkResult                    *pResults
        };
        result = queuePresentKHR(kraut.PresentQueue.Handle, &presentInfo);

        switch( result ) {
            case VK_SUCCESS:
                break;
            case VK_ERROR_OUT_OF_DATE_KHR:
            case VK_SUBOPTIMAL_KHR:
                return kvkOnWindowSizeChanged();
            default:
                std::cout << "Image presentation failure!" << std::endl;
                return false;
        }

        return true;

    }

    void KrautVK::kvkPollEvents() {
        glfwPollEvents();
    }

    int KrautVK::kvkCreateRenderPass() {
        VkAttachmentDescription attachmentDescription[] = {
                {
                        0,                                   // VkAttachmentDescriptionFlags   flags
                        kraut.Vulkan.SwapChain.Format,              // VkFormat                       format
                        VK_SAMPLE_COUNT_1_BIT,               // VkSampleCountFlagBits          samples
                        VK_ATTACHMENT_LOAD_OP_CLEAR,         // VkAttachmentLoadOp             loadOp
                        VK_ATTACHMENT_STORE_OP_STORE,        // VkAttachmentStoreOp            storeOp
                        VK_ATTACHMENT_LOAD_OP_DONT_CARE,     // VkAttachmentLoadOp             stencilLoadOp
                        VK_ATTACHMENT_STORE_OP_DONT_CARE,    // VkAttachmentStoreOp            stencilStoreOp
                        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,     // VkImageLayout                  initialLayout;
                        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR      // VkImageLayout                  finalLayout
                }
        };

        VkAttachmentReference colorAttachmentReferences[] = {
                {
                        0,                                          // uint32_t                       attachment
                        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL    // VkImageLayout                  layout
                }
        };

        VkSubpassDescription subpassDescriptions[] = {
                {
                        0,                                          // VkSubpassDescriptionFlags      flags
                        VK_PIPELINE_BIND_POINT_GRAPHICS,            // VkPipelineBindPoint            pipelineBindPoint
                        0,                                          // uint32_t                       inputAttachmentCount
                        nullptr,                                    // const VkAttachmentReference   *pInputAttachments
                        1,                                          // uint32_t                       colorAttachmentCount
                        colorAttachmentReferences,                  // const VkAttachmentReference   *pColorAttachments
                        nullptr,                                    // const VkAttachmentReference   *pResolveAttachments
                        nullptr,                                    // const VkAttachmentReference   *pDepthStencilAttachment
                        0,                                          // uint32_t                       preserveAttachmentCount
                        nullptr                                     // const uint32_t*                pPreserveAttachments
                }
        };

        std::vector<VkSubpassDependency> dependencies = {
                {
                        VK_SUBPASS_EXTERNAL,                            // uint32_t                       srcSubpass
                        0,                                              // uint32_t                       dstSubpass
                        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,           // VkPipelineStageFlags           srcStageMask
                        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  // VkPipelineStageFlags           dstStageMask
                        VK_ACCESS_MEMORY_READ_BIT,                      // VkAccessFlags                  srcAccessMask
                        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,           // VkAccessFlags                  dstAccessMask
                        VK_DEPENDENCY_BY_REGION_BIT                     // VkDependencyFlags              dependencyFlags
                },

                {
                        0,                                              // uint32_t                       srcSubpass
                        VK_SUBPASS_EXTERNAL,                            // uint32_t                       dstSubpass
                        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  // VkPipelineStageFlags           srcStageMask
                        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,           // VkPipelineStageFlags           dstStageMask
                        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,           // VkAccessFlags                  srcAccessMask
                        VK_ACCESS_MEMORY_READ_BIT,                      // VkAccessFlags                  dstAccessMask
                        VK_DEPENDENCY_BY_REGION_BIT                     // VkDependencyFlags              dependencyFlags
                }
        };

        VkRenderPassCreateInfo renderPassCreateInfo = {
                VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,    // VkStructureType                sType
                nullptr,                                      // const void                    *pNext
                0,                                            // VkRenderPassCreateFlags        flags
                1,                                            // uint32_t                       attachmentCount
                attachmentDescription,                        // const VkAttachmentDescription *pAttachments
                1,                                            // uint32_t                       subpassCount
                subpassDescriptions,                          // const VkSubpassDescription    *pSubpasses
                static_cast<uint32_t>(dependencies.size()),   // uint32_t                       dependencyCount
                &dependencies[0]                              // const VkSubpassDependency     *pDependencies
        };

        if(createRenderPass(kraut.Vulkan.Device.Handle, &renderPassCreateInfo, nullptr, &kraut.Vulkan.RenderPass) == VK_SUCCESS){
            return SUCCESS;
        } else{
            return VULKAN_RENDERPASS_CREATION_FAILED;
        }


    }

    bool KrautVK::kvkCreateFrameBuffers(VkFramebuffer &framebuffer, VkImageView imageView) {

        if(framebuffer != VK_NULL_HANDLE) {
            destroyFramebuffer(kraut.Vulkan.Device.Handle, framebuffer, nullptr);
            framebuffer = VK_NULL_HANDLE;
        }

        VkFramebufferCreateInfo framebufferCreateInfo = {
                VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,      // VkStructureType                sType
                nullptr,                                        // const void                    *pNext
                0,                                              // VkFramebufferCreateFlags       flags
                kraut.Vulkan.RenderPass,                              // VkRenderPass                   renderPass
                1,                                              // uint32_t                       attachmentCount
                &imageView,                                    // const VkImageView             *pAttachments
                kraut.Vulkan.SwapChain.Extent.width,                    // uint32_t                       width
                kraut.Vulkan.SwapChain.Extent.height,                   // uint32_t                       height
                1                                               // uint32_t                       layers
        };

        if(createFramebuffer(kraut.Vulkan.Device.Handle, &framebufferCreateInfo, nullptr, &framebuffer) != VK_SUCCESS ) {
            std::cout << "Could not create a framebuffer!" << std::endl;
            return false;
        }

        return true;
    }

    int KrautVK::kvkCreatePipelines(){
        GarbageCollector<VkShaderModule, PFN_vkDestroyShaderModule> vertexShaderModule = Tools::loadShader(Tools::rootPath + std::string("/data/shadervert.spv"));
        GarbageCollector<VkShaderModule, PFN_vkDestroyShaderModule> fragmentShaderModule = Tools::loadShader(Tools::rootPath + std::string("/data/shaderfrag.spv"));

        if( !vertexShaderModule || !fragmentShaderModule ) {
            return VULKAN_PIPELINES_CREATION_FAILED;
        }

        VkVertexInputBindingDescription vertexBindingDescription = {
                    0,                                  // uint32_t            binding
                    sizeof(VertexData),                 // uint32_t            stride
                    VK_VERTEX_INPUT_RATE_VERTEX         // VkVertexInputRate   inputRate
        };

        VkVertexInputAttributeDescription vertexAttributeDescriptions[] = {
                {
                        0,                                          // uint32_t    location
                        vertexBindingDescription.binding,           // uint32_t    binding
                        VK_FORMAT_R32G32B32A32_SFLOAT,             // VkFormat    format
                        0                                          // uint32_t    offset
                },
                {
                        1,                                         // uint32_t    location
                        vertexBindingDescription.binding,           // uint32_t    binding
                        VK_FORMAT_R32G32B32A32_SFLOAT,             // VkFormat    format
                        4 * sizeof(float)                          // uint32_t    offset
                }
        };

        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {
                VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,      // VkStructureType                                sType
                nullptr,                                                        // const void                                    *pNext
                0,                                                              // VkPipelineVertexInputStateCreateFlags          flags;
                1,                                                              // uint32_t                                       vertexBindingDescriptionCount
                &vertexBindingDescription,                                      // const VkVertexInputBindingDescription         *pVertexBindingDescriptions
                2,                                                              // uint32_t                                       vertexAttributeDescriptionCount
                vertexAttributeDescriptions                                     // const VkVertexInputAttributeDescription       *pVertexAttributeDescriptions
        };

        //Load Shaders
        std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos = {
                // Vertex shader
                {
                        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,        // VkStructureType                                sType
                        nullptr,                                                    // const void                                    *pNext
                        0,                                                          // VkPipelineShaderStageCreateFlags               flags
                        VK_SHADER_STAGE_VERTEX_BIT,                                 // VkShaderStageFlagBits                          stage
                        vertexShaderModule.get(),                                   // VkShaderModule                                 module
                        "main",                                                     // const char                                    *pName
                        nullptr                                                     // const VkSpecializationInfo                    *pSpecializationInfo
                },

                // Fragment shader
                {
                        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,        // VkStructureType                                sType
                        nullptr,                                                    // const void                                    *pNext
                        0,                                                          // VkPipelineShaderStageCreateFlags               flags
                        VK_SHADER_STAGE_FRAGMENT_BIT,                               // VkShaderStageFlagBits                          stage
                        fragmentShaderModule.get(),                                 // VkShaderModule                                 module
                        "main",                                                     // const char                                    *pName
                        nullptr                                                     // const VkSpecializationInfo                    *pSpecializationInfo
                }
        };

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {
                VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,  // VkStructureType                                sType
                nullptr,                                                      // const void                                    *pNext
                0,                                                            // VkPipelineInputAssemblyStateCreateFlags        flags
                KVK_PRIMITIVE_TOPOLOGY,                                       // VkPrimitiveTopology                            topology
                VK_FALSE                                                      // VkBool32                                       primitiveRestartEnable
        };

        VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {
                VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,        // VkStructureType                                sType
                nullptr,                                                      // const void                                    *pNext
                0,                                                            // VkPipelineViewportStateCreateFlags             flags
                1,                                                            // uint32_t                                       viewportCount
                nullptr,                                                      // const VkViewport                              *pViewports
                1,                                                            // uint32_t                                       scissorCount
                nullptr                                                       // const VkRect2D                                *pScissors
        };

        VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {
                VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,   // VkStructureType                                sType
                nullptr,                                                      // const void                                    *pNext
                0,                                                            // VkPipelineRasterizationStateCreateFlags        flags
                VK_FALSE,                                                     // VkBool32                                       depthClampEnable
                VK_FALSE,                                                     // VkBool32                                       rasterizerDiscardEnable
                VK_POLYGON_MODE_FILL,                                         // VkPolygonMode                                  polygonMode
                KVK_CULL_MODE,                                                // VkCullModeFlags                                cullMode
                KVK_CULL_FRONT_FACE,                                          // VkFrontFace                                    frontFace
                VK_FALSE,                                                     // VkBool32                                       depthBiasEnable
                0.0f,                                                         // float                                          depthBiasConstantFactor
                0.0f,                                                         // float                                          depthBiasClamp
                0.0f,                                                         // float                                          depthBiasSlopeFactor
                1.0f                                                          // float                                          lineWidth
        };

        VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {
                VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,     // VkStructureType                                sType
                nullptr,                                                      // const void                                    *pNext
                0,                                                            // VkPipelineMultisampleStateCreateFlags          flags
                VK_SAMPLE_COUNT_1_BIT,                                        // VkSampleCountFlagBits                          rasterizationSamples
                VK_FALSE,                                                     // VkBool32                                       sampleShadingEnable
                1.0f,                                                         // float                                          minSampleShading
                nullptr,                                                      // const VkSampleMask                            *pSampleMask
                VK_FALSE,                                                     // VkBool32                                       alphaToCoverageEnable
                VK_FALSE                                                      // VkBool32                                       alphaToOneEnable
        };

        VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {
                VK_FALSE,                                                     // VkBool32                                       blendEnable
                VK_BLEND_FACTOR_ONE,                                          // VkBlendFactor                                  srcColorBlendFactor
                VK_BLEND_FACTOR_ZERO,                                         // VkBlendFactor                                  dstColorBlendFactor
                VK_BLEND_OP_ADD,                                              // VkBlendOp                                      colorBlendOp
                VK_BLEND_FACTOR_ONE,                                          // VkBlendFactor                                  srcAlphaBlendFactor
                VK_BLEND_FACTOR_ZERO,                                         // VkBlendFactor                                  dstAlphaBlendFactor
                VK_BLEND_OP_ADD,                                              // VkBlendOp                                      alphaBlendOp
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |         // VkColorComponentFlags                          colorWriteMask
                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
        };

        VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {
                VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,     // VkStructureType                                sType
                nullptr,                                                      // const void                                    *pNext
                0,                                                            // VkPipelineColorBlendStateCreateFlags           flags
                VK_FALSE,                                                     // VkBool32                                       logicOpEnable
                VK_LOGIC_OP_COPY,                                             // VkLogicOp                                      logicOp
                1,                                                            // uint32_t                                       attachmentCount
                &colorBlendAttachmentState,                                   // const VkPipelineColorBlendAttachmentState     *pAttachments
                { 0.0f, 0.0f, 0.0f, 0.0f }                                    // float                                          blendConstants[4]
        };

        if(!kvkCreatePipelineLayout()) {
            return VULKAN_PIPELINES_CREATION_FAILED;
        }

        std::vector<VkDynamicState> dynamicStates = {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR,
        };

        VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo = {
                VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,         // VkStructureType                                sType
                nullptr,                                                      // const void                                    *pNext
                0,                                                            // VkPipelineDynamicStateCreateFlags              flags
                static_cast<uint32_t>(dynamicStates.size()),                  // uint32_t                                       dynamicStateCount
                &dynamicStates[0]                                             // const VkDynamicState                          *pDynamicStates
        };


        VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
                VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,                // VkStructureType                                sType
                nullptr,                                                        // const void                                    *pNext
                0,                                                              // VkPipelineCreateFlags                          flags
                static_cast<uint32_t>(shaderStageCreateInfos.size()),           // uint32_t                                       stageCount
                &shaderStageCreateInfos[0],                                     // const VkPipelineShaderStageCreateInfo         *pStages
                &vertexInputStateCreateInfo,                                    // const VkPipelineVertexInputStateCreateInfo    *pVertexInputState;
                &inputAssemblyStateCreateInfo,                                  // const VkPipelineInputAssemblyStateCreateInfo  *pInputAssemblyState
                nullptr,                                                        // const VkPipelineTessellationStateCreateInfo   *pTessellationState
                &viewportStateCreateInfo,                                       // const VkPipelineViewportStateCreateInfo       *pViewportState
                &rasterizationStateCreateInfo,                                  // const VkPipelineRasterizationStateCreateInfo  *pRasterizationState
                &multisampleStateCreateInfo,                                    // const VkPipelineMultisampleStateCreateInfo    *pMultisampleState
                nullptr,                                                        // const VkPipelineDepthStencilStateCreateInfo   *pDepthStencilState
                &colorBlendStateCreateInfo,                                     // const VkPipelineColorBlendStateCreateInfo     *pColorBlendState
                &pipelineDynamicStateCreateInfo,                                // const VkPipelineDynamicStateCreateInfo        *pDynamicState
                kraut.Vulkan.PipelineLayout,                                    // VkPipelineLayout                               layout
                kraut.Vulkan.RenderPass,                                        // VkRenderPass                                   renderPass
                0,                                                              // uint32_t                                       subpass
                VK_NULL_HANDLE,                                                 // VkPipeline                                     basePipelineHandle
                -1                                                              // int32_t                                        basePipelineIndex
        };


        if(createGraphicsPipelines(kraut.Vulkan.Device.Handle, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &kraut.Vulkan.GraphicsPipeline) == VK_SUCCESS){
            return SUCCESS;

        } else{
            return VULKAN_PIPELINES_CREATION_FAILED;

        };

    }

    bool KrautVK::kvkCreatePipelineLayout(){

        VkPipelineLayoutCreateInfo layoutCreateInfo = {
                VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,  // VkStructureType                sType
                nullptr,                                        // const void                    *pNext
                0,                                              // VkPipelineLayoutCreateFlags    flags
                1,                                              // uint32_t                       setLayoutCount
                &kraut.Vulkan.Descriptor.Layout,                // const VkDescriptorSetLayout   *pSetLayouts
                0,                                              // uint32_t                       pushConstantRangeCount
                nullptr                                         // const VkPushConstantRange     *pPushConstantRanges
        };

        if(createPipelineLayout(kraut.Vulkan.Device.Handle, &layoutCreateInfo, nullptr, &kraut.Vulkan.PipelineLayout) != VK_SUCCESS) {
            return false;
        }

        return true;

    }

    bool KrautVK::kvkAllocateBufferMemory(VkBuffer buffer, VkMemoryPropertyFlagBits memoryProperty, VkDeviceMemory *memory) {
        VkMemoryRequirements bufferMemoryRequirements;
        getBufferMemoryRequirements(kraut.Vulkan.Device.Handle, buffer, &bufferMemoryRequirements);

        for(uint32_t i = 0; i < kraut.Vulkan.Device.MemoryProperties.memoryTypeCount; ++i) {
            if((bufferMemoryRequirements.memoryTypeBits & (1 << i)) &&
                (kraut.Vulkan.Device.MemoryProperties.memoryTypes[i].propertyFlags & memoryProperty)) {

                VkMemoryAllocateInfo memory_allocate_info = {
                        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,     // VkStructureType                        sType
                        nullptr,                                    // const void                            *pNext
                        bufferMemoryRequirements.size,              // VkDeviceSize                           allocationSize
                        i                                           // uint32_t                               memoryTypeIndex
                };

                if(allocateMemory(kraut.Vulkan.Device.Handle, &memory_allocate_info, nullptr, memory) == VK_SUCCESS) {
                    return true;
                }
            }
        }
        return false;

    }

    int KrautVK::kvkAllocateCommandBuffer(VkCommandPool pool, uint32_t count, VkCommandBuffer *commandBuffer) {

        VkCommandBufferAllocateInfo cmdBufferAllocateInfo = {
                VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, // VkStructureType              sType
                nullptr,                                        // const void*                  pNext
                pool,                                           // VkCommandPool                commandPool
                VK_COMMAND_BUFFER_LEVEL_PRIMARY,                // VkCommandBufferLevel         level
                count                                           // uint32_t                     bufferCount
        };

        if (allocateCommandBuffers(kraut.Vulkan.Device.Handle, &cmdBufferAllocateInfo, commandBuffer) != VK_SUCCESS) {
            return VULKAN_COMMAND_BUFFER_CREATION_FAILED;
        }

        return SUCCESS;
    }

    int KrautVK::kvkCreateSemaphore(VkSemaphore *semaphore) {

        VkSemaphoreCreateInfo semaphoreCreateInfo = {
                VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,      // VkStructureType          sType
                nullptr,                                      // const void*              pNext
                0                                             // VkSemaphoreCreateFlags   flags
        };

            if ((createSemaphore(kraut.Vulkan.Device.Handle, &semaphoreCreateInfo, nullptr, semaphore) != VK_SUCCESS)) {
                return VULKAN_SEMAPHORE_CREATION_FAILED;
            }

        return SUCCESS;
    }

    int KrautVK::kvkCreateFence(VkFence *fence) {
        VkFenceCreateInfo fenceCreateInfo = {
                VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,              // VkStructureType                sType
                nullptr,                                          // const void                    *pNext
                VK_FENCE_CREATE_SIGNALED_BIT                      // VkFenceCreateFlags             flags
        };

        if(createFence(kraut.Vulkan.Device.Handle, &fenceCreateInfo, nullptr, fence) != VK_SUCCESS) {
            return VULKAN_FENCE_CREATION_FAILED;
        }

        return SUCCESS;

    }

    bool KrautVK::kvkCreateImage(const uint32_t &width, const uint32_t &height, VkImage *image) {
        VkImageCreateInfo imageCreateInfo = {
                VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,  // VkStructureType        sType;
                nullptr,                              // const void            *pNext
                0,                                    // VkImageCreateFlags     flags
                VK_IMAGE_TYPE_2D,                     // VkImageType            imageType
                VK_FORMAT_R8G8B8A8_UNORM,             // VkFormat               format
                {                                     // VkExtent3D             extent

                        width,                        // uint32_t               width
                        height,                       // uint32_t               height
                        1                             // uint32_t               depth
                },
                1,                                    // uint32_t               mipLevels
                1,                                    // uint32_t               arrayLayers
                VK_SAMPLE_COUNT_1_BIT,                // VkSampleCountFlagBits  samples
                VK_IMAGE_TILING_OPTIMAL,              // VkImageTiling          tiling
                VK_IMAGE_USAGE_TRANSFER_DST_BIT |     // VkImageUsageFlags      usage
                VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_SHARING_MODE_EXCLUSIVE,            // VkSharingMode          sharingMode
                0,                                    // uint32_t               queueFamilyIndexCount
                nullptr,                              // const uint32_t        *pQueueFamilyIndices
                VK_IMAGE_LAYOUT_UNDEFINED             // VkImageLayout          initialLayout
        };

        return createImage(kraut.Vulkan.Device.Handle, &imageCreateInfo, nullptr, image) == VK_SUCCESS;

    }

    bool KrautVK::kvkAllocateImageMemory(VkImage image, VkMemoryPropertyFlagBits property, VkDeviceMemory *memory) {
        // Get the memory requirements from the device
        VkMemoryRequirements imageMemoryRequirements;
        getImageMemoryRequirements(kraut.Vulkan.Device.Handle, image, &imageMemoryRequirements);

        //Find the right memory type that is compatible with our needs and is supported by the hardware
        for(uint32_t i = 0; i < kraut.Vulkan.Device.MemoryProperties.memoryTypeCount; ++i) {
            if((imageMemoryRequirements.memoryTypeBits & (1 << i)) && (kraut.Vulkan.Device.MemoryProperties.memoryTypes[i].propertyFlags & property)) {

                VkMemoryAllocateInfo memoryAllocateInfo = {
                        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, // VkStructureType  sType
                        nullptr,                                // const void      *pNext
                        imageMemoryRequirements.size,           // VkDeviceSize     allocationSize
                        i                                       // uint32_t         memoryTypeIndex
                };

                if(allocateMemory(kraut.Vulkan.Device.Handle, &memoryAllocateInfo, nullptr, memory) == VK_SUCCESS ) {
                    return true;
                }
            }
        }
        return false;


    }

    int KrautVK::kvkCreateTexture(const std::string relPath, Com::ImageParameters &image) {
        int width = 0;
        int height = 0;
        int dataSize = 0;

        std::vector<char> textureData = Tools::getImageData(Tools::rootPath + relPath, 4, &width, &height, nullptr, &dataSize);

        if(!kvkCreateImage(static_cast<const uint32_t &>(width), static_cast<const uint32_t &>(height), &image.Handle))
            return VULKAN_TEXTURE_CREATION_FAILED;

        if(!kvkAllocateImageMemory(image.Handle, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &image.Memory))
            return VULKAN_TEXTURE_CREATION_FAILED;

        if(bindImageMemory(kraut.Vulkan.Device.Handle, image.Handle, image.Memory, 0) != VK_SUCCESS)
            return VULKAN_TEXTURE_CREATION_FAILED;

        if(!kvkCreateImageView(image, VK_FORMAT_R8G8B8A8_UNORM))
            return VULKAN_TEXTURE_CREATION_FAILED;

        if(!kvkCreateSampler(&image.Sampler))
            return VULKAN_TEXTURE_CREATION_FAILED;

        if(!kvkCopyTextureToGPU(image, textureData.data(), static_cast<uint32_t>(dataSize), static_cast<uint32_t>(width), static_cast<uint32_t>(height))){
            return VULKAN_TEXTURE_CREATION_FAILED;
        }

        return SUCCESS;

    }

    bool KrautVK::kvkCreateImageView(Com::ImageParameters &image, const VkFormat &format) {
        VkImageViewCreateInfo imageViewCreateInfo = {
                VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, // VkStructureType          sType
                nullptr,                                  // const void              *pNext
                0,                                        // VkImageViewCreateFlags   flags
                image.Handle,                             // VkImage                  image
                VK_IMAGE_VIEW_TYPE_2D,                    // VkImageViewType          viewType
                format,                                   // VkFormat                 format
                {                                         // VkComponentMapping       components
                        VK_COMPONENT_SWIZZLE_IDENTITY,            // VkComponentSwizzle       r
                        VK_COMPONENT_SWIZZLE_IDENTITY,            // VkComponentSwizzle       g
                        VK_COMPONENT_SWIZZLE_IDENTITY,            // VkComponentSwizzle       b
                        VK_COMPONENT_SWIZZLE_IDENTITY             // VkComponentSwizzle       a
                },
                {                                                 // VkImageSubresourceRange  subresourceRange
                        VK_IMAGE_ASPECT_COLOR_BIT,                // VkImageAspectFlags       aspectMask
                        0,                                        // uint32_t                 baseMipLevel
                        1,                                        // uint32_t                 levelCount
                        0,                                        // uint32_t                 baseArrayLayer
                        1                                         // uint32_t                 layerCount
                }
        };

        return createImageView(kraut.Vulkan.Device.Handle, &imageViewCreateInfo, nullptr, &image.View ) == VK_SUCCESS;

    }

    bool KrautVK::kvkCopyTextureToGPU(Com::ImageParameters &image, char* textureData, uint32_t dataSize, uint32_t width, uint32_t height) {

        void *stagingBufferMemoryPointer;
        if(mapMemory(kraut.Vulkan.Device.Handle, kraut.StagingBuffer.Memory, 0, dataSize, 0, &stagingBufferMemoryPointer) != VK_SUCCESS) {
            return false;
        }

        memcpy(stagingBufferMemoryPointer, textureData, dataSize);

        VkMappedMemoryRange flushRange = {
                VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,              // VkStructureType                        sType
                nullptr,                                            // const void                            *pNext
                kraut.StagingBuffer.Memory,                         // VkDeviceMemory                         memory
                0,                                                  // VkDeviceSize                           offset
                dataSize                                            // VkDeviceSize                           size
        };
        flushMappedMemoryRanges(kraut.Vulkan.Device.Handle, 1, &flushRange);

        unmapMemory(kraut.Vulkan.Device.Handle, kraut.StagingBuffer.Memory);

        // Prepare command buffer to copy data from staging buffer to a vertex buffer
        VkCommandBufferBeginInfo commandBufferBeginInfo = {
                VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,        // VkStructureType                        sType
                nullptr,                                            // const void                            *pNext
                VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,        // VkCommandBufferUsageFlags              flags
                nullptr                                             // const VkCommandBufferInheritanceInfo  *pInheritanceInfo
        };

        VkCommandBuffer commandBuffer = kraut.Vulkan.RenderingResources[0].CommandBuffer;

        beginCommandBuffer( commandBuffer, &commandBufferBeginInfo);

        VkImageSubresourceRange imageSubresourceRange = {
                VK_IMAGE_ASPECT_COLOR_BIT,                          // VkImageAspectFlags                     aspectMask
                0,                                                  // uint32_t                               baseMipLevel
                1,                                                  // uint32_t                               levelCount
                0,                                                  // uint32_t                               baseArrayLayer
                1                                                   // uint32_t                               layerCount
        };

        VkImageMemoryBarrier imageMemoryBarrierFromUndefinedToTransferDst = {
                VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,             // VkStructureType                        sType
                nullptr,                                            // const void                            *pNext
                0,                                                  // VkAccessFlags                          srcAccessMask
                VK_ACCESS_TRANSFER_WRITE_BIT,                       // VkAccessFlags                          dstAccessMask
                VK_IMAGE_LAYOUT_UNDEFINED,                          // VkImageLayout                          oldLayout
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,               // VkImageLayout                          newLayout
                VK_QUEUE_FAMILY_IGNORED,                            // uint32_t                               srcQueueFamilyIndex
                VK_QUEUE_FAMILY_IGNORED,                            // uint32_t                               dstQueueFamilyIndex
                image.Handle,                   // VkImage                                image
                imageSubresourceRange                               // VkImageSubresourceRange                subresourceRange
        };
        cmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrierFromUndefinedToTransferDst);

        VkBufferImageCopy bufferImageCopyInfo = {
                0,                                                  // VkDeviceSize                           bufferOffset
                0,                                                  // uint32_t                               bufferRowLength
                0,                                                  // uint32_t                               bufferImageHeight
                {                                                   // VkImageSubresourceLayers               imageSubresource
                        VK_IMAGE_ASPECT_COLOR_BIT,                          // VkImageAspectFlags                     aspectMask
                        0,                                                  // uint32_t                               mipLevel
                        0,                                                  // uint32_t                               baseArrayLayer
                        1                                                   // uint32_t                               layerCount
                },
                {                                                   // VkOffset3D                             imageOffset
                        0,                                                  // int32_t                                x
                        0,                                                  // int32_t                                y
                        0                                                   // int32_t                                z
                },
                {                                                   // VkExtent3D                             imageExtent
                        width,                                              // uint32_t                               width
                        height,                                             // uint32_t                               height
                        1                                                   // uint32_t                               depth
                }
        };
        cmdCopyBufferToImage(commandBuffer, kraut.StagingBuffer.Handle, image.Handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopyInfo);

        VkImageMemoryBarrier imageMemoryBarrierFromTransferToShaderRead = {
                VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,             // VkStructureType                        sType
                nullptr,                                            // const void                            *pNext
                VK_ACCESS_TRANSFER_WRITE_BIT,                       // VkAccessFlags                          srcAccessMask
                VK_ACCESS_SHADER_READ_BIT,                          // VkAccessFlags                          dstAccessMask
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,               // VkImageLayout                          oldLayout
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,           // VkImageLayout                          newLayout
                VK_QUEUE_FAMILY_IGNORED,                            // uint32_t                               srcQueueFamilyIndex
                VK_QUEUE_FAMILY_IGNORED,                            // uint32_t                               dstQueueFamilyIndex
                image.Handle,                   // VkImage                                image
                imageSubresourceRange                               // VkImageSubresourceRange                subresourceRange
        };
        cmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrierFromTransferToShaderRead);

        endCommandBuffer(commandBuffer);

        // Submit command buffer and copy data from staging buffer to a vertex buffer
        VkSubmitInfo submitInfo = {
                VK_STRUCTURE_TYPE_SUBMIT_INFO,                      // VkStructureType                        sType
                nullptr,                                            // const void                            *pNext
                0,                                                  // uint32_t                               waitSemaphoreCount
                nullptr,                                            // const VkSemaphore                     *pWaitSemaphores
                nullptr,                                            // const VkPipelineStageFlags            *pWaitDstStageMask;
                1,                                                  // uint32_t                               commandBufferCount
                &commandBuffer,                                     // const VkCommandBuffer                 *pCommandBuffers
                0,                                                  // uint32_t                               signalSemaphoreCount
                nullptr                                             // const VkSemaphore                     *pSignalSemaphores
        };

        if(queueSubmit(kraut.GraphicsQueue.Handle, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
            return false;
        }

        deviceWaitIdle(kraut.Vulkan.Device.Handle);

        return true;
    }

    bool KrautVK::kvkCreateSampler(VkSampler *sampler) {
        VkSamplerCreateInfo samplerCreateInfo = {
                VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,  // VkStructureType        sType
                nullptr,                                // const void*            pNext
                0,                                      // VkSamplerCreateFlags   flags
                VK_FILTER_LINEAR,                       // VkFilter               magFilter
                VK_FILTER_LINEAR,                       // VkFilter               minFilter
                VK_SAMPLER_MIPMAP_MODE_NEAREST,         // VkSamplerMipmapMode    mipmapMode
                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,  // VkSamplerAddressMode   addressModeU
                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,  // VkSamplerAddressMode   addressModeV
                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,  // VkSamplerAddressMode   addressModeW
                0.0f,                                   // float                  mipLodBias
                VK_FALSE,                               // VkBool32               anisotropyEnable
                1.0f,                                   // float                  maxAnisotropy
                VK_FALSE,                               // VkBool32               compareEnable
                VK_COMPARE_OP_ALWAYS,                   // VkCompareOp            compareOp
                0.0f,                                   // float                  minLod
                0.0f,                                   // float                  maxLod
                VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,// VkBorderColor          borderColor
                VK_FALSE                                // VkBool32               unnormalizedCoordinates
        };

        return createSampler(kraut.Vulkan.Device.Handle, &samplerCreateInfo, nullptr, sampler) == VK_SUCCESS;
    }

    bool KrautVK::kvkCreateBuffer(Com::BufferParameters &buffer, VkBufferCreateFlags usage, VkMemoryPropertyFlagBits memoryProperty) {

        VkBufferCreateInfo vkBufferCreateInfo = {
                VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,             // VkStructureType        sType
                nullptr,                                          // const void            *pNext
                0,                                                // VkBufferCreateFlags    flags
                buffer.Size,                                      // VkDeviceSize           size
                usage,                                            // VkBufferUsageFlags     usage
                VK_SHARING_MODE_EXCLUSIVE,                        // VkSharingMode          sharingMode
                0,                                                // uint32_t               queueFamilyIndexCount
                nullptr                                           // const uint32_t        *pQueueFamilyIndices
        };

        if(createBuffer(kraut.Vulkan.Device.Handle, &vkBufferCreateInfo, nullptr, &buffer.Handle) != VK_SUCCESS ) {
            return false;
        }
        if(!kvkAllocateBufferMemory(buffer.Handle, memoryProperty, &buffer.Memory)) {
            return false;
        }

        return !(bindBufferMemory(kraut.Vulkan.Device.Handle, buffer.Handle, buffer.Memory, 0) != VK_SUCCESS);

    }

    int KrautVK::kvkCreateVertexBuffer() {
        const std::vector<float> &vertexData = GlobalVertexData;

        kraut.DemoResources.VertexBuffer.Size = static_cast<uint32_t>(vertexData.size() * sizeof(vertexData[0]));
        if(!kvkCreateBuffer(kraut.DemoResources.VertexBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
            return VULKAN_VERTEX_CREATION_FAILED;
        }

        return SUCCESS;

    }

    int KrautVK::kvkCreateStagingBuffer() {
        kraut.StagingBuffer.Size = KVK_STAGING_BUFFER_SIZE;
        if(!kvkCreateBuffer(kraut.StagingBuffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)){
            return VULKAN_VERTEX_CREATION_FAILED;
        };

        return SUCCESS;

    }

    int KrautVK::kvkCopyBufferToGPU() {
        const std::vector<float> &vertexData = GlobalVertexData;

        void *stagingBufferMemoryPointer;
        if(mapMemory(kraut.Vulkan.Device.Handle, kraut.StagingBuffer.Memory, 0, kraut.DemoResources.VertexBuffer.Size, 0, &stagingBufferMemoryPointer) != VK_SUCCESS) {
            return VULKAN_VERTEX_CREATION_FAILED;
        }

        memcpy(stagingBufferMemoryPointer, &vertexData[0], kraut.DemoResources.VertexBuffer.Size);

        VkMappedMemoryRange flushRange = {
                VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,            // VkStructureType                        sType
                nullptr,                                          // const void                            *pNext
                kraut.StagingBuffer.Memory,                       // VkDeviceMemory                         memory
                0,                                                // VkDeviceSize                           offset
                kraut.DemoResources.VertexBuffer.Size             // VkDeviceSize                           size
        };

        flushMappedMemoryRanges(kraut.Vulkan.Device.Handle, 1, &flushRange);

        unmapMemory(kraut.Vulkan.Device.Handle, kraut.StagingBuffer.Memory);

        VkCommandBufferBeginInfo commandBufferBeginInfo = {
                VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,      // VkStructureType                        sType
                nullptr,                                          // const void                            *pNext
                VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,      // VkCommandBufferUsageFlags              flags
                nullptr                                           // const VkCommandBufferInheritanceInfo  *pInheritanceInfo
        };

        VkCommandBuffer commandBuffer = kraut.Vulkan.RenderingResources[0].CommandBuffer;

        beginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

        VkBufferCopy bufferCopyInfo = {
                0,                                                // VkDeviceSize                           srcOffset
                0,                                                // VkDeviceSize                           dstOffset
                kraut.DemoResources.VertexBuffer.Size             // VkDeviceSize                           size
        };

        cmdCopyBuffer(commandBuffer, kraut.StagingBuffer.Handle, kraut.DemoResources.VertexBuffer.Handle, 1, &bufferCopyInfo);

        VkBufferMemoryBarrier bufferMemoryBarrier = {
                VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,          // VkStructureType                        sType;
                nullptr,                                          // const void                            *pNext
                VK_ACCESS_MEMORY_WRITE_BIT,                       // VkAccessFlags                          srcAccessMask
                VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,              // VkAccessFlags                          dstAccessMask
                VK_QUEUE_FAMILY_IGNORED,                          // uint32_t                               srcQueueFamilyIndex
                VK_QUEUE_FAMILY_IGNORED,                          // uint32_t                               dstQueueFamilyIndex
                kraut.DemoResources.VertexBuffer.Handle,                       // VkBuffer                               buffer
                0,                                                // VkDeviceSize                           offset
                VK_WHOLE_SIZE                                     // VkDeviceSize                           size
        };

        cmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, nullptr, 1, &bufferMemoryBarrier, 0, nullptr);

        endCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo = {
                VK_STRUCTURE_TYPE_SUBMIT_INFO,                    // VkStructureType                        sType
                nullptr,                                          // const void                            *pNext
                0,                                                // uint32_t                               waitSemaphoreCount
                nullptr,                                          // const VkSemaphore                     *pWaitSemaphores
                nullptr,                                          // const VkPipelineStageFlags            *pWaitDstStageMask;
                1,                                                // uint32_t                               commandBufferCount
                &commandBuffer,                                   // const VkCommandBuffer                 *pCommandBuffers
                0,                                                // uint32_t                               signalSemaphoreCount
                nullptr                                           // const VkSemaphore                     *pSignalSemaphores
        };

        if(queueSubmit(kraut.GraphicsQueue.Handle, 1, &submitInfo, VK_NULL_HANDLE ) != VK_SUCCESS) {
            return VULKAN_VERTEX_CREATION_FAILED;
        }

        deviceWaitIdle(kraut.Vulkan.Device.Handle);

        return SUCCESS;

    }

    int KrautVK::kvkInit(const int &width, const int &height, const char *title, const int &fullScreen) {
        std::cout << "\nKrautVK Alpha v" << krautvk_VERSION_MAJOR << "." << krautvk_VERSION_MINOR << "\n";

        printf("Initializing GLFW...\n");
        int status = kvkInitGLFW(width, height, title, fullScreen);
        if (status != SUCCESS)
            return status;

        printf("Initializing Vulkan...\n");
        status = kvkCreateInstance(title);
        if (status != SUCCESS)
            return status;

        printf("Setting Up Hardware...\n");
        status = kvkCreateDevice();
        if (status != SUCCESS)
            return status;

        printf("Loading Rendering Resources...\n");
        if(!kvkCreateSwapChain())
            return INT32_MIN;

        status = kvkCreateCommandPool();
        if (status != SUCCESS)
            return status;

        for(size_t i = 0; i < kraut.Vulkan.RenderingResources.size(); i++) {

            status = kvkAllocateCommandBuffer(kraut.Vulkan.CommandPool, 1, &kraut.Vulkan.RenderingResources[i].CommandBuffer);
            if (status != SUCCESS)
                return status;

            status = kvkCreateSemaphore(&kraut.Vulkan.RenderingResources[i].ImageAvailableSemaphore);
            if (status != SUCCESS)
                return status;

            status = kvkCreateSemaphore(&kraut.Vulkan.RenderingResources[i].FinishedRenderingSemaphore);
            if (status != SUCCESS)
                return status;

            status = kvkCreateFence(&kraut.Vulkan.RenderingResources[i].Fence);
            if (status != SUCCESS)
                return status;
        }

        status = kvkCreateStagingBuffer();
        if (status != SUCCESS)
            return status;

        printf("Staging Test Environment...\n");

        status = kvkCreateTexture("/res/demo.png", kraut.DemoResources.Image);
        if (status != SUCCESS)
            return status;

        printf("Setting Up Engine...\n");
        status = kvkCreateDescriptorSet();
        if (status != SUCCESS)
            return status;

        status = kvkCreateRenderPass();
        if (status != SUCCESS)
            return status;

        status = kvkCreatePipelines();
        if (status != SUCCESS)
            return status;

        status = kvkCreateVertexBuffer();
        if (status != SUCCESS)
            return status;

        status = kvkCopyBufferToGPU();
        if (status != SUCCESS)
            return status;

        printf("KrautVK Alpha Initialized!\n");

        return SUCCESS;
    }

    void KrautVK::kvkTerminate() {
        printf("KrautVK terminating\n");

        if (kraut.Vulkan.Device.Handle != VK_NULL_HANDLE) {
            deviceWaitIdle(kraut.Vulkan.Device.Handle);

            //Destroy Rendering Resource Data
            for(unsigned int i = 0; i < kraut.Vulkan.RenderingResources.size(); i++)
                kraut.Vulkan.RenderingResources[i].DestroyResources();


            //Destroy Command Pool
            if(kraut.Vulkan.CommandPool != VK_NULL_HANDLE) {
                destroyCommandPool(kraut.Vulkan.Device.Handle, kraut.Vulkan.CommandPool, nullptr);
                kraut.Vulkan.CommandPool = VK_NULL_HANDLE;
            }


            //Destroy Vertex Buffer
            if(kraut.DemoResources.VertexBuffer.Handle != VK_NULL_HANDLE) {
                destroyBuffer(kraut.Vulkan.Device.Handle, kraut.DemoResources.VertexBuffer.Handle, nullptr);
                kraut.DemoResources.VertexBuffer.Handle = VK_NULL_HANDLE;
            }

            if(kraut.DemoResources.VertexBuffer.Memory != VK_NULL_HANDLE) {
                freeMemory(kraut.Vulkan.Device.Handle, kraut.DemoResources.VertexBuffer.Memory, nullptr);
                kraut.DemoResources.VertexBuffer.Memory = VK_NULL_HANDLE;
            }

            //Destroy Staging Buffer
            if(kraut.StagingBuffer.Handle != VK_NULL_HANDLE) {
                destroyBuffer(kraut.Vulkan.Device.Handle, kraut.StagingBuffer.Handle, nullptr);
                kraut.StagingBuffer.Handle = VK_NULL_HANDLE;
            }

            if(kraut.StagingBuffer.Memory != VK_NULL_HANDLE) {
                freeMemory(kraut.Vulkan.Device.Handle, kraut.StagingBuffer.Memory, nullptr);
                kraut.StagingBuffer.Memory = VK_NULL_HANDLE;
            }


            //Destroy Pipeline
            if(kraut.Vulkan.GraphicsPipeline != VK_NULL_HANDLE) {
                destroyPipeline(kraut.Vulkan.Device.Handle, kraut.Vulkan.GraphicsPipeline, nullptr);
                kraut.Vulkan.GraphicsPipeline = VK_NULL_HANDLE;
            }

            if(kraut.Vulkan.PipelineLayout != VK_NULL_HANDLE ) {
                destroyPipelineLayout(kraut.Vulkan.Device.Handle, kraut.Vulkan.PipelineLayout, nullptr);
                kraut.Vulkan.PipelineLayout = VK_NULL_HANDLE;
            }

            //Destroy Descriptor Sets
            if(kraut.Vulkan.Descriptor.Pool != VK_NULL_HANDLE ) {
                destroyDescriptorPool(kraut.Vulkan.Device.Handle, kraut.Vulkan.Descriptor.Pool, nullptr);
                kraut.Vulkan.Descriptor.Pool = VK_NULL_HANDLE;
            }

            if(kraut.Vulkan.Descriptor.Layout != VK_NULL_HANDLE ) {
                destroyDescriptorSetLayout(kraut.Vulkan.Device.Handle, kraut.Vulkan.Descriptor.Layout, nullptr);
                kraut.Vulkan.Descriptor.Layout = VK_NULL_HANDLE;
            }

            //Destroy Demo Image
            if(kraut.DemoResources.Image.Sampler != VK_NULL_HANDLE ) {
                destroySampler(kraut.Vulkan.Device.Handle, kraut.DemoResources.Image.Sampler, nullptr);
                kraut.DemoResources.Image.Sampler = VK_NULL_HANDLE;
            }

            if(kraut.DemoResources.Image.View != VK_NULL_HANDLE ) {
                destroyImageView(kraut.Vulkan.Device.Handle, kraut.DemoResources.Image.View, nullptr);
                kraut.DemoResources.Image.View = VK_NULL_HANDLE;
            }

            if(kraut.DemoResources.Image.Handle != VK_NULL_HANDLE ) {
                destroyImage(kraut.Vulkan.Device.Handle, kraut.DemoResources.Image.Handle, nullptr);
                kraut.DemoResources.Image.Handle = VK_NULL_HANDLE;
            }

            if(kraut.DemoResources.Image.Memory != VK_NULL_HANDLE ) {
                freeMemory(kraut.Vulkan.Device.Handle, kraut.DemoResources.Image.Memory, nullptr);
                kraut.DemoResources.Image.Memory = VK_NULL_HANDLE;
            }


            //Destroy Renderpass
            if(kraut.Vulkan.RenderPass != VK_NULL_HANDLE) {
                destroyRenderPass(kraut.Vulkan.Device.Handle, kraut.Vulkan.RenderPass, nullptr);
                kraut.Vulkan.RenderPass = VK_NULL_HANDLE;
            }

            //Destroy Swapchain
            if (kraut.Vulkan.SwapChain.Handle != VK_NULL_HANDLE) {
                destroySwapchainKHR(kraut.Vulkan.Device.Handle, kraut.Vulkan.SwapChain.Handle, nullptr);
            }

            //Destroy Device
            destroyDevice(kraut.Vulkan.Device.Handle, nullptr);
        }

        //Destroy Application Surface
        if (kraut.Vulkan.ApplicationSurface != VK_NULL_HANDLE) {
            destroySurfaceKHR(kraut.Vulkan.Instance, kraut.Vulkan.ApplicationSurface, nullptr);
        }

        //Destroy Vulkan Instance
        if (kraut.Vulkan.Instance != VK_NULL_HANDLE) {
            destroyInstance(kraut.Vulkan.Instance, nullptr);
        }

        //Terminate GLFW
        glfwTerminate();

    }

    bool KrautVK::kvkLayoutDescriptorSet() {
        VkDescriptorSetLayoutBinding layoutBinding = {
                0,                                          // uint32_t             binding
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,  // VkDescriptorType     descriptorType
                1,                                          // uint32_t             descriptorCount
                VK_SHADER_STAGE_FRAGMENT_BIT,               // VkShaderStageFlags   stageFlags
                nullptr                                     // const VkSampler     *pImmutableSamplers
        };

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
                VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,  // VkStructureType                      sType
                nullptr,                                              // const void                          *pNext
                0,                                                    // VkDescriptorSetLayoutCreateFlags     flags
                1,                                                    // uint32_t                             bindingCount
                &layoutBinding                                        // const VkDescriptorSetLayoutBinding  *pBindings
        };

        return !(createDescriptorSetLayout(kraut.Vulkan.Device.Handle, &descriptorSetLayoutCreateInfo, nullptr, &kraut.Vulkan.Descriptor.Layout ) != VK_SUCCESS);

    }

    bool KrautVK::kvkCreateDescriptorPool() {
        VkDescriptorPoolSize poolSize = {
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,      // VkDescriptorType               type
                1                                               // uint32_t                       descriptorCount
        };

        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
                VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,  // VkStructureType                sType
                nullptr,                                        // const void                    *pNext
                0,                                              // VkDescriptorPoolCreateFlags    flags
                1,                                              // uint32_t                       maxSets
                1,                                              // uint32_t                       poolSizeCount
                &poolSize                                       // const VkDescriptorPoolSize    *pPoolSizes
        };

        return createDescriptorPool(kraut.Vulkan.Device.Handle, &descriptorPoolCreateInfo, nullptr, &kraut.Vulkan.Descriptor.Pool) == VK_SUCCESS;

    }

    bool KrautVK::kvkAllocateDescriptorSet() {
        VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {
                VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, // VkStructureType                sType
                nullptr,                                        // const void                    *pNext
                kraut.Vulkan.Descriptor.Pool,                   // VkDescriptorPool               descriptorPool
                1,                                              // uint32_t                       descriptorSetCount
                &kraut.Vulkan.Descriptor.Layout                 // const VkDescriptorSetLayout   *pSetLayouts
        };

        return !(allocateDescriptorSets(kraut.Vulkan.Device.Handle, &descriptorSetAllocateInfo, &kraut.Vulkan.Descriptor.Handle ) != VK_SUCCESS);


    }

    void KrautVK::kvkUpdateDescriptorSet() {
        VkDescriptorImageInfo imageInfo = {
                kraut.DemoResources.Image.Sampler,                       // VkSampler                      sampler
                kraut.DemoResources.Image.View,                          // VkImageView                    imageView
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL                 // VkImageLayout                  imageLayout
        };

        VkWriteDescriptorSet descriptorWrites = {
                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,     // VkStructureType                sType
                nullptr,                                    // const void                    *pNext
                kraut.Vulkan.Descriptor.Handle,             // VkDescriptorSet                dstSet
                0,                                          // uint32_t                       dstBinding
                0,                                          // uint32_t                       dstArrayElement
                1,                                          // uint32_t                       descriptorCount
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,  // VkDescriptorType               descriptorType
                &imageInfo,                                 // const VkDescriptorImageInfo   *pImageInfo
                nullptr,                                    // const VkDescriptorBufferInfo  *pBufferInfo
                nullptr                                     // const VkBufferView            *pTexelBufferView
        };

        updateDescriptorSets(kraut.Vulkan.Device.Handle, 1, &descriptorWrites, 0, nullptr);


    }

    int KrautVK::kvkCreateDescriptorSet() {

        if(!kvkLayoutDescriptorSet())
            return VULKAN_DESCRIPTOR_SET_CREATION_FAILED;

        if(!kvkCreateDescriptorPool())
            return VULKAN_DESCRIPTOR_SET_CREATION_FAILED;

        if(!kvkAllocateDescriptorSet())
            return VULKAN_DESCRIPTOR_SET_CREATION_FAILED;

        kvkUpdateDescriptorSet();
        return SUCCESS;
    }
}
