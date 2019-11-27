//
// Created by 25granty on 11/18/19.
//

#pragma once

#ifndef __HEPHAESTUS_VK_INSTANCE_HPP
#define __HEPHAESTUS_VK_INSTANCE_HPP

#include "hp/config.hpp"
#include "hp/vk/vk.hpp"
#include "hp/hp.hpp"

#include <map>
#include <set>
#include <queue>

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

    struct swap_chain_support {
        ::vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<::vk::SurfaceFormatKHR> formats;
        std::vector<::vk::PresentModeKHR> present_modes;
    };

    static queue_family_indices build_queue_fam_indices(::vk::PhysicalDevice *dev, ::vk::SurfaceKHR surf);

    static swap_chain_support get_swap_chain_support(::vk::PhysicalDevice *dev, ::vk::SurfaceKHR surf);


    class window;

    class shader_program {
    private:
        ::hp::vk::window *parent{};
        std::vector<::vk::PipelineShaderStageCreateInfo> stage_cis;
        // The entrypoint string's `c_str` is passed to the create info, and when the pipeline needs to be rebuilt,
        // The string has gone out of scope, and the pointer points to unallocated memory. This prevents that.
        std::queue<const char *> entrypoint_keepalives;
        std::vector<::vk::CommandBuffer> cmd_bufs;

        ::vk::PipelineLayout pipeline_layout;
        ::vk::Pipeline pipeline;
        std::queue<::vk::ShaderModule> mods;

        std::string fp;
        const char *metapath{};

        friend class ::hp::vk::window;

        shader_program(std::string basicString, const char *string, ::hp::vk::window *pWindow);

    public:
        virtual ~shader_program();

        shader_program() = default;

        shader_program &operator=(const shader_program &rhs) = delete;

        shader_program(const shader_program &rhs) = delete;

        shader_program &operator=(shader_program &&rhs) noexcept;

        shader_program(shader_program &&rhs) noexcept;

        void reload_from_file();

        void rebuild_pipeline();
    };

    static void on_resize_event(GLFWwindow *win, int width, int height);

    static void on_iconify_event(GLFWwindow *win, int state);

    class window {
    private:
        GLFWwindow *win{};
        ::vk::SurfaceKHR surf;
        size_t current_frame = 0;

        bool uses_validation_layers{};
        ::vk::Instance inst;
        VkDebugUtilsMessengerEXT debug_msgr{};
        std::vector<::vk::ExtensionProperties> supported_ext;
        std::vector<::vk::LayerProperties> supported_lay;
        std::vector<::vk::ExtensionProperties> phys_dev_ext;

        ::vk::PhysicalDevice *phys_dev{};
        ::vk::Device log_dev;
        std::multimap<float, ::vk::PhysicalDevice> devices;

        ::vk::Queue graphics_queue;
        ::vk::Queue present_queue;

        ::vk::SwapchainKHR swap_chain;
        ::vk::Extent2D swap_extent;
        ::vk::Format swap_fmt{};
        std::vector<::vk::Image> swap_imgs;
        std::vector<::vk::ImageView> swap_views;
        std::vector<::vk::Framebuffer> framebuffers;

        ::vk::CommandPool cmd_pool;
        ::vk::RenderPass render_pass;

        queue_family_indices queue_fam_indices;

        std::vector<::hp::vk::shader_program *> child_shaders;
        ::hp::vk::shader_program *current_shader{};

        std::vector<::vk::Semaphore> img_avail_sms;
        std::vector<::vk::Semaphore> rend_fin_sms;
        std::vector<::vk::Fence> flight_fences;
        std::vector<::vk::Fence> img_fences;

        bool swapchain_recreate_event = false;

        static VKAPI_ATTR ::vk::Bool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                                VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                                const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                                void *pUserData);

        void create_swapchain();

        void recreate_swapchain();

        friend class shader_program;

        friend void on_resize_event(GLFWwindow *win, int width, int height);

        friend void on_iconify_event(GLFWwindow *win, int state);

    public:
        window() = default;

        window(int width, int height, const char *app_name, uint32_t version);

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

        inline bool dev_ext_supported(const char *ext) {
            return std::any_of(phys_dev_ext.begin(), phys_dev_ext.end(), __hp_vk_is_in_extension_prop_list(ext));
        };

        inline bool layer_supported(const char *lay) {
            return std::any_of(supported_lay.begin(), supported_lay.end(), __hp_vk_is_in_layer_prop_list(lay));
        };

        inline bool should_close() {
            return glfwWindowShouldClose(win);
        };

        /*
         * DO NOT attempt to call `delete` on pointer returned by this function! They are cleaned up when window is destroyed!
         */
        inline shader_program *new_shader_program(const std::string &fp, const char *metapath = "/shader_metadat.txt") {
            auto new_prog = new shader_program(fp, metapath, this);
            child_shaders.emplace_back(new_prog);
            return new_prog;
        };

        void draw_frame();

        shader_program *bind_shader_program(shader_program *rhs);
    };
}


#endif //__HEPHAESTUS_VK_INSTANCE_HPP
