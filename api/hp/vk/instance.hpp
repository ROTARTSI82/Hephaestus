//
// Created by 25granty on 11/18/19.
//

#pragma once

#ifndef __HEPHAESTUS_VK_INSTANCE_HPP
#define __HEPHAESTUS_VK_INSTANCE_HPP

#include "hp/config.hpp"
#include "hp/logging.hpp"
#include "hp/vk/vk.hpp"
#include "hp/hp.hpp"

#include <GLFW/glfw3.h>
#include <map>

namespace hp::vk {
    /*
     * Can only be used after `glfwInit` since it relies on some GLFW calls.
     */

    struct __hp_vk_is_in_required_extensions {
        const char *name;

        explicit __hp_vk_is_in_required_extensions(const char *name);

        inline bool operator()(const char *other) const;
    };

    struct __hp_vk_is_in_extension_prop_list {
        const char *ext;

        explicit __hp_vk_is_in_extension_prop_list(const char *ext);

        inline bool operator()(::vk::ExtensionProperties other) const;
    };

    struct __hp_vk_is_in_layer_prop_list {
        const char *lay;

        explicit __hp_vk_is_in_layer_prop_list(const char *lay);

        inline bool operator()(::vk::LayerProperties other) const;
    };

    struct queue_family {
    public:
        queue_family() = default;

        std::optional<uint32_t> graphics_fam;

        inline bool is_complete();

        queue_family(const queue_family &rhs);

        queue_family &operator=(const queue_family &rhs);
    };

    class instance {
    private:
        bool uses_validation_layers{};
        ::vk::Instance inst;
        VkDebugUtilsMessengerEXT debug_msgr{};
        std::vector<::vk::ExtensionProperties> supported_ext;
        std::vector<::vk::LayerProperties> supported_lay;

        ::vk::PhysicalDevice *default_dev;
        std::multimap<int, ::vk::PhysicalDevice> devices;

        queue_family queue_fam;

        static VKAPI_ATTR ::vk::Bool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                                VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                                const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                                void *pUserData);

    public:
        instance() = default;

        instance(const char *app_name, uint32_t version, const std::vector<const char *> &req_ext = {},
                 const std::vector<const char *> &req_layer = {"VK_LAYER_KHRONOS_validation"});

        virtual ~instance();

        instance(const instance &other) = delete;

        instance &operator=(const instance &rhs) = delete;

        instance(instance &&other) noexcept;

        instance &operator=(instance &&other) noexcept;

        ::vk::Result createDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                                  const VkAllocationCallbacks *pAllocator,
                                                  VkDebugUtilsMessengerEXT *pDebugMessenger);

        void
        destroyDebugUtilsMessengerEXT(VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator);

        inline bool ext_supported(const char *ext);

        inline bool layer_supported(const char *lay);
    };
}


#endif //__HEPHAESTUS_VK_INSTANCE_HPP
