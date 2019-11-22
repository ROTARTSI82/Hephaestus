//
// Created by 25granty on 11/18/19.
//

#include "hp/vk/window.hpp"

namespace hp::vk {
    hp::vk::window::window(int width, int height, const char *app_name, uint32_t version) {

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

        for (unsigned i = 0; i < require_ext_count; i++) {
            HP_WARN("Found required extension '{}'! Checking for support...", *(require_ext_arr + i));
            if (!ext_supported(*(require_ext_arr + i))) {
                HP_FATAL("Required extension '{}' is not supported! Aborting!", *(require_ext_arr + i));
                std::terminate();
            }
        }

        for (auto ext : req_ext) {
            HP_INFO("Found requested extension '{}'! Checking for support...", ext);
            if (!ext_supported(ext)) {
                HP_WARN("Requested extension '{}' is not supported! Skipping..", ext);
            } else {
                support_req_ext.emplace_back(ext);
            }
        }

        auto supported_names = new const char *[supported_ext.size()];  // Create new arr since it only accepts arrs of const char*

        for (unsigned long i = 0; i < supported_ext.size(); i++) {
            *(supported_names + i) = supported_ext.at(i).extensionName;

            HP_INFO("Found supported vulkan extension '{}' version {}", supported_ext.at(i).extensionName,
                    supported_ext.at(i).specVersion);
        }

        supported_lay = ::vk::enumerateInstanceLayerProperties();

        const char **avail_layers_name = new const char *[supported_lay.size()];

        std::vector<const char *> support_req_layers;

        if (uses_validation_layers) {

            for (const char *layer : req_layer) {
                HP_INFO("Validation layer {} was requested! Checking support...", layer);
                if (!layer_supported(layer)) {
                    HP_WARN("Requested validation layer '{}' is not available! Skipping!", layer);
                } else {
                    support_req_layers.emplace_back(layer);
                }
            }

            for (unsigned long i = 0; i < supported_lay.size(); i++) {
                *(avail_layers_name + i) = supported_lay.at(i).layerName;
                HP_INFO("Found supported  validation layer '{}' version {}", supported_lay.at(i).layerName,
                        supported_lay.at(i).specVersion);
            }

        } else {
            HP_INFO("Validation layers are currently disabled. Skipping it!");
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
        HP_INFO("Successfully created vulkan window!");

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
            HP_INFO("Successfully created debug messenger!");
        }

        auto vanilla_surf = (VkSurfaceKHR) surf;

        if (handle_res(::vk::Result(glfwCreateWindowSurface((VkInstance) inst, win, nullptr, &vanilla_surf)),
                       HP_GET_CODE_LOC) != ::vk::Result::eSuccess) {
            HP_FATAL("Failed to create window surface!");
        } else {
            HP_INFO("Constructed window surface");
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
            auto swap_chain = get_swap_chain_support(&device, surf);
            auto dev_qfam_indx = build_queue_fam_indices(&device, surf);

            phys_dev_ext = device.enumerateDeviceExtensionProperties();

            HP_INFO("Checking support of physical device '{}' with api {} and driver {}...", props.deviceName,
                    props.apiVersion, props.driverVersion);

            float n = req_dev_ext.size() + 4;

            float score = -1000.0f;
            if (features.geometryShader) { // Required features.
                score += 1000.0f / n;
            } else {
                HP_WARN("Device {} doesn't support geometry shaders!", props.deviceName);
            }

            if (!swap_chain.formats.empty()) {
                score += 1000.0f / n;
            } else {
                HP_FATAL("Device {} doesn't support any formats!", props.deviceName);
            }

            if (!swap_chain.present_modes.empty()) {
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
            HP_INFO("Successfully selected fully compatible physical device!");
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

        HP_INFO("Selected physical device '{}'", phys_dev->getProperties().deviceName);

        float queue_priority = 1.0f;  // We are using only a single queue so assign max priority.
        std::vector<::vk::DeviceQueueCreateInfo> queue_cis;

        // Using a set is required to make sure all indices are unique
        std::set<uint32_t> unique_queue_fams = {queue_fam_indices.graphics_fam.value(),
                                                queue_fam_indices.present_fam.value()};

        uint32_t queue_fam_indx_list[] = {queue_fam_indices.graphics_fam.value(),
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

        HP_INFO("Successfully created logical device!");

        auto swap_deets = get_swap_chain_support(phys_dev, surf);

        if (swap_deets.capabilities.currentExtent.width != UINT32_MAX) {
            swap_extent = swap_deets.capabilities.currentExtent;
        } else {
            ::vk::Extent2D actualExtent(static_cast<uint32_t>(width), static_cast<uint32_t>(height));

            actualExtent.setWidth(std::max(swap_deets.capabilities.minImageExtent.width,
                                           std::min(swap_deets.capabilities.maxImageExtent.width, actualExtent.width)));
            actualExtent.setHeight(std::max(swap_deets.capabilities.minImageExtent.height,
                                            std::min(swap_deets.capabilities.maxImageExtent.height,
                                                     actualExtent.height)));
            swap_extent = actualExtent;
        }

        ::vk::SurfaceFormatKHR surf_fmt;
        if (std::any_of(swap_deets.formats.begin(), swap_deets.formats.end(), [](::vk::SurfaceFormatKHR i) {
            return i.format == ::vk::Format::eB8G8R8A8Unorm && i.colorSpace == ::vk::ColorSpaceKHR::eSrgbNonlinear;
        })) {
            surf_fmt = ::vk::SurfaceFormatKHR({VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR});
        } else {
            surf_fmt = swap_deets.formats[0];
        }

        swap_fmt = surf_fmt.format;

        ::vk::PresentModeKHR present_mode;
        if (std::any_of(swap_deets.present_modes.begin(), swap_deets.present_modes.end(),
                        [](::vk::PresentModeKHR i) { return i == ::vk::PresentModeKHR::eMailbox; })) {
            present_mode = ::vk::PresentModeKHR::eMailbox;
        } else {
            present_mode = ::vk::PresentModeKHR::eFifo;
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
            swap_ci.imageSharingMode = ::vk::SharingMode::eConcurrent;
            swap_ci.queueFamilyIndexCount = 2;
            swap_ci.pQueueFamilyIndices = queue_fam_indx_list;
        } else {
            swap_ci.imageSharingMode = ::vk::SharingMode::eExclusive;
            swap_ci.queueFamilyIndexCount = 0; // Optional
            swap_ci.pQueueFamilyIndices = nullptr; // Optional
        }

        swap_ci.preTransform = swap_deets.capabilities.currentTransform;
        swap_ci.compositeAlpha = ::vk::CompositeAlphaFlagBitsKHR::eOpaque;
        swap_ci.presentMode = present_mode;
        swap_ci.clipped = ::vk::Bool32(VK_TRUE);

        swap_ci.oldSwapchain = ::vk::SwapchainKHR();

        swap_chain = ::vk::SwapchainKHR();
        ::vk::SwapchainCreateInfoKHR test_ci;
        if (handle_res(log_dev.createSwapchainKHR(&swap_ci, nullptr, &swap_chain), HP_GET_CODE_LOC) !=
            ::vk::Result::eSuccess) {
            HP_FATAL("Failed to create swapchain! Aborting..");
            std::terminate();
        }
        HP_INFO("Swapchain has been constructed!");

        swap_imgs = log_dev.getSwapchainImagesKHR(swap_chain);

        delete[] supported_names;
        delete[] avail_layers_name;
        delete[] dev_ext_names;

        HP_INFO("Constructed full vkInstance (use_validation_layers={})", uses_validation_layers);
    }

    hp::vk::queue_family_indices build_queue_fam_indices(::vk::PhysicalDevice *dev, ::vk::SurfaceKHR surf) {
        std::vector<::vk::QueueFamilyProperties> queue_fams = dev->getQueueFamilyProperties();
        queue_family_indices ret = {};
        for (unsigned long i = 0; i < queue_fams.size(); i++) {
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
    }

    hp::vk::window &hp::vk::window::operator=(hp::vk::window &&other) noexcept {
        if (&other == this) { // Self-assignment; do nothing
            return *this;
        }

        log_dev.destroySwapchainKHR(swap_chain, nullptr);
        log_dev.destroy();

        if (uses_validation_layers) {
            destroyDebugUtilsMessengerEXT(debug_msgr, nullptr);
        }

        inst.destroySurfaceKHR(surf, nullptr);
        inst.destroy();
        glfwDestroyWindow(win);

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
