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
//#include <GLFW/glfw3native.h>
#include <map>
#include <set>

namespace hp::vk {
    /*
     * Can only be used after `glfwInit` since it relies on some GLFW calls.
     */

    struct __hp_vk_is_in_required_extensions {
        const char *name;

        explicit __hp_vk_is_in_required_extensions(const char *name);

        inline bool operator()(const char *other) const {
            return strcmp(name, other) == 0;
        };
    };

    struct __hp_vk_is_in_extension_prop_list {
        const char *ext;

        explicit __hp_vk_is_in_extension_prop_list(const char *ext);

        inline bool operator()(::vk::ExtensionProperties other) const {
            return strcmp(other.extensionName, ext) == 0;
        };
    };

    struct __hp_vk_is_in_layer_prop_list {
        const char *lay;

        explicit __hp_vk_is_in_layer_prop_list(const char *lay);

        inline bool operator()(::vk::LayerProperties other) const {
            return strcmp(lay, other.layerName) == 0;
        };
    };

    struct queue_family_indices {
    public:
        queue_family_indices() = default;

        std::optional<uint32_t> graphics_fam;

        std::optional<uint32_t> present_fam;

        inline bool is_complete();

        queue_family_indices(const queue_family_indices &rhs);

        queue_family_indices &operator=(const queue_family_indices &rhs);

        queue_family_indices(queue_family_indices &&rhs) noexcept;

        queue_family_indices &operator=(queue_family_indices &&rhs) noexcept;
    };

    class window {
    private:
        GLFWwindow *win{};
        ::vk::SurfaceKHR surf;

        bool uses_validation_layers{};
        ::vk::Instance inst;
        VkDebugUtilsMessengerEXT debug_msgr{};
        std::vector<::vk::ExtensionProperties> supported_ext;
        std::vector<::vk::LayerProperties> supported_lay;

        ::vk::PhysicalDevice *phys_dev{};
        ::vk::Device log_dev;
        std::multimap<int, ::vk::PhysicalDevice> devices;

        ::vk::Queue graphics_queue;
        ::vk::Queue present_queue;

        queue_family_indices queue_fam_indices;
        char support_mode{};  // Support mode 0 = All support, 1 = Requested support, 2 = minimal support.

        static VKAPI_ATTR ::vk::Bool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                                VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                                const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                                void *pUserData);

    public:
        window() = default;

        window(int width, int height, const char *app_name, uint32_t version,
               const std::vector<const char *> &req_ext = {},
               const std::vector<const char *> &req_layer = {"VK_LAYER_KHRONOS_validation"});

        virtual ~window();

        window(const window &other) = delete;

        window &operator=(const window &rhs) = delete;

        window(window &&other) noexcept;

        window &operator=(window &&other) noexcept;

        ::vk::Result createDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                                  const VkAllocationCallbacks *pAllocator,
                                                  VkDebugUtilsMessengerEXT *pDebugMessenger);

        void
        destroyDebugUtilsMessengerEXT(VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator);

        inline bool ext_supported(const char *ext) {
            return std::any_of(supported_ext.begin(), supported_ext.end(), __hp_vk_is_in_extension_prop_list(ext));
        };

        inline bool layer_supported(const char *lay) {
            return std::any_of(supported_lay.begin(), supported_lay.end(), __hp_vk_is_in_layer_prop_list(lay));
        };

        inline bool should_close() {
            return glfwWindowShouldClose(win);
        };
    };
}


#endif //__HEPHAESTUS_VK_INSTANCE_HPP
