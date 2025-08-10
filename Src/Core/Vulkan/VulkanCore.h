#pragma once
#include <functional>
#include <vulkan/vulkan.hpp>
#include <glm.hpp>

class VulkanInstance;

// Debug Callback
using DebugCallback = std::function<void(
    vk::DebugUtilsMessageSeverityFlagBitsEXT Severity,
    vk::DebugUtilsMessageTypeFlagsEXT Type,
    const std::string& Message
    )>;