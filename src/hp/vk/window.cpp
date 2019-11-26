//
// Created by 25granty on 11/18/19.
//

#include "hp/vk/window.hpp"

namespace hp::vk {
    hp::vk::window::window(int width, int height, const char *app_name, uint32_t version) {
        cmd_bufs_recorded = false;
        current_shader = nullptr;  // Avoid hours of debugging undefined behavior.
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);  // Don't automatically create an OpenGL context
        glfwWindowHint(GLFW_RESIZABLE,
                       GLFW_FALSE);  // Don't allow resizing for now: Resizing requires special vulkan code.

        // "VK_LAYER_LUNARG_api_dump",
        const std::vector<const char *> &req_layer = {"VK_LAYER_KHRONOS_validation",
                                                      "VK_LAYER_LUNARG_standard_validation"};
        const std::vector<const char *> &req_dev_ext = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        std::vector<const char *> req_ext = {};

        bool only_use_requested = true;

        win = glfwCreateWindow(width, height, app_name, nullptr, nullptr);

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

            HP_DEBUG("Checking support of physical device '{}' with include {} and driver {}...", props.deviceName,
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

        swap_chain = ::vk::SwapchainKHR();
        create_swapchain();

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
        log_dev.destroyCommandPool(cmd_pool, nullptr);

        for (auto fb : framebuffers) {
            log_dev.destroyFramebuffer(fb, nullptr);
        }

        while (!child_shaders.empty()) {
            auto front = child_shaders.front();
            delete front;
            child_shaders.pop();
        }

        log_dev.destroyRenderPass(render_pass, nullptr);

        for (auto img : swap_views) {
            log_dev.destroyImageView(img, nullptr);
        }

        log_dev.destroySwapchainKHR(swap_chain, nullptr);
        log_dev.destroy();

        if (uses_validation_layers) {
            destroyDebugUtilsMessengerEXT(debug_msgr, nullptr);
        }

        inst.destroySurfaceKHR(surf, nullptr);
        inst.destroy();
        glfwDestroyWindow(win);
    }

    hp::vk::window::window(hp::vk::window &&other) noexcept {
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
        current_shader = other.current_shader;
        framebuffers = std::move(other.framebuffers);
        cmd_pool = other.cmd_pool;
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
        current_shader = other.current_shader;
        framebuffers = std::move(other.framebuffers);
        cmd_pool = other.cmd_pool;

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

    void window::create_swapchain() {
        auto swap_deets = get_swap_chain_support(phys_dev, surf);

        if (swap_deets.capabilities.currentExtent.width != UINT32_MAX) {
            swap_extent = swap_deets.capabilities.currentExtent;
        } else {
            int width, height;
            glfwGetFramebufferSize(win, &width, &height);
            ::vk::Extent2D actualExtent(static_cast<uint32_t>(width), static_cast<uint32_t>(height));

            actualExtent.setWidth(std::max(swap_deets.capabilities.minImageExtent.width,
                                           std::min(swap_deets.capabilities.maxImageExtent.width, actualExtent.width)));
            actualExtent.setHeight(std::max(swap_deets.capabilities.minImageExtent.height,
                                            std::min(swap_deets.capabilities.maxImageExtent.height,
                                                     actualExtent.height)));
            swap_extent = actualExtent;
        }

        HP_DEBUG("Got surface extent of {}x{}", swap_extent.width, swap_extent.height);

        ::vk::SurfaceFormatKHR surf_fmt;
        if (std::any_of(swap_deets.formats.begin(), swap_deets.formats.end(), [](::vk::SurfaceFormatKHR i) {
            return i.format == ::vk::Format::eB8G8R8A8Unorm && i.colorSpace == ::vk::ColorSpaceKHR::eSrgbNonlinear;
        })) {
            surf_fmt = ::vk::SurfaceFormatKHR({VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR});
            HP_DEBUG(
                    "Optimal surface format is available! Selecting vk::Format::eB8G8R8A8Unorm with vk::ColorSpaceKHR::eSrgbNonlinear");
        } else {
            surf_fmt = swap_deets.formats[0];
            HP_DEBUG("Optimal surface format isn't supported! Selecting first supported format...");
        }

        swap_fmt = surf_fmt.format;

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

        ::vk::SwapchainCreateInfoKHR swap_ci(::vk::SwapchainCreateFlagsKHR(), surf, min_img_count, surf_fmt.format,
                                             surf_fmt.colorSpace, swap_extent, 1,
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

        ::vk::SwapchainCreateInfoKHR test_ci;
        if (handle_res(log_dev.createSwapchainKHR(&swap_ci, nullptr, &swap_chain), HP_GET_CODE_LOC) !=
            ::vk::Result::eSuccess) {
            HP_FATAL("Failed to create swapchain! Aborting..");
            std::terminate();
        }
        HP_DEBUG("Swapchain with minimum {} images has been constructed!", min_img_count);

        swap_imgs = log_dev.getSwapchainImagesKHR(swap_chain);
        HP_DEBUG("Queried {} swap chain images!", swap_imgs.size());

        swap_views.resize(swap_imgs.size());
        for (size_t i = 0; i < swap_imgs.size(); i++) {
            ::vk::ImageViewCreateInfo view_ci(::vk::ImageViewCreateFlags(), swap_imgs[i], ::vk::ImageViewType::e2D,
                                              swap_fmt,
                                              {::vk::ComponentSwizzle::eIdentity, ::vk::ComponentSwizzle::eIdentity,
                                               ::vk::ComponentSwizzle::eIdentity, ::vk::ComponentSwizzle::eIdentity},
                                              {::vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

            if (handle_res(log_dev.createImageView(&view_ci, nullptr, &swap_views[i]), HP_GET_CODE_LOC) !=
                ::vk::Result::eSuccess) {
                HP_FATAL("Failed to create image view!");
            }
        }
        HP_DEBUG("Image views constructed successfully!");


        // Render pass stuff
        ::vk::AttachmentDescription color_attach(::vk::AttachmentDescriptionFlags(), swap_fmt,
                                                 ::vk::SampleCountFlagBits::e1,
                                                 ::vk::AttachmentLoadOp::eClear, ::vk::AttachmentStoreOp::eStore,
                                                 ::vk::AttachmentLoadOp::eDontCare,
                                                 ::vk::AttachmentStoreOp::eDontCare, ::vk::ImageLayout::eUndefined,
                                                 ::vk::ImageLayout::ePresentSrcKHR);

        ::vk::AttachmentReference color_attach_ref(0, ::vk::ImageLayout::eColorAttachmentOptimal);

        ::vk::SubpassDescription subpass(::vk::SubpassDescriptionFlags(), ::vk::PipelineBindPoint::eGraphics, 0,
                                         nullptr,
                                         1, &color_attach_ref, nullptr, nullptr, 0, nullptr);

        ::vk::RenderPassCreateInfo rend_pass_ci(::vk::RenderPassCreateFlags(), 1, &color_attach, 1, &subpass, 0,
                                                nullptr);

        if (handle_res(log_dev.createRenderPass(&rend_pass_ci, nullptr, &render_pass), HP_GET_CODE_LOC) !=
            ::vk::Result::eSuccess) {
            HP_FATAL("Failed to create render pass! Aborting!");
            std::terminate();
        }
        HP_DEBUG("Render pass constructed successfully!");

        // Framebuffers
        framebuffers.resize(swap_views.size());
        for (size_t i = 0; i < swap_views.size(); i++) {
            ::vk::FramebufferCreateInfo framebuf_ci(::vk::FramebufferCreateFlags(), render_pass, 1, &swap_views[i],
                                                    swap_extent.width, swap_extent.height, 1);

            if (handle_res(log_dev.createFramebuffer(&framebuf_ci, nullptr, &framebuffers[i]), HP_GET_CODE_LOC) !=
                ::vk::Result::eSuccess) {
                HP_FATAL("Failed to create framebuffer!");
                std::terminate();
            }
        }
        HP_DEBUG("Framebuffers constructed successfully!");

        // Command pools and buffers
        ::vk::CommandPoolCreateInfo pool_ci(::vk::CommandPoolCreateFlags(), queue_fam_indices.graphics_fam.value());
        if (handle_res(log_dev.createCommandPool(&pool_ci, nullptr, &cmd_pool), HP_GET_CODE_LOC) !=
            ::vk::Result::eSuccess) {
            HP_FATAL("Failed to create command pool!");
            std::terminate();
        }
        HP_DEBUG("Command pool constructed successfully!");

        cmd_bufs.resize(framebuffers.size());
        ::vk::CommandBufferAllocateInfo cmd_buf_ai(cmd_pool, ::vk::CommandBufferLevel::ePrimary, cmd_bufs.size());

        if (handle_res(log_dev.allocateCommandBuffers(&cmd_buf_ai, cmd_bufs.data()), HP_GET_CODE_LOC) !=
            ::vk::Result::eSuccess) {
            HP_FATAL("Failed to allocated command buffers!");
            std::terminate();
        }
    }

    shader_program *window::bind_shader_program(shader_program *rhs) {
        if (rhs->pipeline == ::vk::Pipeline()) {
            HP_WARN("Tried to bind an incomplete shader program '{}'! Ignoring call to hp::vk::window::bind_shader_program()!",
                    rhs->fp);
            return nullptr;
        }

        cmd_bufs_recorded = true;

        for (size_t i = 0; i < cmd_bufs.size(); i++) {
            ::vk::CommandBufferBeginInfo cmd_buf_bi(::vk::CommandBufferUsageFlags(), nullptr);

            if (handle_res(cmd_bufs[i].begin(&cmd_buf_bi), HP_GET_CODE_LOC) != ::vk::Result::eSuccess) {
                HP_FATAL("Failed to begin command buffer recording!");
                std::terminate();
            }

            ::vk::ClearValue clear_col(::vk::ClearColorValue(std::array<float, 4>({0.0f, 0.0f, 0.0f, 1.0f})));

            ::vk::RenderPassBeginInfo rend_pass_bi(render_pass, framebuffers[i],
                                                   ::vk::Rect2D(::vk::Offset2D(0, 0), swap_extent), 1, &clear_col);

            cmd_bufs[i].beginRenderPass(&rend_pass_bi, ::vk::SubpassContents::eInline);
            cmd_bufs[i].bindPipeline(::vk::PipelineBindPoint::eGraphics, rhs->pipeline);
            cmd_bufs[i].draw(3, 1, 0, 0);
            cmd_bufs[i].endRenderPass();

#ifdef VULKAN_HPP_DISABLE_ENHANCED_MODE
            if (handle_res(cmd_bufs[i].end(), HP_GET_CODE_LOC) != ::vk::Result::eSuccess) {
                HP_FATAL("Failed to end command buffer recording!");
                std::terminate();
            }
#else
            cmd_bufs[i].end();  // Enhanced mode does exception handling for us. :)
#endif
        }

        return current_shader = rhs;
    }

    hp::vk::__hp_vk_is_in_required_extensions::__hp_vk_is_in_required_extensions(const char *name) : name(name) {}

    hp::vk::__hp_vk_is_in_layer_prop_list::__hp_vk_is_in_layer_prop_list(const char *lay) : lay(lay) {}

    hp::vk::__hp_vk_is_in_extension_prop_list::__hp_vk_is_in_extension_prop_list(const char *ext) : ext(ext) {}

    bool hp::vk::queue_family_indices::is_complete() {
        return graphics_fam.has_value() && present_fam.has_value();
    }

    hp::vk::queue_family_indices::queue_family_indices(const hp::vk::queue_family_indices &rhs) {
        graphics_fam = rhs.graphics_fam;
        present_fam = rhs.present_fam;
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
        graphics_fam = rhs.graphics_fam;
        present_fam = rhs.present_fam;
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
