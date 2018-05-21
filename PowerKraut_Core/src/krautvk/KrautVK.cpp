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
    uint32_t KrautVK::queueFamilyIndex;
    VkQueue KrautVK::commandBuffer;
    VkSurfaceKHR KrautVK::applicationSurface;

    int KrautVK::initGLFW(int width, int height, char *title, int fullScreen) {

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

    //This is where we set up our command buffers and queue families and select our device.
    //When expanding backend functionality, start here.
    int KrautVK::checkDeviceProperties(VkPhysicalDevice physicalDevice, uint32_t &selectedFamilyIndex) {
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
        for (uint32_t i = 0; i < qFamilyCount; ++i) {
            if ((qFamilyProperties[i].queueCount > 0) &&
                (qFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
                selectedFamilyIndex = i;
                std::cout << "Selected Device: " << deviceProperties.deviceName << std::endl;
                return true;
            }
        }

        return false;
    }

    int KrautVK::initVulkan(const char *title) {

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



        //INITIALIZE PHYSICAL DEVICES
        printf("Enumerating physical devices\n");
        uint32_t deviceCount;
        if (enumeratePhysicalDevices(instance, &deviceCount, nullptr) != SUCCESS || deviceCount == 0)
            return VULKAN_NOT_SUPPORTED;

        std::vector <VkPhysicalDevice> devices(deviceCount);
        if (enumeratePhysicalDevices(instance, &deviceCount, &devices[0]) != SUCCESS)
            return VULKAN_NOT_SUPPORTED;

        VkPhysicalDevice selectedDevice = nullptr;
        uint32_t selectedFamilyIndex = UINT32_MAX;
        for (uint32_t i = 0; i < deviceCount; i++) {
            if (checkDeviceProperties(devices[i], selectedFamilyIndex)) {
                selectedDevice = devices[i];
                break;
            }
        }

        if (selectedDevice == nullptr)
            return VULKAN_NOT_SUPPORTED;

        std::vector<float> queuePriorities = {1.0f};

        VkDeviceQueueCreateInfo queueCreateInfo = {
                VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,     // VkStructureType              sType
                nullptr,                                        // const void                  *pNext
                0,                                              // VkDeviceQueueCreateFlags     flags
                selectedFamilyIndex,                            // uint32_t                     queueFamilyIndex
                static_cast<uint32_t>(queuePriorities.size()),  // uint32_t                     queueCount
                &queuePriorities[0]                             // const float                 *pQueuePriorities
        };

        VkDeviceCreateInfo deviceCreateInfo = {
                VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,           // VkStructureType                    sType
                nullptr,                                        // const void                        *pNext
                0,                                              // VkDeviceCreateFlags                flags
                1,                                              // uint32_t                           queueCreateInfoCount
                &queueCreateInfo,                               // const VkDeviceQueueCreateInfo     *pQueueCreateInfos
                0,                                              // uint32_t                           enabledLayerCount
                nullptr,                                        // const char * const                *ppEnabledLayerNames
                0,                                              // uint32_t                           enabledExtensionCount
                nullptr,                                        // const char * const                *ppEnabledExtensionNames
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
        printf("Initializing command buffer\n");
        queueFamilyIndex = selectedFamilyIndex;
        getDeviceQueue(device, queueFamilyIndex, 0, &commandBuffer);

        if(glfwCreateWindowSurface(instance, window, nullptr, &applicationSurface))
            return VULKAN_SURFACE_CREATION_FAILED;


        return SUCCESS;
    }

    int KrautVK::init(int width, int height, char *title, int fullScreen) {
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
        int status = initGLFW(width, height, title, fullScreen);

        if (status != SUCCESS)
            return status;

        status = initVulkan(title);

        if (status != SUCCESS)
            return status;

        printf("KrautVK Alpha Initialized\n");
        return SUCCESS;
    }

    int KrautVK::windowShouldClose() {
        return glfwWindowShouldClose(window);

    }

    void KrautVK::pollEvents() {
        glfwPollEvents();
    }

    void KrautVK::terminate() {
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
        return KrautVK::init(width, height, title, fullScreen);
    }

    EXPORT int windowShouldClose() {
        return KrautVK::windowShouldClose();
    }

    EXPORT void pollEvents() {
        KrautVK::pollEvents();
    }

    EXPORT void terminate() {
        KrautVK::terminate();
    }
}