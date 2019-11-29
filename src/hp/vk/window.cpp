//
// Created by 25granty on 11/18/19.
//

#include "hp/vk/window.hpp"
#include "boost/bind.hpp"
#include "vk_mem_alloc.h"

namespace hp::vk {
    static void on_resize_event(GLFWwindow *win, int width, int height) {
        auto app = reinterpret_cast<window *>(glfwGetWindowUserPointer(win));
        app->swapchain_recreate_event = true;
    }

    static void on_iconify_event(GLFWwindow *win, int state) {
        if (state == GLFW_TRUE) {
            auto app = reinterpret_cast<window *>(glfwGetWindowUserPointer(win));
            app->swapchain_recreate_event = true;
        }
    }

    static void bind_vbo_helper(::vk::Buffer *vbo, ::vk::CommandBuffer cmd, window *win) {
        ::vk::DeviceSize offset = 0;
        cmd.bindVertexBuffers(0, 1, vbo, &offset);
    }

    static void bind_shader_helper(::vk::Pipeline pipeline, ::vk::CommandBuffer cmd, window *win) {
        cmd.bindPipeline(::vk::PipelineBindPoint::eGraphics, pipeline);
    }

    static void draw_cmd_helper(unsigned num_verts, ::vk::CommandBuffer cmd, window *win) {
        cmd.draw(num_verts, 1, 0, 0);
    }

    static void set_viewport_helper(::vk::Viewport vp, ::vk::CommandBuffer cmd, window *win) {
        cmd.setViewport(0, 1, &vp);
    }

    static void set_scissor_helper(::vk::Rect2D sc, ::vk::CommandBuffer cmd, window *win) {
        cmd.setScissor(0, 1, &sc);
    }

    hp::vk::window::window(int width, int height, const char *app_name, uint32_t version) {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);  // Don't automatically create an OpenGL context

        // "VK_LAYER_LUNARG_api_dump",
        const std::vector<const char *> &req_layer = {"VK_LAYER_KHRONOS_validation",
                                                      "VK_LAYER_LUNARG_standard_validation"};
        const std::vector<const char *> &req_dev_ext = {VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                                                        "VK_KHR_get_memory_requirements2"};
        std::vector<const char *> req_ext = {};

        bool only_use_requested = true;

        win = glfwCreateWindow(width, height, app_name, nullptr, nullptr);
        glfwSetWindowUserPointer(win, this);
        glfwSetFramebufferSizeCallback(win, on_resize_event);
        glfwSetWindowIconifyCallback(win, on_iconify_event);

        uses_validation_layers = hp::vk::validation_layers_enabled;

        if (uses_validation_layers) {
            req_ext.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        // Query supported
        supported_ext = ::vk::enumerateInstanceExtensionProperties();
        if (!ext_supported(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
            HP_WARN("Validation layer extension is not supported! Disabling validation layers!");
            uses_validation_layers = false;
        }

        // Query required extensions
        uint32_t require_ext_count = 0;
        const char **require_ext_arr = glfwGetRequiredInstanceExtensions(&require_ext_count);

        std::vector<const char *> support_req_ext(require_ext_arr, require_ext_arr + require_ext_count);

        for (size_t i = 0; i < require_ext_count; i++) {
            HP_DEBUG("Found required extension '{}'! Checking for support...", *(require_ext_arr + i));
            if (!ext_supported(*(require_ext_arr + i))) {
                HP_FATAL("Required extension '{}' is not supported! Aborting!", *(require_ext_arr + i));
                std::terminate();
            }
        }

        for (auto ext : req_ext) {
            HP_DEBUG("Found requested extension '{}'! Checking for support...", ext);
            if (!ext_supported(ext)) {
                HP_WARN("Requested extension '{}' is not supported! Skipping..", ext);
            } else if (std::find(support_req_ext.begin(), support_req_ext.end(), ext) == support_req_ext.end()) {
                support_req_ext.emplace_back(ext);
            }
        }

        auto supported_names = new const char *[supported_ext.size()];  // Create new arr since it only accepts arrs of const char*

        for (size_t i = 0; i < supported_ext.size(); i++) {
            *(supported_names + i) = supported_ext.at(i).extensionName;

            HP_DEBUG("Found supported vulkan extension '{}' version {}", supported_ext.at(i).extensionName,
                     supported_ext.at(i).specVersion);
        }

        supported_lay = ::vk::enumerateInstanceLayerProperties();

        const char **avail_layers_name = new const char *[supported_lay.size()];

        std::vector<const char *> support_req_layers;

        if (uses_validation_layers) {

            for (const char *layer : req_layer) {
                HP_DEBUG("Validation layer {} was requested! Checking support...", layer);
                if (!layer_supported(layer)) {
                    HP_WARN("Requested validation layer '{}' is not available! Skipping!", layer);
                } else {
                    support_req_layers.emplace_back(layer);
                }
            }

            for (size_t i = 0; i < supported_lay.size(); i++) {
                *(avail_layers_name + i) = supported_lay.at(i).layerName;
                HP_DEBUG("Found supported  validation layer '{}' version {}", supported_lay.at(i).layerName,
                         supported_lay.at(i).specVersion);
            }

        } else {
            HP_DEBUG("Validation layers are currently disabled. Skipping it!");
        }

        ::vk::ApplicationInfo app_inf(app_name, version, "Hephaestus", HP_VK_VERSION_INT, VK_API_VERSION_1_1);


        ::vk::InstanceCreateInfo all_support_ci;
        if (only_use_requested) {
            all_support_ci = ::vk::InstanceCreateInfo(::vk::InstanceCreateFlags(), &app_inf,
                                                      validation_layers_enabled ? support_req_layers.size() : 0,
                                                      validation_layers_enabled ? support_req_layers.data() : nullptr,
                                                      support_req_ext.size(), support_req_ext.data());
        } else {
            all_support_ci = ::vk::InstanceCreateInfo(::vk::InstanceCreateFlags(), &app_inf,
                                                      validation_layers_enabled ? supported_lay.size() : 0,
                                                      validation_layers_enabled ? avail_layers_name : nullptr,
                                                      supported_ext.size(), supported_names);
        }

        if (handle_res(::vk::createInstance(&all_support_ci, nullptr, &inst), HP_GET_CODE_LOC) !=
            ::vk::Result::eSuccess) {
            HP_FATAL("Failed to create vulkan Instance! Aborting!");
            std::terminate();
        }
        HP_DEBUG("Successfully created vulkan window!");

        if (uses_validation_layers) {
//        ::vk::DebugUtilsMessengerCreateInfoEXT createInfo(::vk::DebugUtilsMessengerCreateFlagsEXT(),
//                static_cast<::vk::DebugUtilsMessageSeverityFlagsEXT>(VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT),
//                static_cast<::vk::DebugUtilsMessageTypeFlagsEXT>(VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT),
//                debugCallback);

            VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            createInfo.messageSeverity =
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            createInfo.messageType =
                    VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            createInfo.pfnUserCallback = debugCallback;

            createDebugUtilsMessengerEXT(&createInfo, nullptr, &debug_msgr);
            HP_DEBUG("Successfully created debug messenger!");
        }

        auto vanilla_surf = (VkSurfaceKHR) surf;

        if (handle_res(::vk::Result(glfwCreateWindowSurface((VkInstance) inst, win, nullptr, &vanilla_surf)),
                       HP_GET_CODE_LOC) != ::vk::Result::eSuccess) {
            HP_FATAL("Failed to create window surface!");
        } else {
            HP_DEBUG("Constructed window surface");
        }
        surf = ::vk::SurfaceKHR(vanilla_surf);

        devices = std::multimap<float, ::vk::PhysicalDevice>();
        std::vector<::vk::PhysicalDevice> devs = inst.enumeratePhysicalDevices();
        if (devs.empty()) {
            HP_FATAL("No physical devices available! Aborting!");
            std::terminate();
        }

        for (::vk::PhysicalDevice device : devs) {
            ::vk::PhysicalDeviceProperties props = device.getProperties();
            ::vk::PhysicalDeviceFeatures features = device.getFeatures();
            auto swap_supp = get_swap_chain_support(&device, surf);
            auto dev_qfam_indx = build_queue_fam_indices(&device, surf);

            phys_dev_ext = device.enumerateDeviceExtensionProperties();
            for (auto ext : phys_dev_ext) {
                HP_DEBUG("Device {} supports extension '{}'!", props.deviceName, ext.extensionName);
            }

            HP_DEBUG("Checking support of physical device '{}' with api {} and driver {}...", props.deviceName,
                     props.apiVersion, props.driverVersion);

            float n = req_dev_ext.size() + 4;

            float score = -1000.0f;
            if (features.geometryShader) { // Required features.
                score += 1000.0f / n;
            } else {
                HP_FATAL("Device {} doesn't support geometry shaders!", props.deviceName);
            }

            if (!swap_supp.formats.empty()) {
                score += 1000.0f / n;
            } else {
                HP_FATAL("Device {} doesn't support any formats!", props.deviceName);
            }

            if (!swap_supp.present_modes.empty()) {
                score += 1000.0f / n;
            } else {
                HP_FATAL("Device {} doesn't support any presentation modes!", props.deviceName);
            }

            if (dev_qfam_indx.is_complete()) {
                score += 1000.0f / n;
            } else {
                HP_FATAL("Device {}'s queue family isn't complete!", props.deviceName);
            }

            for (auto ext : req_dev_ext) {
                if (dev_ext_supported(ext)) {
                    score += 1000.0f / n;
                } else {
                    HP_WARN("Device {} doesn't support requeseted extension: '{}'", props.deviceName, ext);
                }
            }

            // Score device based on max texture size and device type.
            score += props.limits.maxImageDimension2D;
            if (props.deviceType == ::vk::PhysicalDeviceType::eDiscreteGpu) {
                score += 1000;
            } else {
                HP_WARN("Device {} isn't a discrete GPU!", props.deviceName);
            }

            devices.insert(std::make_pair(score, device));
        }

        if (devices.rbegin()->first > 0) { // Choose the device with the most score (if above 0)
            phys_dev = &devices.rbegin()->second;
            HP_DEBUG("Successfully selected fully compatible physical device!");
        } else {
            HP_FATAL("No suitable device could be found! Selecting the best one we have.");
            phys_dev = &devices.rbegin()->second;
        }

        queue_fam_indices = build_queue_fam_indices(phys_dev, surf);

        phys_dev->getMemoryProperties(&mem_props);
        phys_dev_ext = phys_dev->enumerateDeviceExtensionProperties();
        const char **dev_ext_names = new const char *[phys_dev_ext.size()];
        std::vector<const char *> support_req_dev_ext;

        for (auto e : req_dev_ext) {
            if (dev_ext_supported(e)) {
                support_req_dev_ext.emplace_back(e);
            } else {
                HP_WARN("The requested extension '{}' is unsupported!", e);
            }
        }

        HP_DEBUG("Selected physical device '{}'", phys_dev->getProperties().deviceName);

        float queue_priority = 1.0f;  // We are using only a single queue so assign max priority.
        std::vector<::vk::DeviceQueueCreateInfo> queue_cis;

        // Using a set is required to make sure all indices are unique
        std::set<uint32_t> unique_queue_fams = {queue_fam_indices.graphics_fam.value(),
                                                queue_fam_indices.present_fam.value()};

        for (uint32_t q_fam : unique_queue_fams) {
            ::vk::DeviceQueueCreateInfo queue_ci(::vk::DeviceQueueCreateFlags(), q_fam, 1, &queue_priority);
            queue_cis.push_back(queue_ci);
        }

        ::vk::PhysicalDeviceFeatures req_dev_features = ::vk::PhysicalDeviceFeatures();

        ::vk::DeviceCreateInfo log_dev_ci;
        if (only_use_requested) {
            log_dev_ci = ::vk::DeviceCreateInfo(::vk::DeviceCreateFlags(), queue_cis.size(), queue_cis.data(),
                                                validation_layers_enabled ? support_req_layers.size() : 0,
                                                validation_layers_enabled ? support_req_layers.data() : nullptr,
                                                support_req_dev_ext.size(),
                                                support_req_dev_ext.data(), &req_dev_features);
        } else {
            log_dev_ci = ::vk::DeviceCreateInfo(::vk::DeviceCreateFlags(), queue_cis.size(), queue_cis.data(),
                                                validation_layers_enabled ? supported_lay.size() : 0,
                                                validation_layers_enabled ? avail_layers_name : nullptr,
                                                phys_dev_ext.size(),
                                                dev_ext_names, &req_dev_features);
        }

        if (handle_res(phys_dev->createDevice(&log_dev_ci, nullptr, &log_dev), HP_GET_CODE_LOC) !=
            ::vk::Result::eSuccess) {
            HP_FATAL("Failed to create logical device! Aborting!");
            std::terminate();
        }

        log_dev.getQueue(queue_fam_indices.graphics_fam.value(), 0, &graphics_queue);

        log_dev.getQueue(queue_fam_indices.present_fam.value(), 0, &present_queue);

        HP_DEBUG("Successfully created logical device!");

        VmaVulkanFunctions vk_func_ptrs = {};
        vk_func_ptrs.vkGetBufferMemoryRequirements2KHR = (PFN_vkGetBufferMemoryRequirements2KHR) log_dev.getProcAddr(
                "vkGetBufferMemoryRequirements2KHR");

        if (vk_func_ptrs.vkGetBufferMemoryRequirements2KHR == nullptr) {
            HP_FATAL("`vkGetBufferMemoryRequirements2KHR()` cannot be loaded! :( ");
        }

        VmaAllocatorCreateInfo allocator_ci = {};
        allocator_ci.pVulkanFunctions = &vk_func_ptrs;
        allocator_ci.physicalDevice = *phys_dev;
        allocator_ci.device = log_dev;
        allocator_ci.instance = inst;
        allocator_ci.vulkanApiVersion = VK_API_VERSION_1_1;
        vmaCreateAllocator(&allocator_ci, &allocator);

        swap_chain = ::vk::SwapchainKHR();
        create_swapchain(false);

        ::vk::SemaphoreCreateInfo sm_ci((::vk::SemaphoreCreateFlags()));
        ::vk::FenceCreateInfo fence_ci(::vk::FenceCreateFlagBits::eSignaled);

        img_avail_sms.resize(max_frames_in_flight);
        rend_fin_sms.resize(max_frames_in_flight);
        flight_fences.resize(max_frames_in_flight);
        img_fences.resize(swap_imgs.size(), ::vk::Fence());

        for (size_t i = 0; i < max_frames_in_flight; i++) {
            if (handle_res(log_dev.createSemaphore(&sm_ci, nullptr, &img_avail_sms[i]), HP_GET_CODE_LOC) !=
                ::vk::Result::eSuccess ||
                handle_res(log_dev.createSemaphore(&sm_ci, nullptr, &rend_fin_sms[i]), HP_GET_CODE_LOC) !=
                ::vk::Result::eSuccess ||
                handle_res(log_dev.createFence(&fence_ci, nullptr, &flight_fences[i]), HP_GET_CODE_LOC) !=
                ::vk::Result::eSuccess) {
                HP_FATAL("Failed to create a semaphore!");
                std::terminate();
            }
        }

        delete[] supported_names;
        delete[] avail_layers_name;
        delete[] dev_ext_names;

        HP_DEBUG("Constructed full vkInstance (use_validation_layers={})", uses_validation_layers);
    }


    hp::vk::queue_family_indices build_queue_fam_indices(::vk::PhysicalDevice *dev, ::vk::SurfaceKHR surf) {
        std::vector<::vk::QueueFamilyProperties> queue_fams = dev->getQueueFamilyProperties();
        queue_family_indices ret = {};
        for (size_t i = 0; i < queue_fams.size(); i++) {
            if (queue_fams.at(i).queueFlags & ::vk::QueueFlagBits::eGraphics) {
                ret.graphics_fam = i;
            }

            if (dev->getSurfaceSupportKHR(i, surf)) {
                ret.present_fam = i;
            }

            if (ret.is_complete()) {
                break;
            }
        }

        return ret;
    }

    hp::vk::swap_chain_support get_swap_chain_support(::vk::PhysicalDevice *dev, ::vk::SurfaceKHR surf) {
        hp::vk::swap_chain_support supp{};

        supp.capabilities = dev->getSurfaceCapabilitiesKHR(surf);
        supp.formats = dev->getSurfaceFormatsKHR(surf);
        supp.present_modes = dev->getSurfacePresentModesKHR(surf);

        return supp;
    }

    hp::vk::window::~window() {
        log_dev.waitIdle(); // Wait for operations to finish

        for (size_t i = 0; i < max_frames_in_flight; i++) {
            log_dev.destroySemaphore(img_avail_sms.at(i), nullptr);
            log_dev.destroySemaphore(rend_fin_sms.at(i), nullptr);
            log_dev.destroyFence(flight_fences.at(i), nullptr);
        }

        for (auto fence : child_fences) {
            log_dev.destroyFence(fence, nullptr);
        }

        log_dev.destroyCommandPool(cmd_pool, nullptr);

        for (auto fb : framebuffers) {
            log_dev.destroyFramebuffer(fb, nullptr);
        }

        for (auto front : child_shaders) {
            delete front;
        }

        log_dev.destroyRenderPass(render_pass, nullptr);

        for (auto img : swap_views) {
            log_dev.destroyImageView(img, nullptr);
        }

        log_dev.destroySwapchainKHR(swap_chain, nullptr);

        for (auto buf : child_bufs) {
            delete buf;
        }

        vmaDestroyAllocator(allocator);

        log_dev.destroy();

        if (uses_validation_layers) {
            destroyDebugUtilsMessengerEXT(debug_msgr, nullptr);
        }

        inst.destroySurfaceKHR(surf, nullptr);
        inst.destroy();
        glfwDestroyWindow(win);
    }

    hp::vk::window::window(hp::vk::window &&other) noexcept {
        *this = std::move(other);
    }

    hp::vk::window &hp::vk::window::operator=(hp::vk::window &&other) noexcept {
        if (&other == this) { // Self-assignment; do nothing
            return *this;
        }

        this->~window();

        phys_dev = other.phys_dev;
        phys_dev_ext = std::move(other.phys_dev_ext);
        queue_fam_indices = std::move(other.queue_fam_indices);
        devices = std::move(other.devices);
        log_dev = other.log_dev;
        inst = other.inst;
        supported_ext = std::move(other.supported_ext);
        uses_validation_layers = other.uses_validation_layers;
        supported_lay = std::move(other.supported_lay);
        debug_msgr = other.debug_msgr;
        win = other.win;
        surf = other.surf;
        graphics_queue = other.graphics_queue;
        present_queue = other.present_queue;
        swap_chain = other.swap_chain;
        swap_extent = other.swap_extent;
        swap_fmt = other.swap_fmt;
        swap_imgs = std::move(other.swap_imgs);
        child_shaders = std::move(other.child_shaders);
        render_pass = other.render_pass;
        framebuffers = std::move(other.framebuffers);
        cmd_pool = other.cmd_pool;
        img_avail_sms = std::move(other.img_avail_sms);
        rend_fin_sms = std::move(other.rend_fin_sms);
        current_frame = other.current_frame;
        flight_fences = std::move(other.flight_fences);
        img_fences = std::move(other.img_fences);
        swapchain_recreate_event = other.swapchain_recreate_event;
        mem_props = other.mem_props;
        child_bufs = std::move(other.child_bufs);
        record_buffer = std::move(other.record_buffer);
        cmd_bufs = std::move(other.cmd_bufs);
        swap_recreate_callback = other.swap_recreate_callback;
        allocator = std::move(other.allocator);
        child_bufs = std::move(other.child_bufs);
        child_fences = std::move(other.child_fences);
//        render_mtx = std::move(other.render_mtx);
        return *this;
    }

    ::vk::Result hp::vk::window::createDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator,
                                                              VkDebugUtilsMessengerEXT *pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) inst.getProcAddr("vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return handle_res(::vk::Result(func(inst, pCreateInfo, pAllocator, pDebugMessenger)), HP_GET_CODE_LOC);
        } else {
            return ::vk::Result::eErrorExtensionNotPresent;
        }
    }

    ::vk::Bool32 hp::vk::window::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                               VkDebugUtilsMessageTypeFlagsEXT messageType,
                                               const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                               void *pUserData) {
        HP_FATAL("[** VULKAN ERROR **]: {}", pCallbackData->pMessage);
        return VK_FALSE;
    }

    void hp::vk::window::destroyDebugUtilsMessengerEXT(VkDebugUtilsMessengerEXT debugMessenger,
                                                       const VkAllocationCallbacks *pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) inst.getProcAddr("vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(inst, debugMessenger, pAllocator);
        }
    }

    void window::create_swapchain(bool do_destroy) {
        auto swap_deets = get_swap_chain_support(phys_dev, surf);

        ::vk::SwapchainKHR new_swap;
        ::vk::Extent2D new_extent;
        std::vector<::vk::Image> new_imgs;
        std::vector<::vk::ImageView> new_views = std::vector<::vk::ImageView>();
        ::vk::SurfaceFormatKHR new_fmt;
        ::vk::RenderPass new_pass;
        std::vector<::vk::Framebuffer> new_bufs = std::vector<::vk::Framebuffer>();
//        ::vk::CommandPool new_pool;

        if (swap_deets.capabilities.currentExtent.width != UINT32_MAX) {
            new_extent = swap_deets.capabilities.currentExtent;
        } else {
            int width, height;
            glfwGetFramebufferSize(win, &width, &height);
            ::vk::Extent2D actualExtent(static_cast<uint32_t>(width), static_cast<uint32_t>(height));

            actualExtent.setWidth(std::max(swap_deets.capabilities.minImageExtent.width,
                                           std::min(swap_deets.capabilities.maxImageExtent.width, actualExtent.width)));
            actualExtent.setHeight(std::max(swap_deets.capabilities.minImageExtent.height,
                                            std::min(swap_deets.capabilities.maxImageExtent.height,
                                                     actualExtent.height)));
            new_extent = actualExtent;
        }

        HP_DEBUG("Got surface extent of {}x{}", new_extent.width, new_extent.height);

        if (std::any_of(swap_deets.formats.begin(), swap_deets.formats.end(), [](::vk::SurfaceFormatKHR i) {
            return i.format == ::vk::Format::eB8G8R8A8Unorm && i.colorSpace == ::vk::ColorSpaceKHR::eSrgbNonlinear;
        })) {
            new_fmt = ::vk::SurfaceFormatKHR({VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR});
            HP_DEBUG(
                    "Optimal surface format is available! Selecting vk::Format::eB8G8R8A8Unorm with vk::ColorSpaceKHR::eSrgbNonlinear");
        } else {
            new_fmt = swap_deets.formats[0];
            HP_DEBUG("Optimal surface format isn't supported! Selecting first supported format...");
        }

        ::vk::PresentModeKHR present_mode;
        if (std::any_of(swap_deets.present_modes.begin(), swap_deets.present_modes.end(),
                        [](::vk::PresentModeKHR i) { return i == ::vk::PresentModeKHR::eMailbox; })) {
            present_mode = ::vk::PresentModeKHR::eMailbox;
            HP_DEBUG("Optimal presentation mode available! Selected vk::PresentModeKHR::eMailbox");
        } else {
            present_mode = ::vk::PresentModeKHR::eFifo;
            HP_DEBUG("Optimal presentation mode is NOT available! Selected vk::PresentModeKHR::eFifo");
        }

        uint32_t min_img_count = swap_deets.capabilities.minImageCount + 1;

        // Check we don't exceed max images
        if (swap_deets.capabilities.maxImageCount > 0 && min_img_count > swap_deets.capabilities.maxImageCount) {
            min_img_count = swap_deets.capabilities.maxImageCount;
        }

        ::vk::SwapchainCreateInfoKHR swap_ci(::vk::SwapchainCreateFlagsKHR(), surf, min_img_count, new_fmt.format,
                                             new_fmt.colorSpace, new_extent, 1,
                                             ::vk::ImageUsageFlagBits::eColorAttachment);

        if (queue_fam_indices.graphics_fam.value() != queue_fam_indices.present_fam.value()) {
            uint32_t queue_fam_indx_list[] = {queue_fam_indices.graphics_fam.value(),
                                              queue_fam_indices.present_fam.value()};

            swap_ci.imageSharingMode = ::vk::SharingMode::eConcurrent;
            swap_ci.queueFamilyIndexCount = 2;
            swap_ci.pQueueFamilyIndices = queue_fam_indx_list;

            HP_DEBUG("Graphics family and present family are unique! Using vk::SharingMode::eConcurrent!");
        } else {
            swap_ci.imageSharingMode = ::vk::SharingMode::eExclusive;
            swap_ci.queueFamilyIndexCount = 0; // Optional
            swap_ci.pQueueFamilyIndices = nullptr; // Optional
            HP_DEBUG("Graphics family and present family are the same! Using vk::SharingMode::eExclusive!");
        }

        swap_ci.preTransform = swap_deets.capabilities.currentTransform;
        swap_ci.compositeAlpha = ::vk::CompositeAlphaFlagBitsKHR::eOpaque;
        swap_ci.presentMode = present_mode;
        swap_ci.clipped = ::vk::Bool32(VK_TRUE);

        swap_ci.oldSwapchain = swap_chain;

        if (handle_res(log_dev.createSwapchainKHR(&swap_ci, nullptr, &new_swap), HP_GET_CODE_LOC) !=
            ::vk::Result::eSuccess) {
            HP_FATAL("Failed to create swapchain! Aborting..");
            std::terminate();
        }
        HP_DEBUG("Swapchain with minimum {} images has been constructed!", min_img_count);

        new_imgs = log_dev.getSwapchainImagesKHR(new_swap);
        HP_DEBUG("Queried {} swap chain images!", new_imgs.size());

        new_views.resize(new_imgs.size());
        for (size_t i = 0; i < new_imgs.size(); i++) {
            ::vk::ImageViewCreateInfo view_ci(::vk::ImageViewCreateFlags(), new_imgs[i], ::vk::ImageViewType::e2D,
                                              new_fmt.format,
                                              {::vk::ComponentSwizzle::eIdentity, ::vk::ComponentSwizzle::eIdentity,
                                               ::vk::ComponentSwizzle::eIdentity, ::vk::ComponentSwizzle::eIdentity},
                                              {::vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

            if (handle_res(log_dev.createImageView(&view_ci, nullptr, &new_views[i]), HP_GET_CODE_LOC) !=
                ::vk::Result::eSuccess) {
                HP_FATAL("Failed to create image view!");
            }
        }
        HP_DEBUG("Image views constructed successfully!");


        // Render pass stuff
        ::vk::AttachmentDescription color_attach(::vk::AttachmentDescriptionFlags(), new_fmt.format,
                                                 ::vk::SampleCountFlagBits::e1,
                                                 ::vk::AttachmentLoadOp::eClear, ::vk::AttachmentStoreOp::eStore,
                                                 ::vk::AttachmentLoadOp::eDontCare,
                                                 ::vk::AttachmentStoreOp::eDontCare, ::vk::ImageLayout::eUndefined,
                                                 ::vk::ImageLayout::ePresentSrcKHR);

        ::vk::AttachmentReference color_attach_ref(0, ::vk::ImageLayout::eColorAttachmentOptimal);

        ::vk::SubpassDescription subpass(::vk::SubpassDescriptionFlags(), ::vk::PipelineBindPoint::eGraphics, 0,
                                         nullptr,
                                         1, &color_attach_ref, nullptr, nullptr, 0, nullptr);

        ::vk::SubpassDependency subpass_dep(VK_SUBPASS_EXTERNAL, 0, ::vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                            ::vk::PipelineStageFlagBits::eColorAttachmentOutput, ::vk::AccessFlags(),
                                            ::vk::AccessFlagBits::eColorAttachmentRead |
                                            ::vk::AccessFlagBits::eColorAttachmentWrite, ::vk::DependencyFlags());

        ::vk::RenderPassCreateInfo rend_pass_ci(::vk::RenderPassCreateFlags(), 1, &color_attach, 1, &subpass, 1,
                                                &subpass_dep);

        if (handle_res(log_dev.createRenderPass(&rend_pass_ci, nullptr, &new_pass), HP_GET_CODE_LOC) !=
            ::vk::Result::eSuccess) {
            HP_FATAL("Failed to create render pass! Aborting!");
            std::terminate();
        }
        HP_DEBUG("Render pass constructed successfully!");

        // Framebuffers
        new_bufs.resize(new_views.size());
        for (size_t i = 0; i < new_views.size(); i++) {
            ::vk::FramebufferCreateInfo framebuf_ci(::vk::FramebufferCreateFlags(), new_pass, 1, &new_views[i],
                                                    new_extent.width, new_extent.height, 1);

            if (handle_res(log_dev.createFramebuffer(&framebuf_ci, nullptr, &new_bufs[i]), HP_GET_CODE_LOC) !=
                ::vk::Result::eSuccess) {
                HP_FATAL("Failed to create framebuffer!");
                std::terminate();
            }
        }
        HP_DEBUG("Framebuffers constructed successfully!");

        if (!do_destroy) {
            // Command pools and buffers
            ::vk::CommandPoolCreateInfo pool_ci(::vk::CommandPoolCreateFlags(), queue_fam_indices.graphics_fam.value());
            if (handle_res(log_dev.createCommandPool(&pool_ci, nullptr, &cmd_pool), HP_GET_CODE_LOC) !=
                ::vk::Result::eSuccess) {
                HP_FATAL("Failed to create command pool!");
                std::terminate();
            }
            HP_DEBUG("Command pool constructed successfully!");
        }

        std::vector<::vk::CommandBuffer> ncmd_bufs = std::vector<::vk::CommandBuffer>();
        if (do_destroy) {
            if (swap_recreate_callback != nullptr) {
                swap_recreate_callback(new_extent);
            } else {
                HP_WARN("No swapchain recreation callback is set! Command buffers may become outdated!");
            }

            ncmd_bufs = get_cmd_bufs(&new_bufs, &new_pass, &new_extent, &cmd_pool);
        }

        std::lock_guard<std::recursive_mutex> lg(render_mtx);
        log_dev.waitIdle();

        if (do_destroy) {
            for (auto buf : cmd_bufs) {
                log_dev.freeCommandBuffers(cmd_pool, 1, &buf);
            }
//            log_dev.destroyCommandPool(cmd_pool, nullptr);

            for (auto fb : framebuffers) {
                log_dev.destroyFramebuffer(fb, nullptr);
            }

            log_dev.destroyRenderPass(render_pass, nullptr);

            for (auto img : swap_views) {
                log_dev.destroyImageView(img, nullptr);
            }

            log_dev.destroySwapchainKHR(swap_chain, nullptr);
        }

        swap_extent = new_extent;
        swap_chain = new_swap;
        swap_imgs = std::move(new_imgs);
        swap_views = std::move(new_views);
        render_pass = new_pass;
        framebuffers = std::move(new_bufs);
//        cmd_pool = new_pool;
        swap_fmt = new_fmt.format;
        render_pass = new_pass;

        if (do_destroy) {
            cmd_bufs = std::move(ncmd_bufs);
        }
    }

    void window::draw_frame() {
        std::lock_guard<std::recursive_mutex> lg(render_mtx);

        log_dev.waitForFences(1, &flight_fences[current_frame], ::vk::Bool32(VK_TRUE), UINT64_MAX);

        uint32_t img_indx;
        ::vk::Result res = log_dev.acquireNextImageKHR(swap_chain, UINT64_MAX, img_avail_sms[current_frame],
                                                       ::vk::Fence(), &img_indx);
        if (res == ::vk::Result::eErrorOutOfDateKHR || res == ::vk::Result::eSuboptimalKHR) {
            HP_INFO("Recreating swapchain from image querying!");
            recreate_swapchain();
            return;
        } else if (handle_res(res, HP_GET_CODE_LOC) != ::vk::Result::eSuccess) {
            HP_FATAL("Failed to query next image! Skipping frame!");
            return;
        }

        // Check if a previous frame is using this image (i.e. there is its fence to wait on)
        if (img_fences[img_indx] != ::vk::Fence()) {
            log_dev.waitForFences(1, &img_fences[img_indx], VK_TRUE, UINT64_MAX);
        }
        // Mark the image as now being in use by this frame
        img_fences[img_indx] = flight_fences[current_frame];

        ::vk::PipelineStageFlags wait_stage = ::vk::PipelineStageFlagBits::eColorAttachmentOutput;
        ::vk::SubmitInfo cmd_buf_si(1, &img_avail_sms[current_frame], &wait_stage, 1,
                                    &cmd_bufs[img_indx], 1, &rend_fin_sms[current_frame]);

        log_dev.resetFences(1, &flight_fences[current_frame]);
        if (handle_res(graphics_queue.submit(1, &cmd_buf_si, flight_fences[current_frame]), HP_GET_CODE_LOC) !=
            ::vk::Result::eSuccess) {
            HP_FATAL("Failed to submit draw commands! Skipping frame!");
            return;
        }

        ::vk::PresentInfoKHR frame_pi(1, &rend_fin_sms[current_frame], 1, &swap_chain, &img_indx, nullptr);
        ::vk::Result pres_res = present_queue.presentKHR(&frame_pi);
        if (pres_res == ::vk::Result::eErrorOutOfDateKHR || pres_res == ::vk::Result::eSuboptimalKHR ||
            swapchain_recreate_event) {
            swapchain_recreate_event = false;
            HP_INFO("Recreating swapchain from presentation!");
            recreate_swapchain();
        } else if (handle_res(pres_res, HP_GET_CODE_LOC) != ::vk::Result::eSuccess) {
            HP_FATAL("Failed to present image! Skipping frame!");
        }

        current_frame = (current_frame + 1) % max_frames_in_flight;
    }


    std::vector<::vk::CommandBuffer> window::get_cmd_bufs(std::vector<::vk::Framebuffer> *frame_bufs,
                                                          ::vk::RenderPass *rend_pass, ::vk::Extent2D *extent,
                                                          ::vk::CommandPool *use_cmd_pool) {
        std::vector<::vk::CommandBuffer> ret(frame_bufs->size());
        ::vk::CommandBufferAllocateInfo cmd_buf_ai(*use_cmd_pool, ::vk::CommandBufferLevel::ePrimary,
                                                   ret.size());

        if (handle_res(log_dev.allocateCommandBuffers(&cmd_buf_ai, ret.data()), HP_GET_CODE_LOC) !=
            ::vk::Result::eSuccess) {
            HP_FATAL("Failed to allocated command buffers!");
            std::terminate();
        }

        for (size_t i = 0; i < ret.size(); i++) {
            ::vk::CommandBufferBeginInfo cmd_buf_bi(::vk::CommandBufferUsageFlags(), nullptr);

            if (handle_res(ret[i].begin(&cmd_buf_bi), HP_GET_CODE_LOC) != ::vk::Result::eSuccess) {
                HP_FATAL("Failed to begin command buffer recording!");
                std::terminate();
            }

            ::vk::ClearValue clear_col(::vk::ClearColorValue(std::array<float, 4>({0.0f, 0.0f, 0.0f, 1.0f})));

            ::vk::RenderPassBeginInfo rend_pass_bi(*rend_pass, (*frame_bufs)[i],
                                                   ::vk::Rect2D(::vk::Offset2D(0, 0), *extent), 1,
                                                   &clear_col);

            ret[i].beginRenderPass(&rend_pass_bi, ::vk::SubpassContents::eInline);

//            ret[i].bindPipeline(::vk::PipelineBindPoint::eGraphics, pipeline);
//            ret[i].setViewport(0, 1, &viewport);
//            ret[i].setScissor(0, 1, &scissor);

            for (const auto &fn : record_buffer) {
                fn(ret[i], this);
            }

            ret[i].endRenderPass();

#ifdef VULKAN_HPP_DISABLE_ENHANCED_MODE
            if (handle_res(ret[i].end(), HP_GET_CODE_LOC) != ::vk::Result::eSuccess) {
                HP_FATAL("Failed to end command buffer recording!");
                std::terminate();
            }
#else
            ret[i].end();  // Enhanced mode does exception handling for us. :)
#endif
        }
        return ret;
    }

    void window::recreate_swapchain() {
        int width = 0, height = 0;
        glfwGetFramebufferSize(win, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(win, &width, &height);
            glfwWaitEvents();
        }

        create_swapchain(true);
    }

    void window::clear_recording() {
        record_buffer.clear();
    }

    void window::save_recording() {
//        ::vk::CommandPool new_cmd_pool;
//
//        ::vk::CommandPoolCreateInfo pool_ci(::vk::CommandPoolCreateFlags(), queue_fam_indices.graphics_fam.value());
//        if (handle_res(log_dev.createCommandPool(&pool_ci, nullptr, &new_cmd_pool), HP_GET_CODE_LOC) !=
//            ::vk::Result::eSuccess) {
//            HP_FATAL("Failed to create command pool!");
//            std::terminate();
//        }

        auto new_cmd_buf = get_cmd_bufs(&framebuffers, &render_pass, &swap_extent, &cmd_pool);

        std::lock_guard<std::recursive_mutex> lg(render_mtx);

        log_dev.waitIdle();
        for (auto buf : cmd_bufs) {
            log_dev.freeCommandBuffers(cmd_pool, 1, &buf);
        }
//        log_dev.destroyCommandPool(cmd_pool, nullptr);
//        cmd_pool = new_cmd_pool;
        cmd_bufs = std::move(new_cmd_buf);
    }

    void window::rec_bind_vbo(vertex_buffer *vbo) {
        record_buffer.emplace_back(boost::bind(bind_vbo_helper, &vbo->buf, _1, _2));
    }

    void window::rec_draw(unsigned num_verts) {
        record_buffer.emplace_back(boost::bind(draw_cmd_helper, num_verts, _1, _2));
    }

    void window::rec_bind_shader(shader_program *shader) {
        record_buffer.emplace_back(boost::bind(bind_shader_helper, shader->pipeline, _1, _2));
    }

    void window::rec_set_viewport(::vk::Viewport viewport) {
        record_buffer.emplace_back(boost::bind(set_viewport_helper, viewport, _1, _2));
    }

    void window::rec_set_scissor(::vk::Rect2D scissor) {
        record_buffer.emplace_back(boost::bind(set_scissor_helper, scissor, _1, _2));
    }

    void window::rec_set_default_viewport() {
        ::vk::Viewport viewport(0.0f, 0.0f, (float) swap_extent.width, (float) swap_extent.height, 0.0f,
                                1.0f);
        record_buffer.emplace_back(boost::bind(set_viewport_helper, viewport, _1, _2));
    }

    void window::rec_set_default_scissor() {
        ::vk::Rect2D scissor(::vk::Offset2D(0, 0), swap_extent);
        record_buffer.emplace_back(boost::bind(set_scissor_helper, scissor, _1, _2));
    }

    std::pair<::vk::Fence *, ::vk::CommandBuffer> window::copy_buffer(generic_buffer *source, generic_buffer *dest,
                                                                      bool wait, size_t src_offset, size_t dest_offset,
                                                                      size_t size) {
        if (source->capacity != dest->capacity && size == 0) {
            HP_FATAL("copy_buffer() called with auto size and mismatched source & dest buffer sizes!");
            HP_FATAL("Source buffer was {} bytes, but dest buffer was {} bytes!", source->capacity, dest->capacity);
            return {nullptr, ::vk::CommandBuffer()};
        }
        ::vk::CommandBufferAllocateInfo cmd_ai(cmd_pool, ::vk::CommandBufferLevel::ePrimary, 1);

        ::vk::CommandBuffer cmd_buf;
        log_dev.allocateCommandBuffers(&cmd_ai, &cmd_buf);

        ::vk::CommandBufferBeginInfo cmd_bi(::vk::CommandBufferUsageFlagBits::eOneTimeSubmit, nullptr);
        cmd_buf.begin(&cmd_bi);

        ::vk::BufferCopy cpy_region(src_offset, dest_offset, size == 0 ? source->capacity : size);
        cmd_buf.copyBuffer(source->buf, dest->buf, 1, &cpy_region);
        cmd_buf.end();

        ::vk::SubmitInfo submit_inf(0, nullptr, nullptr, 1, &cmd_buf, 0, nullptr);

        ::vk::Fence *ret = new_fence();

        ::vk::FenceCreateInfo fence_ci((::vk::FenceCreateFlags()));
        if (handle_res(log_dev.createFence(&fence_ci, nullptr, ret), HP_GET_CODE_LOC) != ::vk::Result::eSuccess) {
            HP_FATAL("Failed to create fences!");
        }

        graphics_queue.submit(1, &submit_inf, *ret);

        if (wait) {
            log_dev.waitForFences(1, ret, ::vk::Bool32(VK_TRUE), UINT64_MAX);
            delete_fence(ret);
            log_dev.freeCommandBuffers(cmd_pool, 1, &cmd_buf);
            return {nullptr, ::vk::CommandBuffer()};;
        } else {
            return {ret, cmd_buf};
        }
    }

    hp::vk::__hp_vk_is_in_layer_prop_list::__hp_vk_is_in_layer_prop_list(const char *lay) : lay(lay) {}

    hp::vk::__hp_vk_is_in_extension_prop_list::__hp_vk_is_in_extension_prop_list(const char *ext) : ext(ext) {}

    bool hp::vk::queue_family_indices::is_complete() {
        return graphics_fam.has_value() && present_fam.has_value();
    }

    hp::vk::queue_family_indices::queue_family_indices(const hp::vk::queue_family_indices &rhs) {
        *this = rhs;
    }

    hp::vk::queue_family_indices &hp::vk::queue_family_indices::operator=(const hp::vk::queue_family_indices &rhs) {
        if (this == &rhs) {
            return *this;
        }
        graphics_fam = rhs.graphics_fam;
        present_fam = rhs.present_fam;

        return *this;
    }

    hp::vk::queue_family_indices::queue_family_indices(hp::vk::queue_family_indices &&rhs) noexcept {
        *this = std::move(rhs);
    }

    hp::vk::queue_family_indices &hp::vk::queue_family_indices::operator=(hp::vk::queue_family_indices &&rhs) noexcept {
        if (this == &rhs) {
            return *this;
        }
        graphics_fam = rhs.graphics_fam;
        present_fam = rhs.present_fam;
        return *this;
    }
}
