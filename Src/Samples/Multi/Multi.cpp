#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#include <iostream>
#include <vector>
#include <stdexcept>

// 错误检查宏
#define VK_CHECK_RESULT(f) \
{ \
    VkResult res = (f); \
    if (res != VK_SUCCESS) \
    { \
        std::cerr << "Vulkan error: " << res << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        abort(); \
    } \
}

// 全局变量
VkInstance instance = VK_NULL_HANDLE;
VkSurfaceKHR surface = VK_NULL_HANDLE;
GLFWwindow* window = nullptr;

// 调试回调函数
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    std::cerr << "Vulkan Validation: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

// 初始化窗口
void initWindow()
{
    // 初始化GLFW
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW!");
    }
    
    // 设置GLFW不使用OpenGL
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    
    // 创建窗口
    window = glfwCreateWindow(800, 600, "Vulkan Validation", nullptr, nullptr);
    if (!window) {
        throw std::runtime_error("Failed to create GLFW window!");
    }
}

// 创建Vulkan实例
void createInstance()
{
    // 应用信息
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan Validation";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    // 获取GLFW所需的扩展
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    
    // 需要启用的扩展列表
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); // 添加调试扩展

    // 验证层（如果可用）
    std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    // 创建实例信息
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    
    // 检查验证层是否可用
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    
    bool layersAvailable = true;
    for (const char* layerName : validationLayers) {
        bool layerFound = false;
        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound) {
            layersAvailable = false;
            std::cout << "Validation layer '" << layerName << "' not available." << std::endl;
            break;
        }
    }
    
    if (layersAvailable) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
        std::cout << "Validation layers not available, skipping..." << std::endl;
    }

    // 创建Vulkan实例
    VK_CHECK_RESULT(vkCreateInstance(&createInfo, nullptr, &instance));
    std::cout << "Vulkan instance created successfully!" << std::endl;
}

// 创建显示表面
void createSurface()
{
    VK_CHECK_RESULT(glfwCreateWindowSurface(instance, window, nullptr, &surface));
    std::cout << "Window surface created successfully!" << std::endl;
}

// 验证VMA初始化
void validateVma(VkInstance instance)
{
    VmaAllocator allocator = VK_NULL_HANDLE;
    
    // 1. 选择物理设备
    uint32_t gpuCount = 0;
    vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr);
    if (gpuCount == 0) {
        std::cerr << "No Vulkan-capable GPUs found!" << std::endl;
        return;
    }
    
    std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
    vkEnumeratePhysicalDevices(instance, &gpuCount, physicalDevices.data());
    VkPhysicalDevice physicalDevice = physicalDevices[0]; // 使用第一个GPU
    
    // 2. 创建逻辑设备
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = 0; // 使用第一个队列族
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    
    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.queueCreateInfoCount = 1;
    
    VkDevice device;
    if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS) {
        std::cerr << "Failed to create logical device!" << std::endl;
        return;
    }
    
    // 3. 创建分配器
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.device = device;
    allocatorInfo.instance = instance;
    
    VkResult result = vmaCreateAllocator(&allocatorInfo, &allocator);
    if (result == VK_SUCCESS) {
        std::cout << "VMA initialized successfully!" << std::endl;
        
        // 销毁分配器
        vmaDestroyAllocator(allocator);
    } else {
        std::cerr << "Failed to initialize VMA: " << result << std::endl;
    }
    
    // 4. 销毁逻辑设备
    vkDestroyDevice(device, nullptr);
}

// 清理资源
void cleanup()
{
    if (instance) {
        if (surface) {
            vkDestroySurfaceKHR(instance, surface, nullptr);
        }
        vkDestroyInstance(instance, nullptr);
    }
    
    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

int main()
{
    try {
        std::cout << "====== Vulkan Validation Program ======" << std::endl;
        std::cout << "1. Initializing GLFW..." << std::endl;
        initWindow();
        
        std::cout << "2. Creating Vulkan instance..." << std::endl;
        createInstance();
        
        std::cout << "3. Creating window surface..." << std::endl;
        createSurface();
        
        std::cout << "4. Validating VMA initialization..." << std::endl;
        validateVma(instance); // 传入Vulkan实例
        
        std::cout << "\nAll components initialized successfully!" << std::endl;
        std::cout << "Vulkan API, GLFW and VMA are properly configured." << std::endl;
        
        // 主循环
        std::cout << "\nPress ESC to exit the program..." << std::endl;
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            
            // 检查ESC键
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                break;
            }
        }
        
        std::cout << "\nCleaning up resources..." << std::endl;
        cleanup();
        std::cout << "Program exited successfully!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        cleanup();
        return -1;
    }
}