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

    GLFWwindow *KrautVK::window;
    GLFWmonitor *KrautVK::monitor;
    VkInstance KrautVK::instance;
    VkDevice KrautVK::device;
    uint32_t KrautVK::graphicsQueueFamilyIndex;
    uint32_t KrautVK::presentationQueueFamilyIndex;
    VkQueue KrautVK::GraphicsCommandBuffer;
    VkQueue KrautVK::PresentationCommandBuffer;
    VkSurfaceKHR KrautVK::applicationSurface;

    int KrautVK::kvkInitGLFW(int width, int height, char *title, int fullScreen) {

        if (!glfwInit())
            return INIT_FAILED;

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, false);

        monitor = glfwGetPrimaryMonitor();

        if (fullScreen)
            window = glfwCreateWindow(width, height, title, monitor, nullptr);
        else
            window = glfwCreateWindow(width, height, title, nullptr, nullptr);


        if (!window) {
            return WINDOW_CREATION_FAILED;
        }

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

            if(glfwGetPhysicalDevicePresentationSupport(instance, physicalDevice, i) && currentPresentationQueueFamilyIndex == UINT32_MAX){
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

        printf("Loading global level procedures\n");
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

        if (createInstance(&instanceCreateInfo, nullptr, &instance) != SUCCESS)
            return VULKAN_INSTANCE_CREATION_FAILED;

        printf("Loading instance level procedures\n");
        createDevice = (PFN_vkCreateDevice) glfwGetInstanceProcAddress(instance, "vkCreateDevice");
        enumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices) glfwGetInstanceProcAddress(instance, "vkEnumeratePhysicalDevices");
        getPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties) glfwGetInstanceProcAddress(instance, "vkGetPhysicalDeviceProperties");
        getPhysicalDeviceFeatures = (PFN_vkGetPhysicalDeviceFeatures) glfwGetInstanceProcAddress(instance, "vkGetPhysicalDeviceFeatures");
        getPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties) glfwGetInstanceProcAddress(instance, "vkGetPhysicalDeviceQueueFamilyProperties");
        destroyInstance = (PFN_vkDestroyInstance) glfwGetInstanceProcAddress(instance, "vkDestroyInstance");
        destroySurfaceKHR = (PFN_vkDestroySurfaceKHR) glfwGetInstanceProcAddress(instance, "vkDestroySurfaceKHR");
        enumerateDeviceExtensionProperties = (PFN_vkEnumerateDeviceExtensionProperties) glfwGetInstanceProcAddress(instance, "vkEnumerateDeviceExtensionProperties");


    }

    int KrautVK::kvkCreateDevice() {

        //INITIALIZE PHYSICAL DEVICES
        printf("Enumerating physical devices\n");
        uint32_t deviceCount;
        if (enumeratePhysicalDevices(instance, &deviceCount, nullptr) != SUCCESS || deviceCount == 0)
            return VULKAN_NOT_SUPPORTED;

        std::vector <VkPhysicalDevice> devices(deviceCount);
        if (enumeratePhysicalDevices(instance, &deviceCount, &devices[0]) != SUCCESS)
            return VULKAN_NOT_SUPPORTED;

        VkPhysicalDevice selectedDevice = nullptr;
        uint32_t selectedGraphicsQueueFamilyIndex = UINT32_MAX;
        uint32_t selectedPresentationQueueFamilyIndex = UINT32_MAX;

        for (uint32_t i = 0; i < deviceCount; i++) {
            if (kvkCheckDeviceProperties(devices[i], selectedGraphicsQueueFamilyIndex, selectedPresentationQueueFamilyIndex)) {
                selectedDevice = devices[i];
                break;
            }
        }

        if (selectedDevice == nullptr)
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

        if(createDevice(selectedDevice, &deviceCreateInfo, nullptr, &device) != SUCCESS)
            return VULKAN_DEVICE_CREATION_FAILED;

        printf("Loading device level procedures\n");
        getDeviceProcAddr = (PFN_vkGetDeviceProcAddr) glfwGetInstanceProcAddress(instance, "vkGetDeviceProcAddr");
        getDeviceQueue = (PFN_vkGetDeviceQueue) getDeviceProcAddr(device, "vkGetDeviceQueue");
        deviceWaitIdle = (PFN_vkDeviceWaitIdle) getDeviceProcAddr(device, "vkDeviceWaitIdle");
        destroyDevice = (PFN_vkDestroyDevice) getDeviceProcAddr(device, "vkDestroyDevice");

        //INITIALIZE COMMAND BUFFER
        printf("Initializing command buffers\n");
        graphicsQueueFamilyIndex = selectedGraphicsQueueFamilyIndex;
        presentationQueueFamilyIndex = selectedPresentationQueueFamilyIndex;

        getDeviceQueue(device, graphicsQueueFamilyIndex, 0, &GraphicsCommandBuffer);
        getDeviceQueue(device, presentationQueueFamilyIndex, 0, &PresentationCommandBuffer);
    }

    int KrautVK::kvkInitVulkan(const char *title) {

        kvkCreateInstance(title);
        kvkCreateDevice();

        if(glfwCreateWindowSurface(instance, window, nullptr, &applicationSurface))
            return VULKAN_SURFACE_CREATION_FAILED;


        return SUCCESS;
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

        printf("KrautVK Alpha Initialized\n");
        return SUCCESS;
    }

    int KrautVK::kvkWindowShouldClose() {
        return glfwWindowShouldClose(window);

    }

    void KrautVK::kvkPollEvents() {
        glfwPollEvents();
    }

    void KrautVK::kvkTerminate() {
        if(device != nullptr) {
            deviceWaitIdle(device);
            destroyDevice(device, nullptr);
        }

        if(applicationSurface != nullptr) {
            destroySurfaceKHR(instance, applicationSurface, nullptr);
        }

        if(instance != nullptr) {
            destroyInstance(instance, nullptr);
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
}