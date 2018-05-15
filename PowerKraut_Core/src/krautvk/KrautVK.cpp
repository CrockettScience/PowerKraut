#include "KrautVK.hpp"

namespace KrautVK {

    GLFWwindow* KrautVK::window;
    GLFWmonitor* KrautVK::monitor;

    int KrautVK::initGLFW(int width, int height, char *title, int fullScreen) {

        if (!glfwInit())
            return INIT_FAILED;

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
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

    int KrautVK::initVulkan(const char *title) {

        if (!glfwVulkanSupported())
            return VULKAN_NOT_SUPPORTED;

        printf("Loading Vulkan Instance\n");
        //initialize function pointers and create Vulkan instance
        createInstance = (PFN_vkCreateInstance) glfwGetInstanceProcAddress(nullptr, "vkCreateInstance");

        VkApplicationInfo applicationInfo = {
                VK_STRUCTURE_TYPE_APPLICATION_INFO,             // VkStructureType            sType
                NULL,                                           // const void                *pNext
                title,                                          // const char                *pApplicationName
                VK_MAKE_VERSION(1, 0, 0),                       // uint32_t                   applicationVersion
                "KrautVK",                                      // const char                *pEngineName
                version,                                        // uint32_t                   engineVersion
                VK_API_VERSION_1_1                              // uint32_t                   apiVersion
        };

        VkInstanceCreateInfo instanceCreateInfo = {
                VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,         // VkStructureType            sType
                NULL,                                           // const void*                pNext
                0,                                              // VkInstanceCreateFlags      flags
                &applicationInfo,                              // const VkApplicationInfo   *pApplicationInfo
                0,                                              // uint32_t                   enabledLayerCount
                NULL,                                           // const char * const        *ppEnabledLayerNames
                0,                                              // uint32_t                   enabledExtensionCount
                NULL                                            // const char * const        *ppEnabledExtensionNames
        };

        return SUCCESS;
    }

    int KrautVK::init(int width, int height, char *title, int fullScreen) {
        printf("KrautVK Alpha\n");

        printf("Initializing GLFW\n");
        int status = initGLFW(width, height, title, fullScreen);

        if (status != SUCCESS)
            return status;
        printf("Initialized Window\n");

        status = initVulkan(title);

        if (status != SUCCESS)
            return status;
        printf("Initialized Vulkan\n");


        printf("KrautVK Alpha Initialized");
        return SUCCESS;
    }

    int KrautVK::windowShouldClose() {
        return glfwWindowShouldClose(window);

    }

    void KrautVK::pollEvents() {
        glfwPollEvents();
    }

    void KrautVK::terminate() {
            glfwTerminate();
        }

    EXPORT int init(int width, int height, char *title, int fullScreen){
        return KrautVK::init(width, height, title, fullScreen);
    }

    EXPORT int windowShouldClose(){
        return KrautVK::windowShouldClose();
    }

    EXPORT void pollEvents(){
        KrautVK::pollEvents();
    }

    EXPORT void terminate(){
        KrautVK::terminate();
    }
}