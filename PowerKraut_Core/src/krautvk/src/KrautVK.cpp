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

namespace KrautVK {

    KrautCommon KrautVK::kraut;

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
            kvkOnWindowSizeChanged(width, height);
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

        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;

        getPhysicalDeviceProperties(physicalDevice, &deviceProperties);
        getPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

        uint32_t majorVersion = VK_VERSION_MAJOR(deviceProperties.apiVersion);
        uint32_t minorVersion = VK_VERSION_MINOR(
                deviceProperties.apiVersion);  //Reserved for when it's time to expect finer version requirements
        uint32_t patchVersion = VK_VERSION_PATCH(deviceProperties.apiVersion);

        if ((majorVersion < 1) || (deviceProperties.limits.maxImageDimension2D < 4096)) {
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

                std::cout << "Selected Device: " << deviceProperties.deviceName << std::endl;
                return true;
            }
        }

        return false;
    }

    int KrautVK::kvkCreateInstance(const char *title) {

        //CREATE VULKAN INSTANCE
        if (!glfwVulkanSupported())
            return VULKAN_NOT_SUPPORTED;

        printf("Loading global level procedures...\n");
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

        printf("Loading instance level procedures...\n");
        createDevice = (PFN_vkCreateDevice) glfwGetInstanceProcAddress(kraut.Vulkan.Instance, "vkCreateDevice");
        enumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices) glfwGetInstanceProcAddress(kraut.Vulkan.Instance, "vkEnumeratePhysicalDevices");
        getPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties) glfwGetInstanceProcAddress(kraut.Vulkan.Instance, "vkGetPhysicalDeviceProperties");
        getPhysicalDeviceFeatures = (PFN_vkGetPhysicalDeviceFeatures) glfwGetInstanceProcAddress(kraut.Vulkan.Instance, "vkGetPhysicalDeviceFeatures");
        getPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties) glfwGetInstanceProcAddress(kraut.Vulkan.Instance, "vkGetPhysicalDeviceQueueFamilyProperties");
        destroyInstance = (PFN_vkDestroyInstance) glfwGetInstanceProcAddress(kraut.Vulkan.Instance, "vkDestroyInstance");
        destroySurfaceKHR = (PFN_vkDestroySurfaceKHR) glfwGetInstanceProcAddress(kraut.Vulkan.Instance, "vkDestroySurfaceKHR");
        enumerateDeviceExtensionProperties = (PFN_vkEnumerateDeviceExtensionProperties) glfwGetInstanceProcAddress(kraut.Vulkan.Instance, "vkEnumerateDeviceExtensionProperties");
        getPhysicalDeviceSurfaceCapabilitiesKHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR) glfwGetInstanceProcAddress(kraut.Vulkan.Instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
        getPhysicalDeviceSurfaceFormatsKHR = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR) glfwGetInstanceProcAddress(kraut.Vulkan.Instance, "vkGetPhysicalDeviceSurfaceFormatsKHR");
        getPhysicalDeviceSurfacePresentModesKHR = (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR) glfwGetInstanceProcAddress(kraut.Vulkan.Instance, "vkGetPhysicalDeviceSurfacePresentModesKHR");


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
                kraut.Vulkan.PhysicalDevice = devices[i];
                break;
            }
        }

        if (kraut.Vulkan.PhysicalDevice == nullptr)
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

        if (createDevice(kraut.Vulkan.PhysicalDevice, &deviceCreateInfo, nullptr, &kraut.Vulkan.Device) != SUCCESS)
            return VULKAN_DEVICE_CREATION_FAILED;

        printf("Loading device level procedures..\n");
        getDeviceProcAddr = (PFN_vkGetDeviceProcAddr) glfwGetInstanceProcAddress(kraut.Vulkan.Instance, "vkGetDeviceProcAddr");

        getDeviceQueue = (PFN_vkGetDeviceQueue)                                                         getDeviceProcAddr(kraut.Vulkan.Device, "vkGetDeviceQueue");
        deviceWaitIdle = (PFN_vkDeviceWaitIdle)                                                         getDeviceProcAddr(kraut.Vulkan.Device, "vkDeviceWaitIdle");
        destroyDevice = (PFN_vkDestroyDevice)                                                           getDeviceProcAddr(kraut.Vulkan.Device, "vkDestroyDevice");
        createSemaphore = (PFN_vkCreateSemaphore)                                                       getDeviceProcAddr(kraut.Vulkan.Device, "vkCreateSemaphore");
        createSwapchainKHR = (PFN_vkCreateSwapchainKHR)                                                 getDeviceProcAddr(kraut.Vulkan.Device, "vkCreateSwapchainKHR");
        destroySwapchainKHR = (PFN_vkDestroySwapchainKHR)                                               getDeviceProcAddr(kraut.Vulkan.Device, "vkDestroySwapchainKHR");
        getSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR)                                           getDeviceProcAddr(kraut.Vulkan.Device, "vkGetSwapchainImagesKHR");
        acquireNextImageKHR = (PFN_vkAcquireNextImageKHR)                                               getDeviceProcAddr(kraut.Vulkan.Device, "vkAcquireNextImageKHR");
        queuePresentKHR = (PFN_vkQueuePresentKHR)                                                       getDeviceProcAddr(kraut.Vulkan.Device, "vkQueuePresentKHR");
        queueSubmit = (PFN_vkQueueSubmit)                                                               getDeviceProcAddr(kraut.Vulkan.Device, "vkQueueSubmit");
        createCommandPool = (PFN_vkCreateCommandPool)                                                   getDeviceProcAddr(kraut.Vulkan.Device, "vkCreateCommandPool");
        allocateCommandBuffers = (PFN_vkAllocateCommandBuffers)                                         getDeviceProcAddr(kraut.Vulkan.Device, "vkAllocateCommandBuffers");
        freeCommandBuffers = (PFN_vkFreeCommandBuffers)                                                 getDeviceProcAddr(kraut.Vulkan.Device, "vkFreeCommandBuffers");
        destroyCommandPool = (PFN_vkDestroyCommandPool)                                                 getDeviceProcAddr(kraut.Vulkan.Device, "vkDestroyCommandPool");
        destroySemaphore = (PFN_vkDestroySemaphore)                                                     getDeviceProcAddr(kraut.Vulkan.Device, "vkDestroySemaphore");
        beginCommandBuffer = (PFN_vkBeginCommandBuffer)                                                 getDeviceProcAddr(kraut.Vulkan.Device, "vkBeginCommandBuffer");
        cmdPipelineBarrier = (PFN_vkCmdPipelineBarrier)                                                 getDeviceProcAddr(kraut.Vulkan.Device, "vkCmdPipelineBarrier");
        endCommandBuffer = (PFN_vkEndCommandBuffer)                                                     getDeviceProcAddr(kraut.Vulkan.Device, "vkEndCommandBuffer");
        destroyImageView = (PFN_vkDestroyImageView)                                                     getDeviceProcAddr(kraut.Vulkan.Device, "vkDestroyImageView");
        createImageView = (PFN_vkCreateImageView)                                                       getDeviceProcAddr(kraut.Vulkan.Device, "vkCreateImageView");
        createRenderPass = (PFN_vkCreateRenderPass)                                                     getDeviceProcAddr(kraut.Vulkan.Device, "vkCreateRenderPass");
        createFramebuffer = (PFN_vkCreateFramebuffer)                                                   getDeviceProcAddr(kraut.Vulkan.Device, "vkCreateFramebuffer");
        createShaderModule = (PFN_vkCreateShaderModule)                                                 getDeviceProcAddr(kraut.Vulkan.Device, "vkCreateShaderModule");
        destroyShaderModule = (PFN_vkDestroyShaderModule)                                               getDeviceProcAddr(kraut.Vulkan.Device, "vkDestroyShaderModule");
        cmdBeginRenderPass = (PFN_vkCmdBeginRenderPass)                                                 getDeviceProcAddr(kraut.Vulkan.Device, "vkCmdBeginRenderPass");
        cmdBindPipeline = (PFN_vkCmdBindPipeline)                                                       getDeviceProcAddr(kraut.Vulkan.Device, "vkCmdBindPipeline");
        cmdDraw = (PFN_vkCmdDraw)                                                                       getDeviceProcAddr(kraut.Vulkan.Device, "vkCmdDraw");
        cmdEndRenderPass = (PFN_vkCmdEndRenderPass)                                                     getDeviceProcAddr(kraut.Vulkan.Device, "vkCmdEndRenderPass");
        destroyPipeline = (PFN_vkDestroyPipeline)                                                       getDeviceProcAddr(kraut.Vulkan.Device, "vkDestroyPipeline");
        destroyRenderPass = (PFN_vkDestroyRenderPass)                                                   getDeviceProcAddr(kraut.Vulkan.Device, "vkDestroyRenderPass");
        destroyFramebuffer = (PFN_vkDestroyFramebuffer)                                                 getDeviceProcAddr(kraut.Vulkan.Device, "vkDestroyFramebuffer");
        createGraphicsPipelines = (PFN_vkCreateGraphicsPipelines)                                       getDeviceProcAddr(kraut.Vulkan.Device, "vkCreateGraphicsPipelines");
        createPipelineLayout = (PFN_vkCreatePipelineLayout)                                             getDeviceProcAddr(kraut.Vulkan.Device, "vkCreatePipelineLayout");
        destroyPipelineLayout = (PFN_vkDestroyPipelineLayout)                                           getDeviceProcAddr(kraut.Vulkan.Device, "vkDestroyPipelineLayout");


        //INITIALIZE COMMAND BUFFER
        printf("Initializing command buffers..\n");
        kraut.GraphicsQueue.FamilyIndex = selectedGraphicsQueueFamilyIndex;
        kraut.PresentQueue.FamilyIndex = selectedPresentationQueueFamilyIndex;

        getDeviceQueue(kraut.Vulkan.Device, kraut.GraphicsQueue.FamilyIndex, 0, &kraut.GraphicsQueue.Handle);
        getDeviceQueue(kraut.Vulkan.Device, kraut.PresentQueue.FamilyIndex, 0, &kraut.PresentQueue.Handle);

        return SUCCESS;
    }

    int KrautVK::kvkCreateSemaphores() {
        printf("Creating Semaphores...\n");

        VkSemaphoreCreateInfo semaphore_create_info = {
                VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,      // VkStructureType          sType
                nullptr,                                      // const void*              pNext
                0                                             // VkSemaphoreCreateFlags   flags
        };

        if ((createSemaphore(kraut.Vulkan.Device, &semaphore_create_info, nullptr, &kraut.Semaphores.ImageAvailable) !=
             VK_SUCCESS) ||
            (createSemaphore(kraut.Vulkan.Device, &semaphore_create_info, nullptr,
                             &kraut.Semaphores.RenderingFinished) != VK_SUCCESS)) {
            return VULKAN_SEMAPHORE_CREATION_FAILED;
        }

        return SUCCESS;
    }

    bool KrautVK::kvkCreateSwapChain() {

        if (kraut.Vulkan.Device != VK_NULL_HANDLE) {
            deviceWaitIdle(kraut.Vulkan.Device);
        }

        for(size_t i = 0; i < kraut.SwapChain.Images.size(); ++i) {
            if(kraut.SwapChain.Images[i].View != VK_NULL_HANDLE) {
                destroyImageView(kraut.Vulkan.Device, kraut.SwapChain.Images[i].View, nullptr);
                kraut.SwapChain.Images[i].View = VK_NULL_HANDLE;
            }
        }
        kraut.SwapChain.Images.clear();

        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        if (getPhysicalDeviceSurfaceCapabilitiesKHR(kraut.Vulkan.PhysicalDevice, kraut.Vulkan.ApplicationSurface, &surfaceCapabilities) != VK_SUCCESS) {
            return false;
        }

        uint32_t formatsCount;
        if ((getPhysicalDeviceSurfaceFormatsKHR(kraut.Vulkan.PhysicalDevice, kraut.Vulkan.ApplicationSurface, &formatsCount, nullptr) != VK_SUCCESS) ||
            (formatsCount == 0)) {
            return false;
        }

        std::vector<VkSurfaceFormatKHR> surfaceFormats(formatsCount);
        if (getPhysicalDeviceSurfaceFormatsKHR(kraut.Vulkan.PhysicalDevice, kraut.Vulkan.ApplicationSurface, &formatsCount, surfaceFormats.data()) != VK_SUCCESS) {
            return false;
        }

        uint32_t presentModesCount;
        if ((getPhysicalDeviceSurfacePresentModesKHR(kraut.Vulkan.PhysicalDevice, kraut.Vulkan.ApplicationSurface, &presentModesCount, nullptr) != VK_SUCCESS) ||
            (presentModesCount == 0)) {
            return false;
        }

        std::vector<VkPresentModeKHR> presentModes(presentModesCount);
        if (getPhysicalDeviceSurfacePresentModesKHR(kraut.Vulkan.PhysicalDevice, kraut.Vulkan.ApplicationSurface,
                                                    &presentModesCount, presentModes.data()) != VK_SUCCESS) {
            return false;
        }

        uint32_t desiredNumberOfImages = kvkGetSwapChainNumImages(surfaceCapabilities);
        VkSurfaceFormatKHR desiredFormat = kvkGetSwapChainFormat(surfaceFormats);
        VkExtent2D desiredExtent = kvkGetSwapChainExtent(surfaceCapabilities);
        VkImageUsageFlags desiredUsage = kvkGetSwapChainUsageFlags(surfaceCapabilities);
        VkSurfaceTransformFlagBitsKHR desiredTransform = kvkGetSwapChainTransform(surfaceCapabilities);
        VkPresentModeKHR desiredPresentMode = kvkGetSwapChainPresentMode(presentModes);
        VkSwapchainKHR oldSwapChain = kraut.SwapChain.Handle;

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

        VkSwapchainCreateInfoKHR swap_chain_create_info = {
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

        if (createSwapchainKHR(kraut.Vulkan.Device, &swap_chain_create_info, nullptr, &kraut.SwapChain.Handle) !=
            VK_SUCCESS) {
            return false;
        }

        kraut.SwapChain.Extent = desiredExtent;
        kraut.SwapChain.Format = desiredFormat.format;

        if (oldSwapChain != VK_NULL_HANDLE) {
            destroySwapchainKHR(kraut.Vulkan.Device, oldSwapChain, nullptr);
        }

        uint32_t imageCount = 0;
        if((getSwapchainImagesKHR( kraut.Vulkan.Device, kraut.SwapChain.Handle, &imageCount, nullptr ) != VK_SUCCESS) ||
            (imageCount == 0) ) {
            std::cout << "Could not get swap chain images!" << std::endl;
            return false;
        }
        kraut.SwapChain.Images.resize( imageCount );

        std::vector<VkImage> images( imageCount );
        if(getSwapchainImagesKHR(kraut.Vulkan.Device, kraut.SwapChain.Handle, &imageCount, images.data() ) != VK_SUCCESS ) {
            std::cout << "Could not get swap chain images!" << std::endl;
            return false;
        }

        for( size_t i = 0; i < kraut.SwapChain.Images.size(); ++i ) {
            kraut.SwapChain.Images[i].Handle = images[i];
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

    bool KrautVK::kvkCreateCommandBuffers(const uint32_t &width, const uint32_t &height) {
        VkCommandPoolCreateInfo cmdPoolCreateInfo = {
                VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,     // VkStructureType              sType
                nullptr,                                        // const void*                  pNext
                0,                                              // VkCommandPoolCreateFlags     flags
                kraut.PresentQueue.FamilyIndex                  // uint32_t                     queueFamilyIndex
        };

        if (createCommandPool(kraut.Vulkan.Device, &cmdPoolCreateInfo, nullptr, &kraut.GraphicsBuffer.CommandPool) !=
            VK_SUCCESS) {
            return false;
        }

        uint32_t imageCount = 0;
        if ((getSwapchainImagesKHR(kraut.Vulkan.Device, kraut.SwapChain.Handle, &imageCount, nullptr) != VK_SUCCESS) ||
            (imageCount == 0)) {
            return false;
        }

        kraut.GraphicsBuffer.CommandBuffers.resize(imageCount);

        VkCommandBufferAllocateInfo cmdBufferAllocateInfo = {
                VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, // VkStructureType              sType
                nullptr,                                        // const void*                  pNext
                kraut.GraphicsBuffer.CommandPool,               // VkCommandPool                commandPool
                VK_COMMAND_BUFFER_LEVEL_PRIMARY,                // VkCommandBufferLevel         level
                imageCount                                      // uint32_t                     bufferCount
        };

        if (allocateCommandBuffers(kraut.Vulkan.Device, &cmdBufferAllocateInfo, &kraut.GraphicsBuffer.CommandBuffers[0]) != VK_SUCCESS) {
            return false;
        }

        return kvkRecordCommandBuffers(width, height);

    }

    bool KrautVK::kvkRecordCommandBuffers(const uint32_t &width, const uint32_t &height) {
        VkCommandBufferBeginInfo graphicsCommandBufferBeginInfo = {
                VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,    // VkStructureType                        sType
                nullptr,                                        // const void                            *pNext
                VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,   // VkCommandBufferUsageFlags              flags
                nullptr                                         // const VkCommandBufferInheritanceInfo  *pInheritanceInfo
        };

        VkImageSubresourceRange imageSubresourceRange = {
                VK_IMAGE_ASPECT_COLOR_BIT,                      // VkImageAspectFlags             aspectMask
                0,                                              // uint32_t                       baseMipLevel
                1,                                              // uint32_t                       levelCount
                0,                                              // uint32_t                       baseArrayLayer
                1                                               // uint32_t                       layerCount
        };

        VkClearValue clear_value = {
                KVK_CLEAR_COLOR,                                // VkClearColorValue              color
        };


        for(size_t i = 0; i < kraut.GraphicsBuffer.CommandBuffers.size(); ++i) {
            beginCommandBuffer(kraut.GraphicsBuffer.CommandBuffers[i], &graphicsCommandBufferBeginInfo);

            if(kraut.PresentQueue.Handle != kraut.GraphicsQueue.Handle) {
                VkImageMemoryBarrier barrierFromPresentToDraw = {
                        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,     // VkStructureType                sType
                        nullptr,                                    // const void                    *pNext
                        VK_ACCESS_MEMORY_READ_BIT,                  // VkAccessFlags                  srcAccessMask
                        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,       // VkAccessFlags                  dstAccessMask
                        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,            // VkImageLayout                  oldLayout
                        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,            // VkImageLayout                  newLayout
                        kraut.PresentQueue.FamilyIndex,             // uint32_t                       srcQueueFamilyIndex
                        kraut.GraphicsQueue.FamilyIndex,            // uint32_t                       dstQueueFamilyIndex
                        kraut.SwapChain.Images[i].Handle,           // VkImage                        image
                        imageSubresourceRange                       // VkImageSubresourceRange        subresourceRange
                };
                cmdPipelineBarrier(kraut.GraphicsBuffer.CommandBuffers[i], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrierFromPresentToDraw);
            }

            VkRenderPassBeginInfo renderPassBeginInfo = {
                    VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,     // VkStructureType                sType
                    nullptr,                                      // const void                    *pNext
                    kraut.Vulkan.RenderPass,                      // VkRenderPass                   renderPass
                    kraut.Vulkan.FrameBuffers[i],                 // VkFramebuffer                  framebuffer
                    {                                             // VkRect2D                       renderArea
                            {                                     // VkOffset2D                     offset
                                    0,                            // int32_t                        x
                                    0                             // int32_t                        y
                            },
                            {                                     // VkExtent2D                     extent
                                    width,                        // uint32_t                        width
                                    height,                       // uint32_t                        height
                            }
                    },
                    1,                                            // uint32_t                       clearValueCount
                    &clear_value                                  // const VkClearValue            *pClearValues
            };

            cmdBeginRenderPass(kraut.GraphicsBuffer.CommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

            cmdBindPipeline(kraut.GraphicsBuffer.CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, kraut.Vulkan.GraphicsPipeline);

            cmdDraw(kraut.GraphicsBuffer.CommandBuffers[i], KVK_VERTEX_COUNT, KVK_INSTANCE_COUNT, 0, 0);

            cmdEndRenderPass(kraut.GraphicsBuffer.CommandBuffers[i]);

            if(kraut.GraphicsQueue.Handle != kraut.PresentQueue.Handle) {
                VkImageMemoryBarrier barrierFromDrawToPresent = {
                        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,       // VkStructureType              sType
                        nullptr,                                      // const void                  *pNext
                        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,         // VkAccessFlags                srcAccessMask
                        VK_ACCESS_MEMORY_READ_BIT,                    // VkAccessFlags                dstAccessMask
                        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,              // VkImageLayout                oldLayout
                        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,              // VkImageLayout                newLayout
                        kraut.GraphicsQueue.FamilyIndex,              // uint32_t                     srcQueueFamilyIndex
                        kraut.PresentQueue.FamilyIndex,               // uint32_t                     dstQueueFamilyIndex
                        kraut.SwapChain.Images[i].Handle,             // VkImage                      image
                        imageSubresourceRange                         // VkImageSubresourceRange      subresourceRange
                };
                cmdPipelineBarrier(kraut.GraphicsBuffer.CommandBuffers[i], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrierFromDrawToPresent);
            }


            if(endCommandBuffer(kraut.GraphicsBuffer.CommandBuffers[i]) != VK_SUCCESS) {
                return false;
            }
        }

        return true;
    }

    int KrautVK::kvkInitVulkan(const char* &title, const uint32_t &width, const uint32_t &height) {

        int status = kvkCreateInstance(title);
        if (status != SUCCESS)
            return status;

        status = kvkCreateDevice();
        if (status != SUCCESS)
            return status;

        status = kvkCreateSemaphores();
        if (status != SUCCESS)
            return status;

        if(!kvkCreateSwapChain())
            return INT32_MIN;

        status = kvkCreateRenderPass();
        if (status != SUCCESS)
            return status;

        status = kvkCreateFrameBuffers(width, height);
        if (status != SUCCESS)
            return status;

        status = kvkCreatePipelines(width, height);
        if (status != SUCCESS)
            return status;

        return SUCCESS;
    }

    void KrautVK::kvkClear() {
        if (kraut.Vulkan.Device != VK_NULL_HANDLE) {
            deviceWaitIdle(kraut.Vulkan.Device);

            if ((!kraut.GraphicsBuffer.CommandBuffers.empty()) &&
                (kraut.GraphicsBuffer.CommandBuffers[0] != VK_NULL_HANDLE)) {
                freeCommandBuffers(kraut.Vulkan.Device, kraut.GraphicsBuffer.CommandPool,
                                   static_cast<uint32_t>(kraut.GraphicsBuffer.CommandBuffers.size()),
                                   kraut.GraphicsBuffer.CommandBuffers.data());
                kraut.GraphicsBuffer.CommandBuffers.clear();
            }

            if (kraut.GraphicsBuffer.CommandPool != VK_NULL_HANDLE) {
                destroyCommandPool(kraut.Vulkan.Device, kraut.GraphicsBuffer.CommandPool, nullptr);
                kraut.GraphicsBuffer.CommandPool = VK_NULL_HANDLE;
            }

            if((!kraut.GraphicsBuffer.CommandBuffers.empty()) && (kraut.GraphicsBuffer.CommandBuffers[0] != VK_NULL_HANDLE)) {
                freeCommandBuffers(kraut.Vulkan.Device, kraut.GraphicsBuffer.CommandPool, static_cast<uint32_t>(kraut.GraphicsBuffer.CommandBuffers.size()), kraut.GraphicsBuffer.CommandBuffers.data());
                kraut.GraphicsBuffer.CommandBuffers.clear();
            }

            if(kraut.GraphicsBuffer.CommandPool != VK_NULL_HANDLE) {
                destroyCommandPool(kraut.Vulkan.Device, kraut.GraphicsBuffer.CommandPool, nullptr);
                kraut.GraphicsBuffer.CommandPool = VK_NULL_HANDLE;
            }

            if(kraut.Vulkan.GraphicsPipeline != VK_NULL_HANDLE) {
                destroyPipeline(kraut.Vulkan.Device, kraut.Vulkan.GraphicsPipeline, nullptr);
                kraut.Vulkan.GraphicsPipeline = VK_NULL_HANDLE;
            }

            if(kraut.Vulkan.RenderPass != VK_NULL_HANDLE) {
                destroyRenderPass(kraut.Vulkan.Device, kraut.Vulkan.RenderPass, nullptr);
                kraut.Vulkan.RenderPass = VK_NULL_HANDLE;
            }

            for(size_t i = 0; i < kraut.Vulkan.FrameBuffers.size(); ++i) {
                if(kraut.Vulkan.FrameBuffers[i] != VK_NULL_HANDLE) {
                    destroyFramebuffer(kraut.Vulkan.Device, kraut.Vulkan.FrameBuffers[i], nullptr);
                    kraut.Vulkan.FrameBuffers[i] = VK_NULL_HANDLE;
                }
            }

            kraut.Vulkan.FrameBuffers.clear();
        }
    }

    bool KrautVK::kvkOnWindowSizeChanged(const int &width, const int &height) {
        kvkClear();

        if (!kvkCreateSwapChain()) {
            return false;
        }

        if( !kvkCreateRenderPass() ) {
            return false;
        }

        if( !kvkCreateFrameBuffers(static_cast<uint32_t>(width), static_cast<uint32_t>(height))) {
            return false;
        }

        if( !kvkCreatePipelines(static_cast<uint32_t>(width), static_cast<uint32_t>(height))) {
            return false;
        }

        return kvkCreateCommandBuffers(static_cast<uint32_t>(width), static_cast<uint32_t>(height));

    }

    int KrautVK::kvkWindowShouldClose() {
        return glfwWindowShouldClose(kraut.GLFW.Window);

    }

    bool KrautVK::kvkRenderUpdate() {

        //Ask the swapchain to give us the next image available. Image will be available after the last call's command buffers have fully processed
        uint32_t imageIndex;
        VkResult result = acquireNextImageKHR(kraut.Vulkan.Device, kraut.SwapChain.Handle, UINT64_MAX,
                                              kraut.Semaphores.ImageAvailable, VK_NULL_HANDLE, &imageIndex);
        switch (result) {
            case VK_SUCCESS:
            case VK_SUBOPTIMAL_KHR:
                break;
            case VK_ERROR_OUT_OF_DATE_KHR:
                return kvkOnWindowSizeChanged(0, 0);
            default:
                return false;
        }

        //Send commands to the hardware, via the cammand buffers
        VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
        VkSubmitInfo submitInfo = {
                VK_STRUCTURE_TYPE_SUBMIT_INFO,                      // VkStructureType              sType
                nullptr,                                            // const void                  *pNext
                1,                                                  // uint32_t                     waitSemaphoreCount
                &kraut.Semaphores.ImageAvailable,                   // const VkSemaphore           *pWaitSemaphores
                &waitDstStageMask,                                  // const VkPipelineStageFlags  *pWaitDstStageMask;
                1,                                                  // uint32_t                     commandBufferCount
                &kraut.GraphicsBuffer.CommandBuffers[imageIndex],    // const VkCommandBuffer       *pCommandBuffers
                1,                                                  // uint32_t                     signalSemaphoreCount
                &kraut.Semaphores.RenderingFinished                 // const VkSemaphore           *pSignalSemaphores
        };

        if (queueSubmit(kraut.PresentQueue.Handle, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
            return false;
        }

        //"Return" the processed image to the swapchain for presentation
        VkPresentInfoKHR presentInfo = {
                VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,           // VkStructureType              sType
                nullptr,                                      // const void                  *pNext
                1,                                            // uint32_t                     waitSemaphoreCount
                &kraut.Semaphores.RenderingFinished,          // const VkSemaphore           *pWaitSemaphores
                1,                                            // uint32_t                     swapchainCount
                &kraut.SwapChain.Handle,                      // const VkSwapchainKHR        *pSwapchains
                &imageIndex,                                  // const uint32_t              *pImageIndices
                nullptr                                       // VkResult                    *pResults
        };
        result = queuePresentKHR(kraut.PresentQueue.Handle, &presentInfo);

        switch (result) {
            case VK_SUCCESS:
                break;
            case VK_ERROR_OUT_OF_DATE_KHR:
            case VK_SUBOPTIMAL_KHR:
                return kvkOnWindowSizeChanged(0, 0);
            default:
                return false;
        }

        return true;
    }

    void KrautVK::kvkPollEvents() {
        glfwPollEvents();
    }

    void KrautVK::kvkTerminate() {
        printf("KrautVK terminating\n");
        kvkClear();

        if (kraut.Vulkan.Device != VK_NULL_HANDLE) {
            deviceWaitIdle(kraut.Vulkan.Device);

            if((kraut.GraphicsBuffer.CommandBuffers.empty()) && (kraut.GraphicsBuffer.CommandBuffers[0] != VK_NULL_HANDLE)) {
                freeCommandBuffers(kraut.Vulkan.Device, kraut.GraphicsBuffer.CommandPool, static_cast<uint32_t>(kraut.GraphicsBuffer.CommandBuffers.size()), &kraut.GraphicsBuffer.CommandBuffers[0]);
                kraut.GraphicsBuffer.CommandBuffers.clear();
            }

            if(kraut.GraphicsBuffer.CommandPool != VK_NULL_HANDLE) {
                destroyCommandPool(kraut.Vulkan.Device, kraut.GraphicsBuffer.CommandPool, nullptr);
                kraut.GraphicsBuffer.CommandPool = VK_NULL_HANDLE;
            }

            if(kraut.Vulkan.GraphicsPipeline != VK_NULL_HANDLE) {
                destroyPipeline(kraut.Vulkan.Device, kraut.Vulkan.GraphicsPipeline, nullptr);
                kraut.Vulkan.GraphicsPipeline = VK_NULL_HANDLE;
            }

            if(kraut.Vulkan.RenderPass != VK_NULL_HANDLE) {
                destroyRenderPass(kraut.Vulkan.Device, kraut.Vulkan.RenderPass, nullptr);
                kraut.Vulkan.RenderPass = VK_NULL_HANDLE;
            }

            for(size_t i = 0; i < kraut.Vulkan.FrameBuffers.size(); ++i) {
                if(kraut.Vulkan.FrameBuffers[i] != VK_NULL_HANDLE) {
                    destroyFramebuffer(kraut.Vulkan.Device, kraut.Vulkan.FrameBuffers[i], nullptr);
                    kraut.Vulkan.FrameBuffers[i] = VK_NULL_HANDLE;
                }

                if(kraut.SwapChain.Images[i].View != VK_NULL_HANDLE) {
                    destroyImageView(kraut.Vulkan.Device, kraut.SwapChain.Images[i].View, nullptr);
                    kraut.SwapChain.Images[i].View = VK_NULL_HANDLE;
                }
            }
            kraut.Vulkan.FrameBuffers.clear();

            if (kraut.Semaphores.ImageAvailable != VK_NULL_HANDLE) {
                destroySemaphore(kraut.Vulkan.Device, kraut.Semaphores.ImageAvailable, nullptr);
            }
            if (kraut.Semaphores.RenderingFinished != VK_NULL_HANDLE) {
                destroySemaphore(kraut.Vulkan.Device, kraut.Semaphores.RenderingFinished, nullptr);
            }
            if (kraut.SwapChain.Handle != VK_NULL_HANDLE) {
                destroySwapchainKHR(kraut.Vulkan.Device, kraut.SwapChain.Handle, nullptr);
            }
            destroyDevice(kraut.Vulkan.Device, nullptr);
        }

        if (kraut.Vulkan.ApplicationSurface != VK_NULL_HANDLE) {
            destroySurfaceKHR(kraut.Vulkan.Instance, kraut.Vulkan.ApplicationSurface, nullptr);
        }

        if (kraut.Vulkan.Instance != VK_NULL_HANDLE) {
            destroyInstance(kraut.Vulkan.Instance, nullptr);
        }
        glfwTerminate();

    }

    int KrautVK::kvkCreateRenderPass() {
        VkAttachmentDescription attachmentDescription[] = {
                {
                        0,                                   // VkAttachmentDescriptionFlags   flags
                        kraut.SwapChain.Format,              // VkFormat                       format
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

        VkRenderPassCreateInfo renderPassCreateInfo = {
                VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,    // VkStructureType                sType
                nullptr,                                      // const void                    *pNext
                0,                                            // VkRenderPassCreateFlags        flags
                1,                                            // uint32_t                       attachmentCount
                attachmentDescription,                        // const VkAttachmentDescription *pAttachments
                1,                                            // uint32_t                       subpassCount
                subpassDescriptions,                          // const VkSubpassDescription    *pSubpasses
                0,                                            // uint32_t                       dependencyCount
                nullptr                                       // const VkSubpassDependency     *pDependencies
        };

        if(createRenderPass(kraut.Vulkan.Device, &renderPassCreateInfo, nullptr, &kraut.Vulkan.RenderPass) == VK_SUCCESS){
            return SUCCESS;
        } else{
            return VULKAN_RENDERPASS_CREATION_FAILED;
        }


    }

    int KrautVK::kvkCreateFrameBuffers(const uint32_t &width, const uint32_t &height) {

        kraut.Vulkan.FrameBuffers.resize(kraut.SwapChain.Images.size());

        for (size_t i = 0; i < kraut.SwapChain.Images.size(); ++i) {
            VkImageViewCreateInfo imageViewCreateInfo = {
                    VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,           // VkStructureType                sType
                    nullptr,                                            // const void                    *pNext
                    0,                                                  // VkImageViewCreateFlags         flags
                    kraut.SwapChain.Images[i].Handle,                   // VkImage                        image
                    VK_IMAGE_VIEW_TYPE_2D,                              // VkImageViewType                viewType
                    kraut.SwapChain.Format,                             // VkFormat                       format

                    {                                                   // VkComponentMapping             components
                            VK_COMPONENT_SWIZZLE_IDENTITY,              // VkComponentSwizzle             r
                            VK_COMPONENT_SWIZZLE_IDENTITY,              // VkComponentSwizzle             g
                            VK_COMPONENT_SWIZZLE_IDENTITY,              // VkComponentSwizzle             b
                            VK_COMPONENT_SWIZZLE_IDENTITY               // VkComponentSwizzle             a
                    },

                    {                                                   // VkImageSubresourceRange        subresourceRange
                            VK_IMAGE_ASPECT_COLOR_BIT,                  // VkImageAspectFlags             aspectMask
                            0,                                          // uint32_t                       baseMipLevel
                            1,                                          // uint32_t                       levelCount
                            0,                                          // uint32_t                       baseArrayLayer
                            1                                           // uint32_t                       layerCount
                    }
            };



            if(createImageView(kraut.Vulkan.Device, &imageViewCreateInfo, nullptr, &kraut.SwapChain.Images[i].View) != VK_SUCCESS) {
                return VULKAN_FRAMEBUFFERS_CREATION_FAILED;
            }


            VkFramebufferCreateInfo framebufferCreateInfo = {
                    VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,  // VkStructureType                sType
                    nullptr,                                    // const void                    *pNext
                    0,                                          // VkFramebufferCreateFlags       flags
                    kraut.Vulkan.RenderPass,                    // VkRenderPass                   renderPass
                    1,                                          // uint32_t                       attachmentCount
                    &kraut.SwapChain.Images[i].View,            // const VkImageView             *pAttachments
                    width,                                      // uint32_t                       width
                    height,                                     // uint32_t                       height
                    1                                           // uint32_t                       layers
            };

            if (createFramebuffer(kraut.Vulkan.Device, &framebufferCreateInfo, nullptr, &kraut.Vulkan.FrameBuffers[i]) != VK_SUCCESS) {
                return VULKAN_FRAMEBUFFERS_CREATION_FAILED;
            }
        }

        return SUCCESS;
    }

    int KrautVK::kvkCreatePipelines(const uint32_t &width, const uint32_t &height){
        GarbageCollector<VkShaderModule, PFN_vkDestroyShaderModule> vertexShaderModule = kvkLoadShader(Tools::rootPath + std::string("/data/shadervert.spv"));
        GarbageCollector<VkShaderModule, PFN_vkDestroyShaderModule> fragmentShaderModule = kvkLoadShader(Tools::rootPath + std::string("/data/shaderfrag.spv"));

        if( !vertexShaderModule || !fragmentShaderModule ) {
            return VULKAN_PIPELINES_CREATION_FAILED;
        }

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

        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {
                VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,    // VkStructureType                                sType
                nullptr,                                                      // const void                                    *pNext
                0,                                                            // VkPipelineVertexInputStateCreateFlags          flags;
                0,                                                            // uint32_t                                       vertexBindingDescriptionCount
                nullptr,                                                      // const VkVertexInputBindingDescription         *pVertexBindingDescriptions
                0,                                                            // uint32_t                                       vertexAttributeDescriptionCount
                nullptr                                                       // const VkVertexInputAttributeDescription       *pVertexAttributeDescriptions
        };

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {
                VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,  // VkStructureType                                sType
                nullptr,                                                      // const void                                    *pNext
                0,                                                            // VkPipelineInputAssemblyStateCreateFlags        flags
                KVK_PRIMITIVE_TOPOLOGY,                                       // VkPrimitiveTopology                            topology
                VK_FALSE                                                      // VkBool32                                       primitiveRestartEnable
        };

        VkViewport viewport = {
                0.0f,                                                         // float           Origin X
                0.0f,                                                         // float           Origin Y
                (float) width,                                                // float           Width
                (float) height,                                               // float           Height
                0.0f,                                                         //gh float           Near
                1.0f                                                          // float           Far
        };

        VkRect2D scissor = {
                {                                                             // VkOffset2D
                        0,                                                    // int32_t         Origin X
                        0                                                     // int32_t         Origin Y
                },
                {                                                             // VkExtent2D
                        width,                                                // int32_t         Width
                        height                                                // int32_t         Height
                }
        };

        VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {
                VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,        // VkStructureType                                sType
                nullptr,                                                      // const void                                    *pNext
                0,                                                            // VkPipelineViewportStateCreateFlags             flags
                1,                                                            // uint32_t                                       viewportCount
                &viewport,                                                    // const VkViewport                              *pViewports
                1,                                                            // uint32_t                                       scissorCount
                &scissor                                                      // const VkRect2D                                *pScissors
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

        GarbageCollector<VkPipelineLayout, PFN_vkDestroyPipelineLayout> pipelineLayout = kvkLoadPipelineLayout();
        if(!pipelineLayout) {
            return VULKAN_PIPELINES_CREATION_FAILED;
        }

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
                nullptr,                                                        // const VkPipelineDynamicStateCreateInfo        *pDynamicState
                pipelineLayout.get(),                                           // VkPipelineLayout                               layout
                kraut.Vulkan.RenderPass,                                        // VkRenderPass                                   renderPass
                0,                                                              // uint32_t                                       subpass
                VK_NULL_HANDLE,                                                 // VkPipeline                                     basePipelineHandle
                -1                                                              // int32_t                                        basePipelineIndex
        };


        if(createGraphicsPipelines(kraut.Vulkan.Device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &kraut.Vulkan.GraphicsPipeline) == VK_SUCCESS){
            return SUCCESS;

        } else{
            return VULKAN_PIPELINES_CREATION_FAILED;

        };

    }

    GarbageCollector<VkPipelineLayout, PFN_vkDestroyPipelineLayout> KrautVK::kvkLoadPipelineLayout(){

        VkPipelineLayoutCreateInfo layoutCreateInfo = {
                VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,  // VkStructureType                sType
                nullptr,                                        // const void                    *pNext
                0,                                              // VkPipelineLayoutCreateFlags    flags
                0,                                              // uint32_t                       setLayoutCount
                nullptr,                                        // const VkDescriptorSetLayout   *pSetLayouts
                0,                                              // uint32_t                       pushConstantRangeCount
                nullptr                                         // const VkPushConstantRange     *pPushConstantRanges
        };

        VkPipelineLayout pipelineLayout;
        if(createPipelineLayout(kraut.Vulkan.Device, &layoutCreateInfo, nullptr, &pipelineLayout ) != VK_SUCCESS) {
            return GarbageCollector<VkPipelineLayout, PFN_vkDestroyPipelineLayout>();
        }

        return GarbageCollector<VkPipelineLayout, PFN_vkDestroyPipelineLayout>(pipelineLayout, destroyPipelineLayout, kraut.Vulkan.Device);

    }

    GarbageCollector<VkShaderModule, PFN_vkDestroyShaderModule> KrautVK::kvkLoadShader(std::string const &filename) {
        const std::vector<char> spv = Tools::getBinaryData(filename);
        if(spv.empty()) {
            return GarbageCollector<VkShaderModule, PFN_vkDestroyShaderModule>();
        }

        VkShaderModuleCreateInfo shaderModuleCreateInfo = {
                VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,    // VkStructureType                sType
                nullptr,                                        // const void                    *pNext
                0,                                              // VkShaderModuleCreateFlags      flags
                spv.size(),                                     // size_t                         codeSize
                reinterpret_cast<const uint32_t*>(&spv[0])      // const uint32_t                *pCode
        };

        VkShaderModule shaderModule;
        if(createShaderModule(kraut.Vulkan.Device, &shaderModuleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            return GarbageCollector<VkShaderModule, PFN_vkDestroyShaderModule>();
        }

        return GarbageCollector<VkShaderModule, PFN_vkDestroyShaderModule>(shaderModule, destroyShaderModule, kraut.Vulkan.Device);

    }

    int KrautVK::kvkInit(const int &width, const int &height, const char *title, const int &fullScreen) {
        std::cout << "\nKrautVK Alpha v" << major << "." << minor << "." << patch
                  << "\nCopyright 2018 Jonathan Crock./,ett\n\n"
                  << "Licensed under the Apache License, Version 2.0 (the \"License\");\n"
                  << "you may not use this file except in compliance with the License.\n"
                  << "You may obtain a copy of the License at\n\n"
                  << "http://www.apache.org/licenses/LICENSE-2.0\n\n"
                  << "Unless required by applicable law or agreed to in writing, software\n"
                  << "distributed under the License is distributed on an \"AS IS\" BASIS,\n"
                  << "WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n"
                  << "See the License for the specific language governing permissions and\n"
                  << "limitations under the License.\n\n";

        printf("Initializing GLFW\n");
        int status = kvkInitGLFW(width, height, title, fullScreen);
        if (status != SUCCESS)
            return status;

        status = kvkInitVulkan(title, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
        if (status != SUCCESS)
            return status;

        if (!kvkCreateCommandBuffers(static_cast<uint32_t>(width), static_cast<uint32_t>(height)))
            return INT32_MIN;

        printf("KrautVK Alpha Initialized!\n");

        return SUCCESS;
    }

    EXPORT int KrautInit(int width, int height, char *title, int fullScreen, char *dllPath) {
        //dllPath exists so as to let the calling .NET Core decide where the root of the dll is
        //as finding it within execution of the framework in this dll is possible but "janky"

        std::string rootPath = dllPath;
        Tools::findAndReplace(rootPath, std::string("\\"), std::string("/"));

        Tools::rootPath = rootPath;
        return KrautVK::kvkInit(width, height, title, fullScreen);
    }

    EXPORT int KrautWindowShouldClose() {
        return KrautVK::kvkWindowShouldClose();
    }

    EXPORT void KrautPollEvents() {
        KrautVK::kvkPollEvents();
    }

    EXPORT void KrautTerminate() {
        KrautVK::kvkTerminate();
    }

    EXPORT void KrautDraw() {
        KrautVK::kvkRenderUpdate();
    }
}