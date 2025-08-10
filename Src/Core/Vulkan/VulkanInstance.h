#pragma once
#include <vector>
#include <vulkan/vulkan.h>
#include "VulkanCore.h"

class VulkanInstance
{
public:
    VulkanInstance(
        const std::string& AppName,
        const glm::uvec2& AppVersion,
        const std::vector<const char*>& Extensions,
        DebugCallback DebugCallback = nullptr
        );

    ~VulkanInstance();

    // Forbid copy construct
    VulkanInstance(const VulkanInstance&) = delete;
    VulkanInstance& operator=(const VulkanInstance&) = delete;

    vk::Instance getInstance() const { return VkInstance; }

    void EnableValidation(bool bEnable) { bEnableValidation = bEnable; }
    bool IsValidationEnabled() const { return bEnableValidation; }
private:
    void CreateDebugMessenger();

    vk::Instance VkInstance;
    
    std::string AppName;
    glm::uvec2 AppVersion;
    DebugCallback DebugCallback;
    bool bEnableValidation = true;
};