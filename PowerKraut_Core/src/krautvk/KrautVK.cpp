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

    KrautVKParameters KrautVK::kvk;

    int KrautVK::kvkInitGLFW(int width, int height, char *title, int fullScreen) {

        if (!glfwInit())
            return GLFW_INIT_FAILED;

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, false);

        kvk.monitor = glfwGetPrimaryMonitor();

        if (fullScreen)
            kvk.window = glfwCreateWindow(width, height, title, kvk.monitor, nullptr);
        else
            kvk.window = glfwCreateWindow(width, height, title, nullptr, nullptr);



        if (!kvk.window) {
            return GLFW_WINDOW_CREATION_FAILED;
        }

        glfwSetWindowSizeCallback(kvk.window, [] (GLFWwindow* unusedWindow, int unusedWidth, int unusedHeight) {
            kvkOnWindowSizeChanged();
        });

        return SUCCESS;
    }

    int KrautVK::kvkCheckExtensionAvailability(const char *extensionName, const std::vector<VkExtensionProperties> &availableExtensions) {
        for( size_t i = 0; i < availableExtensions.size(); ++i ) {
            if( strcmp( availableExtensions[i].extensionName, extensionName ) == 0 ) {
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
        if( (enumerateDeviceExtensionProperties( physicalDevice, nullptr, &extensionsCount, nullptr ) != VK_SUCCESS) || (extensionsCount == 0) ) {
            return false;
        }

        std::vector<VkExtensionProperties> availableExtensions( extensionsCount );
        if( enumerateDeviceExtensionProperties( physicalDevice, nullptr, &extensionsCount, &availableExtensions[0] ) != VK_SUCCESS ) {
            return false;
        }

        std::vector<const char*> deviceExtensions;
        kvkGetRequiredDeviceExtensions(deviceExtensions);

        for( size_t i = 0; i < deviceExtensions.size(); ++i ) {
            if( !kvkCheckExtensionAvailability( deviceExtensions[i], availableExtensions ) ) {
                return false;
            }
        }

        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;

        getPhysicalDeviceProperties(physicalDevice, &deviceProperties);
        getPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

        uint32_t majorVersion = VK_VERSION_MAJOR(deviceProperties.apiVersion);
        uint32_t minorVersion = VK_VERSION_MINOR(deviceProperties.apiVersion);  //Reserved for when it's time to expect finer version requirements
        uint32_t patchVersion = VK_VERSION_PATCH(deviceProperties.apiVersion);

        if ((majorVersion < 1) || (deviceProperties.limits.maxImageDimension2D < 4096)) {
            return false;
        }

        uint32_t qFamilyCount = 0;
        getPhysicalDeviceQueueFamilyProperties(physicalDevice, &qFamilyCount, nullptr);
        if (qFamilyCount == 0) {
            return false;
        }

        std::vector <VkQueueFamilyProperties> qFamilyProperties(qFamilyCount);
        getPhysicalDeviceQueueFamilyProperties(physicalDevice, &qFamilyCount, qFamilyProperties.data());

        uint32_t currentGraphicsQueueFamilyIndex = UINT32_MAX;
        uint32_t currentPresentationQueueFamilyIndex = UINT32_MAX;

        //If a queue supports both graphics and presentation then use it. otherwise use separate queues.
        // If theres not a compatible buffer for either feature, the device is not compatible
        for (uint32_t i = 0; i < qFamilyCount ; ++i) {
            if ((qFamilyProperties[i].queueCount > 0) && (qFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && currentGraphicsQueueFamilyIndex == UINT32_MAX)
                currentGraphicsQueueFamilyIndex = i;

            if(glfwGetPhysicalDevicePresentationSupport(kvk.instance, physicalDevice, i) && currentPresentationQueueFamilyIndex == UINT32_MAX){
                currentPresentationQueueFamilyIndex = i;
            }

            if((currentGraphicsQueueFamilyIndex != UINT32_MAX && currentPresentationQueueFamilyIndex != UINT32_MAX)){
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
        uint32_t  count;
        const char** reqdExtensions = glfwGetRequiredInstanceExtensions(&count);

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

        if (createInstance(&instanceCreateInfo, nullptr, &kvk.instance) != SUCCESS)
            return VULKAN_INSTANCE_CREATION_FAILED;

        printf("Loading instance level procedures...\n");
        createDevice = (PFN_vkCreateDevice) glfwGetInstanceProcAddress(kvk.instance, "vkCreateDevice");
        enumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices) glfwGetInstanceProcAddress(kvk.instance, "vkEnumeratePhysicalDevices");
        getPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties) glfwGetInstanceProcAddress(kvk.instance, "vkGetPhysicalDeviceProperties");
        getPhysicalDeviceFeatures = (PFN_vkGetPhysicalDeviceFeatures) glfwGetInstanceProcAddress(kvk.instance, "vkGetPhysicalDeviceFeatures");
        getPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties) glfwGetInstanceProcAddress(kvk.instance, "vkGetPhysicalDeviceQueueFamilyProperties");
        destroyInstance = (PFN_vkDestroyInstance) glfwGetInstanceProcAddress(kvk.instance, "vkDestroyInstance");
        destroySurfaceKHR = (PFN_vkDestroySurfaceKHR) glfwGetInstanceProcAddress(kvk.instance, "vkDestroySurfaceKHR");
        enumerateDeviceExtensionProperties = (PFN_vkEnumerateDeviceExtensionProperties) glfwGetInstanceProcAddress(kvk.instance, "vkEnumerateDeviceExtensionProperties");
        getPhysicalDeviceSurfaceCapabilitiesKHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR) glfwGetInstanceProcAddress(kvk.instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
        getPhysicalDeviceSurfaceFormatsKHR = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR) glfwGetInstanceProcAddress(kvk.instance, "vkGetPhysicalDeviceSurfaceFormatsKHR");
        getPhysicalDeviceSurfacePresentModesKHR = (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR) glfwGetInstanceProcAddress(kvk.instance, "vkGetPhysicalDeviceSurfacePresentModesKHR");


        return SUCCESS;
    }

    int KrautVK::kvkCreateDevice() {
        if(glfwCreateWindowSurface(kvk.instance, kvk.window, nullptr, &kvk.applicationSurface))
            return VULKAN_SURFACE_CREATION_FAILED;

        //INITIALIZE PHYSICAL DEVICES
        printf("Enumerating physical devices...\n");
        uint32_t deviceCount;
        if (enumeratePhysicalDevices(kvk.instance, &deviceCount, nullptr) != SUCCESS || deviceCount == 0)
            return VULKAN_NOT_SUPPORTED;

        std::vector <VkPhysicalDevice> devices(deviceCount);
        if (enumeratePhysicalDevices(kvk.instance, &deviceCount, &devices[0]) != SUCCESS)
            return VULKAN_NOT_SUPPORTED;

        uint32_t selectedGraphicsQueueFamilyIndex = UINT32_MAX;
        uint32_t selectedPresentationQueueFamilyIndex = UINT32_MAX;

        for (uint32_t i = 0; i < deviceCount; i++) {
            if (kvkCheckDeviceProperties(devices[i], selectedGraphicsQueueFamilyIndex, selectedPresentationQueueFamilyIndex)) {
                kvk.physicalDevice = devices[i];
                break;
            }
        }

        if (kvk.physicalDevice == nullptr)
            return VULKAN_NOT_SUPPORTED;

        std::vector<VkDeviceQueueCreateInfo> qCreateInfos;
        std::vector<float> queuePriorities = {1.0f};

        qCreateInfos.push_back( {
                                        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,     // VkStructureType              sType
                                        nullptr,                                        // const void                  *pNext
                                        0,                                              // VkDeviceQueueCreateFlags     flags
                                        selectedGraphicsQueueFamilyIndex,                  // uint32_t                     queueFamilyIndex
                                        static_cast<uint32_t>(queuePriorities.size()),  // uint32_t                     queueCount
                                        &queuePriorities[0]                             // const float                 *pQueuePriorities
                                } );

        if(selectedGraphicsQueueFamilyIndex != selectedPresentationQueueFamilyIndex) {
            qCreateInfos.push_back({
                                           VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,     // VkStructureType              sType
                                           nullptr,                                        // const void                  *pNext
                                           0,                                              // VkDeviceQueueCreateFlags     flags
                                           selectedGraphicsQueueFamilyIndex,                  // uint32_t                     queueFamilyIndex
                                           static_cast<uint32_t>(queuePriorities.size()),  // uint32_t                     queueCount
                                           &queuePriorities[0]                             // const float                 *pQueuePriorities
                                   });
        }

        std::vector<const char*> extensions;
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

        if(createDevice(kvk.physicalDevice, &deviceCreateInfo, nullptr, &kvk.device) != SUCCESS)
            return VULKAN_DEVICE_CREATION_FAILED;

        printf("Loading device level procedures..\n");
        getDeviceProcAddr = (PFN_vkGetDeviceProcAddr) glfwGetInstanceProcAddress(kvk.instance, "vkGetDeviceProcAddr");
        getDeviceQueue = (PFN_vkGetDeviceQueue) getDeviceProcAddr(kvk.device, "vkGetDeviceQueue");
        deviceWaitIdle = (PFN_vkDeviceWaitIdle) getDeviceProcAddr(kvk.device, "vkDeviceWaitIdle");
        destroyDevice = (PFN_vkDestroyDevice) getDeviceProcAddr(kvk.device, "vkDestroyDevice");
        createSemaphore = (PFN_vkCreateSemaphore) getDeviceProcAddr(kvk.device, "vkCreateSemaphore");
        createSwapchainKHR = (PFN_vkCreateSwapchainKHR) getDeviceProcAddr(kvk.device, "vkCreateSwapchainKHR");
        destroySwapchainKHR = (PFN_vkDestroySwapchainKHR) getDeviceProcAddr(kvk.device, "vkDestroySwapchainKHR");
        getSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR) getDeviceProcAddr(kvk.device, "vkGetSwapchainImagesKHR");
        acquireNextImageKHR = (PFN_vkAcquireNextImageKHR) getDeviceProcAddr(kvk.device, "vkAcquireNextImageKHR");
        queuePresentKHR = (PFN_vkQueuePresentKHR) getDeviceProcAddr(kvk.device, "vkQueuePresentKHR");
        queueSubmit = (PFN_vkQueueSubmit) getDeviceProcAddr(kvk.device, "vkQueueSubmit");
        createCommandPool = (PFN_vkCreateCommandPool) getDeviceProcAddr(kvk.device, "vkCreateCommandPool");
        allocateCommandBuffers = (PFN_vkAllocateCommandBuffers) getDeviceProcAddr(kvk.device, "vkAllocateCommandBuffers");
        freeCommandBuffers = (PFN_vkFreeCommandBuffers) getDeviceProcAddr(kvk.device, "vkFreeCommandBuffers");
        destroyCommandPool = (PFN_vkDestroyCommandPool) getDeviceProcAddr(kvk.device, "vkDestroyCommandPool");
        destroySemaphore = (PFN_vkDestroySemaphore) getDeviceProcAddr(kvk.device, "vkDestroySemaphore");
        beginCommandBuffer = (PFN_vkBeginCommandBuffer) getDeviceProcAddr(kvk.device, "vkBeginCommandBuffer");
        cmdPipelineBarrier = (PFN_vkCmdPipelineBarrier) getDeviceProcAddr(kvk.device, "vkCmdPipelineBarrier");
        cmdClearColorImage = (PFN_vkCmdClearColorImage) getDeviceProcAddr(kvk.device, "vkCmdClearColorImage");
        endCommandBuffer = (PFN_vkEndCommandBuffer) getDeviceProcAddr(kvk.device, "vkEndCommandBuffer");


        //INITIALIZE COMMAND BUFFER
        printf("Initializing command buffers..\n");
        kvk.graphicsQueueFamilyIndex = selectedGraphicsQueueFamilyIndex;
        kvk.presentationQueueFamilyIndex = selectedPresentationQueueFamilyIndex;

        getDeviceQueue(kvk.device, kvk.graphicsQueueFamilyIndex, 0, &kvk.graphicsQueue);
        getDeviceQueue(kvk.device, kvk.presentationQueueFamilyIndex, 0, &kvk.presentationQueue);

        return SUCCESS;
    }

    int KrautVK::kvkCreateSemaphores() {
        printf("Creating Semaphores...\n");

        VkSemaphoreCreateInfo semaphore_create_info = {
                VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,      // VkStructureType          sType
                nullptr,                                      // const void*              pNext
                0                                             // VkSemaphoreCreateFlags   flags
        };

        if((createSemaphore(kvk.device, &semaphore_create_info, nullptr, &kvk.imageAvailableSemaphore) != VK_SUCCESS) ||
            (createSemaphore(kvk.device, &semaphore_create_info, nullptr, &kvk.renderingFinishedSemaphore) != VK_SUCCESS)) {
            return VULKAN_SEMAPHORE_CREATION_FAILED;
        }

        return SUCCESS;
    }

    bool KrautVK::kvkCreateSwapChain() {

        if(kvk.device != VK_NULL_HANDLE) {
            deviceWaitIdle(kvk.device);
        }
        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        if(getPhysicalDeviceSurfaceCapabilitiesKHR(kvk.physicalDevice, kvk.applicationSurface, &surfaceCapabilities) != VK_SUCCESS) {
            return false;
        }

        uint32_t formatsCount;
        if((getPhysicalDeviceSurfaceFormatsKHR(kvk.physicalDevice, kvk.applicationSurface, &formatsCount, nullptr) != VK_SUCCESS) ||
            (formatsCount == 0)) {
            return false;
        }

        std::vector<VkSurfaceFormatKHR> surfaceFormats(formatsCount);
        if(getPhysicalDeviceSurfaceFormatsKHR(kvk.physicalDevice, kvk.applicationSurface, &formatsCount, surfaceFormats.data()) != VK_SUCCESS) {
            return false;
        }

        uint32_t presentModesCount;
        if((getPhysicalDeviceSurfacePresentModesKHR(kvk.physicalDevice, kvk.applicationSurface, &presentModesCount, nullptr) != VK_SUCCESS) ||
            (presentModesCount == 0) ) {
            return false;
        }

        std::vector<VkPresentModeKHR> presentModes(presentModesCount);
        if(getPhysicalDeviceSurfacePresentModesKHR(kvk.physicalDevice, kvk.applicationSurface, &presentModesCount, presentModes.data()) != VK_SUCCESS ) {
            return false;
        }

        uint32_t                      desiredNumberOfImages = kvkGetSwapChainNumImages(surfaceCapabilities);
        VkSurfaceFormatKHR            desiredFormat = kvkGetSwapChainFormat(surfaceFormats);
        VkExtent2D                    desiredExtent = kvkGetSwapChainExtent(surfaceCapabilities);
        VkImageUsageFlags             desiredUsage = kvkGetSwapChainUsageFlags(surfaceCapabilities);
        VkSurfaceTransformFlagBitsKHR desiredTransform = kvkGetSwapChainTransform(surfaceCapabilities);
        VkPresentModeKHR              desiredPresentMode = kvkGetSwapChainPresentMode(presentModes);
        VkSwapchainKHR                oldSwapChain = kvk.swapchain;

        if(static_cast<int>(desiredUsage) == -1) {
            return false;
        }

        if(static_cast<int>(desiredPresentMode) == -1) {
            return false;
        }

        if((desiredExtent.width == 0) || (desiredExtent.height == 0)) {
            // Some asshole minimized the window, but I guess it's fine
            return SUCCESS;
        }

        VkSwapchainCreateInfoKHR swap_chain_create_info = {
                VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,    // VkStructureType                sType
                nullptr,                                        // const void                    *pNext
                0,                                              // VkSwapchainCreateFlagsKHR      flags
                kvk.applicationSurface,                             // VkSurfaceKHR                   surface
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

        if(createSwapchainKHR(kvk.device, &swap_chain_create_info, nullptr, &kvk.swapchain ) != VK_SUCCESS) {
            return false;
        }
        if(oldSwapChain != VK_NULL_HANDLE) {
            destroySwapchainKHR(kvk.device, oldSwapChain, nullptr);
        }

        return true;
    }

    uint32_t KrautVK::kvkGetSwapChainNumImages(VkSurfaceCapabilitiesKHR surfaceCapabilities) {
        // Set of images defined in a swap chain may not always be available for application to render to:
        // One may be displayed and one may wait in a queue to be presented
        // If application wants to use more images at the same time it must ask for more images
        uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
        if( (surfaceCapabilities.maxImageCount > 0) &&
            (imageCount > surfaceCapabilities.maxImageCount) ) {
            imageCount = surfaceCapabilities.maxImageCount;
        }
        return imageCount;
    }

    VkSurfaceFormatKHR KrautVK::kvkGetSwapChainFormat(std::vector<VkSurfaceFormatKHR> surfaceFormats) {
        // If the list contains only one entry with undefined format
        // it means that there are no preferred surface formats and any can be chosen
        if((surfaceFormats.size() == 1) &&
            (surfaceFormats[0].format == VK_FORMAT_UNDEFINED)) {
            return{VK_FORMAT_R8G8B8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR};
        }

        // Check if list contains most widely used R8 G8 B8 A8 format
        // with nonlinear color space
        for(VkSurfaceFormatKHR &surfaceFormat : surfaceFormats) {
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
        if( surfaceCapabilities.currentExtent.width == -1 ) {
            VkExtent2D swapChainExtent = { 640, 480 };
            if( swapChainExtent.width < surfaceCapabilities.minImageExtent.width ) {
                swapChainExtent.width = surfaceCapabilities.minImageExtent.width;
            }
            if( swapChainExtent.height < surfaceCapabilities.minImageExtent.height ) {
                swapChainExtent.height = surfaceCapabilities.minImageExtent.height;
            }
            if( swapChainExtent.width > surfaceCapabilities.maxImageExtent.width ) {
                swapChainExtent.width = surfaceCapabilities.maxImageExtent.width;
            }
            if( swapChainExtent.height > surfaceCapabilities.maxImageExtent.height ) {
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
        if( surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT ) {
            return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        }

        std::cout << "VK_IMAGE_USAGE_TRANSFER_DST image usage is not supported by the swap chain!" << std::endl
                  << "Supported swap chain's image usages include:" << std::endl
                  << (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT              ? "    VK_IMAGE_USAGE_TRANSFER_SRC\n" : "")
                  << (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT              ? "    VK_IMAGE_USAGE_TRANSFER_DST\n" : "")
                  << (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT                   ? "    VK_IMAGE_USAGE_SAMPLED\n" : "")
                  << (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_STORAGE_BIT                   ? "    VK_IMAGE_USAGE_STORAGE\n" : "")
                  << (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT          ? "    VK_IMAGE_USAGE_COLOR_ATTACHMENT\n" : "")
                  << (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT  ? "    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT\n" : "")
                  << (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT      ? "    VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT\n" : "")
                  << (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT          ? "    VK_IMAGE_USAGE_INPUT_ATTACHMENT" : "")
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
        if( surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR ) {
            return VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        } else {
            return surfaceCapabilities.currentTransform;
        }
    }

    VkPresentModeKHR KrautVK::kvkGetSwapChainPresentMode(std::vector<VkPresentModeKHR> presentModes) {
        // FIFO present mode is always available
        // MAILBOX is the lowest latency V-Sync enabled mode (something like triple-buffering) so use it if available
        for(VkPresentModeKHR &presentMode : presentModes) {
            if(presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return presentMode;
            }
        }
        for(VkPresentModeKHR &presentMode : presentModes) {
            if(presentMode == VK_PRESENT_MODE_FIFO_KHR) {
                return presentMode;
            }
        }

        return static_cast<VkPresentModeKHR>(-1);
    }

    bool KrautVK::kvkCreateCommandBuffers() {
        VkCommandPoolCreateInfo cmdPoolCreateInfo = {
                VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,     // VkStructureType              sType
                nullptr,                                        // const void*                  pNext
                0,                                              // VkCommandPoolCreateFlags     flags
                kvk.presentationQueueFamilyIndex                    // uint32_t                     queueFamilyIndex
        };

        if(createCommandPool(kvk.device, &cmdPoolCreateInfo, nullptr, &kvk.presentationCmdPool) != VK_SUCCESS) {
            return false;
        }

        uint32_t imageCount = 0;
        if((getSwapchainImagesKHR(kvk.device, kvk.swapchain, &imageCount, nullptr) != VK_SUCCESS) || (imageCount == 0) ) {
            return false;
        }

        kvk.presentationCmdBuffers.resize(imageCount);

        VkCommandBufferAllocateInfo cmd_buffer_allocate_info = {
                VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, // VkStructureType              sType
                nullptr,                                        // const void*                  pNext
                kvk.presentationCmdPool,                            // VkCommandPool                commandPool
                VK_COMMAND_BUFFER_LEVEL_PRIMARY,                // VkCommandBufferLevel         level
                imageCount                                      // uint32_t                     bufferCount
        };

        if(allocateCommandBuffers(kvk.device, &cmd_buffer_allocate_info, &kvk.presentationCmdBuffers[0] ) != VK_SUCCESS) {
            return false;
        }

        return kvkRecordCommandBuffers();

    }

    bool KrautVK::kvkRecordCommandBuffers() {
        uint32_t imageCount = static_cast<uint32_t>(kvk.presentationCmdBuffers.size());

        std::vector<VkImage> swapChainImages( imageCount );
        if( getSwapchainImagesKHR(kvk.device, kvk.swapchain, &imageCount, &swapChainImages[0]) != VK_SUCCESS) {
            std::cout << "Could not get swap chain images!" << std::endl;
            return false;
        }

        VkCommandBufferBeginInfo cmdBufferBeginInfo = {
                VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,  // VkStructureType                        sType
                nullptr,                                      // const void                            *pNext
                VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, // VkCommandBufferUsageFlags              flags
                nullptr                                       // const VkCommandBufferInheritanceInfo  *pInheritanceInfo
        };

        VkClearColorValue clearColor = {
                { 0.2f, 0.5f, 1.0f, 0.0f }
        };

        VkImageSubresourceRange imageSubresourceRange = {
                VK_IMAGE_ASPECT_COLOR_BIT,                    // VkImageAspectFlags                     aspectMask
                0,                                            // uint32_t                               baseMipLevel
                1,                                            // uint32_t                               levelCount
                0,                                            // uint32_t                               baseArrayLayer
                1                                             // uint32_t                               layerCount
        };

        for( uint32_t i = 0; i < imageCount; ++i ) {
            VkImageMemoryBarrier barrierFromPresentToClear = {
                    VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,     // VkStructureType                        sType
                    nullptr,                                    // const void                            *pNext
                    VK_ACCESS_MEMORY_READ_BIT,                  // VkAccessFlags                          srcAccessMask
                    VK_ACCESS_TRANSFER_WRITE_BIT,               // VkAccessFlags                          dstAccessMask
                    VK_IMAGE_LAYOUT_UNDEFINED,                  // VkImageLayout                          oldLayout
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,       // VkImageLayout                          newLayout
                    VK_QUEUE_FAMILY_IGNORED,                    // uint32_t                               srcQueueFamilyIndex
                    VK_QUEUE_FAMILY_IGNORED,                    // uint32_t                               dstQueueFamilyIndex
                    swapChainImages[i],                         // VkImage                                image
                    imageSubresourceRange                       // VkImageSubresourceRange                subresourceRange
            };

            VkImageMemoryBarrier barrierFromClearToPresent = {
                    VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,     // VkStructureType                        sType
                    nullptr,                                    // const void                            *pNext
                    VK_ACCESS_TRANSFER_WRITE_BIT,               // VkAccessFlags                          srcAccessMask
                    VK_ACCESS_MEMORY_READ_BIT,                  // VkAccessFlags                          dstAccessMask
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,       // VkImageLayout                          oldLayout
                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,            // VkImageLayout                          newLayout
                    VK_QUEUE_FAMILY_IGNORED,                    // uint32_t                               srcQueueFamilyIndex
                    VK_QUEUE_FAMILY_IGNORED,                    // uint32_t                               dstQueueFamilyIndex
                    swapChainImages[i],                         // VkImage                                image
                    imageSubresourceRange                       // VkImageSubresourceRange                subresourceRange
            };

            // We have to reorganize the data in order to pass specific operations on it,
            // pass those operations, then organize it back
            beginCommandBuffer(kvk.presentationCmdBuffers[i], &cmdBufferBeginInfo);
            cmdPipelineBarrier(kvk.presentationCmdBuffers[i], VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1,
                                 &barrierFromPresentToClear);

            cmdClearColorImage(kvk.presentationCmdBuffers[i], swapChainImages[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                 &clearColor, 1, &imageSubresourceRange);

            cmdPipelineBarrier(kvk.presentationCmdBuffers[i], VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1,
                                 &barrierFromClearToPresent);
            if (endCommandBuffer(kvk.presentationCmdBuffers[i]) != VK_SUCCESS) {
                std::cout << "Could not record command buffers!" << std::endl;
                return false;
            }
        }

        return true;
    }

    int KrautVK::kvkInitVulkan(const char *title) {

        int status = kvkCreateInstance(title);
        if (status != SUCCESS)
            return status;

        status = kvkCreateDevice();
        if(status != SUCCESS)
            return status;

        status = kvkCreateSemaphores();
        if(status != SUCCESS)
            return status;

        return SUCCESS;
    }

    int KrautVK::kvkClear() {
        if(kvk.device != VK_NULL_HANDLE) {
            deviceWaitIdle(kvk.device);

            if((!kvk.presentationCmdBuffers.empty()) && (kvk.presentationCmdBuffers[0] != VK_NULL_HANDLE) ) {
                freeCommandBuffers(kvk.device, kvk.presentationCmdPool, static_cast<uint32_t>(kvk.presentationCmdBuffers.size()), kvk.presentationCmdBuffers.data());
                kvk.presentationCmdBuffers.clear();
            }

            if(kvk.presentationCmdPool != VK_NULL_HANDLE ) {
                destroyCommandPool(kvk.device, kvk.presentationCmdPool, nullptr);
                kvk.presentationCmdPool = VK_NULL_HANDLE;
            }
        }
    }

    bool KrautVK::kvkOnWindowSizeChanged() {
        kvkClear();

        if(!kvkCreateSwapChain()) {
            return false;
        }

        return kvkCreateCommandBuffers();

    }

    int KrautVK::kvkInit(int width, int height, char *title, int fullScreen) {
        std::cout << "\nKrautVK Alpha v" << major << "." << minor << "." << patch
                  << "\nCopyright 2018 Jonathan Crockett\n\n"
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

        status = kvkInitVulkan(title);
        if (status != SUCCESS)
            return status;

        printf("KrautVK Alpha Initialized!\n");
        if(!kvkCreateSwapChain() || !kvkCreateCommandBuffers())
            return INT32_MIN;

        return SUCCESS;
    }

    int KrautVK::kvkWindowShouldClose() {
        return glfwWindowShouldClose(kvk.window);

    }

    bool KrautVK::kvkRenderUpdate() {

        //Ask the swapchain to give us the next image available. Image will be available after the last call's command buffers have fully processed
        uint32_t imageIndex;
        VkResult result = acquireNextImageKHR(kvk.device, kvk.swapchain, UINT64_MAX, kvk.imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
        switch(result) {
            case VK_SUCCESS:
            case VK_SUBOPTIMAL_KHR:
                break;
            case VK_ERROR_OUT_OF_DATE_KHR:
                return kvkOnWindowSizeChanged();
            default:
                return false;
        }

        //Send commands to the hardware, via the cammand buffers
        VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
        VkSubmitInfo submitInfo = {
                VK_STRUCTURE_TYPE_SUBMIT_INFO,                // VkStructureType              sType
                nullptr,                                      // const void                  *pNext
                1,                                            // uint32_t                     waitSemaphoreCount
                &kvk.imageAvailableSemaphore,                     // const VkSemaphore           *pWaitSemaphores
                &waitDstStageMask,                            // const VkPipelineStageFlags  *pWaitDstStageMask;
                1,                                            // uint32_t                     commandBufferCount
                &kvk.presentationCmdBuffers[imageIndex],          // const VkCommandBuffer       *pCommandBuffers
                1,                                            // uint32_t                     signalSemaphoreCount
                &kvk.renderingFinishedSemaphore                   // const VkSemaphore           *pSignalSemaphores
        };

        if(queueSubmit(kvk.presentationQueue, 1, &submitInfo, VK_NULL_HANDLE ) != VK_SUCCESS) {
            return false;
        }

        //"Return" the processed image to the swapchain for presentation
        VkPresentInfoKHR presentInfo = {
                VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,           // VkStructureType              sType
                nullptr,                                      // const void                  *pNext
                1,                                            // uint32_t                     waitSemaphoreCount
                &kvk.renderingFinishedSemaphore,                  // const VkSemaphore           *pWaitSemaphores
                1,                                            // uint32_t                     swapchainCount
                &kvk.swapchain,                                   // const VkSwapchainKHR        *pSwapchains
                &imageIndex,                                  // const uint32_t              *pImageIndices
                nullptr                                       // VkResult                    *pResults
        };
        result = queuePresentKHR(kvk.presentationQueue, &presentInfo);

        switch(result) {
            case VK_SUCCESS:
                break;
            case VK_ERROR_OUT_OF_DATE_KHR:
            case VK_SUBOPTIMAL_KHR:
                return kvkOnWindowSizeChanged();
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

        if(kvk.device != VK_NULL_HANDLE) {
            deviceWaitIdle(kvk.device);

            if(kvk.imageAvailableSemaphore != VK_NULL_HANDLE) {
                destroySemaphore(kvk.device, kvk.imageAvailableSemaphore, nullptr);
            }
            if(kvk.renderingFinishedSemaphore != VK_NULL_HANDLE) {
                destroySemaphore(kvk.device, kvk.renderingFinishedSemaphore, nullptr);
            }
            if(kvk.swapchain != VK_NULL_HANDLE) {
                destroySwapchainKHR(kvk.device, kvk.swapchain, nullptr );
            }
            destroyDevice(kvk.device, nullptr);
        }

        if(kvk.applicationSurface != VK_NULL_HANDLE) {
            destroySurfaceKHR(kvk.instance, kvk.applicationSurface, nullptr );
        }

        if(kvk.instance != VK_NULL_HANDLE) {
            destroyInstance(kvk.instance, nullptr);
        }
        glfwTerminate();

    }

    EXPORT int init(int width, int height, char *title, int fullScreen) {
        return KrautVK::kvkInit(width, height, title, fullScreen);
    }

    EXPORT int windowShouldClose() {
        return KrautVK::kvkWindowShouldClose();
    }

    EXPORT void pollEvents() {
        KrautVK::kvkPollEvents();
    }

    EXPORT void terminate() {
        KrautVK::kvkTerminate();
    }

    EXPORT void draw(){
        KrautVK::kvkRenderUpdate();
    }
}