//
// Created by 25granty on 11/18/19.
//

#include "hp/vk/window.hpp"

hp::vk::window::window(int width, int height, const char *app_name, uint32_t version,
                       const std::vector<const char *> &req_ext,
                       const std::vector<const char *> &req_layer) {

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);  // Don't automatically create an OpenGL context
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);  // Don't allow resizing for now: Resizing requires special vulkan code.

    win = glfwCreateWindow(width, height, app_name, nullptr, nullptr);

    uses_validation_layers = hp::vk::validation_layers_enabled;

    std::vector<const char *> req_ext_cpy(req_ext);
    if (uses_validation_layers) {
        req_ext_cpy.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    for (auto ext : req_ext_cpy) {
        HP_INFO("Found requested extension '{}'! Checking for support...", ext);
    }

    ::vk::ApplicationInfo app_inf(app_name, version, "Hephaestus", HP_VK_VERSION_INT, VK_API_VERSION_1_1);

    // Query required extensions
    uint32_t require_ext_count = 0;
    const char **require_ext_arr = glfwGetRequiredInstanceExtensions(&require_ext_count);

    for (unsigned i = 0; i < require_ext_count; i++) {
        HP_WARN("Found required extension '{}'! Checking for support...", *(require_ext_arr + i));
    }

    // Query supported
    supported_ext = ::vk::enumerateInstanceExtensionProperties();
    if (!ext_supported(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
        HP_WARN("Validation layer extension is not supported! Disabling validation layers!");
        uses_validation_layers = false;
    }

    auto supported_names = new const char *[supported_ext.size()];  // Create new arr since it only accepts arrs of const char*
    std::vector<const char *> supported_requested;  // Create a new vec to include required extensions.

    unsigned number_found_required = 0;
    for (unsigned long i = 0; i < supported_ext.size(); i++) {
        const char *current_name = supported_ext.at(i).extensionName;
        *(supported_names + i) = current_name;

        auto comparator = __hp_vk_is_in_required_extensions(current_name);

        if (std::any_of(require_ext_arr, require_ext_arr + require_ext_count, comparator)) {
            HP_WARN("Found supported required extension '{}' version {}", current_name,
                    supported_ext.at(i).specVersion);
            supported_requested.emplace_back(current_name);
            number_found_required++;
        } else if (std::any_of(req_ext_cpy.begin(), req_ext_cpy.end(), comparator)) {
            HP_INFO("Found supported requested extension '{}' version {}", current_name,
                    supported_ext.at(i).specVersion);
            supported_requested.emplace_back(current_name);
        } else {
            HP_INFO("Found supported optional extension '{}' version {}", current_name,
                    supported_ext.at(i).specVersion);
        }
    }

    if (number_found_required != require_ext_count) {
        HP_FATAL("Not all required extensions are supported! {}/{} Required Extensions available.",
                 number_found_required, require_ext_count);
        std::terminate();
    } else {
        HP_INFO("All required extensions are supported!");
    }

    HP_INFO("Out of {} requested extensions {} were supported. (Count includes other required extensions that might not have been requested)",
            req_ext_cpy.size(), supported_requested.size());

    supported_lay = ::vk::enumerateInstanceLayerProperties();

    std::vector<const char *> supported_req_layers;
    const char **avail_layers_name = new const char *[supported_lay.size()];

    if (uses_validation_layers) {

        for (const char *layer : req_layer) {
            HP_INFO("Validation layer {} was requested! Checking support...", layer);
        }

        for (unsigned long i = 0; i < supported_lay.size(); i++) {
            *(avail_layers_name + i) = supported_lay.at(i).layerName;
            if (std::any_of(req_layer.begin(), req_layer.end(),
                            __hp_vk_is_in_required_extensions(supported_lay.at(i).layerName))) {
                HP_INFO("Found supported requested validation layer '{}' version {}", supported_lay.at(i).layerName,
                        supported_lay.at(i).specVersion);
                supported_req_layers.emplace_back(supported_lay.at(i).layerName);
            } else {
                HP_INFO("Found supported optional validation layer '{}' version {}", supported_lay.at(i).layerName,
                        supported_lay.at(i).specVersion);
            }
        }

        HP_INFO("Out of {} validation layers requested {} were supported.", req_layer.size(),
                supported_req_layers.size());
    } else {
        HP_INFO("Validation layers are currently disabled. Skipping it!");
    }

    ::vk::InstanceCreateInfo all_support_ci(::vk::InstanceCreateFlags(), &app_inf,
                                            validation_layers_enabled ? supported_lay.size() : 0,
                                            validation_layers_enabled ? avail_layers_name : nullptr,
                                            supported_ext.size(), supported_names);
    support_mode = 0;
    if (handle_res(::vk::createInstance(&all_support_ci, nullptr, &inst), HP_GET_CODE_LOC) != ::vk::Result::eSuccess) {
        support_mode = 1;
        HP_FATAL(
                "Failed to create vulkan window with full support! Attempting window creation with only requested features!");
        inst = ::vk::Instance();  // Clear window, might not be necesse

        ::vk::InstanceCreateInfo all_supp_request_ci(::vk::InstanceCreateFlags(), &app_inf,
                                                     validation_layers_enabled ? supported_req_layers.size() : 0,
                                                     validation_layers_enabled ? supported_req_layers.data() : nullptr,
                                                     supported_requested.size(), supported_requested.data());
        if (handle_res(::vk::createInstance(&all_supp_request_ci, nullptr, &inst), HP_GET_CODE_LOC) !=
            ::vk::Result::eSuccess) {
            support_mode = 2;
            HP_FATAL(
                    "Failed to create vulkan window with requested support! Attempting only required features! (Validation layers will be completely disabled)!");
            inst = ::vk::Instance();
            uses_validation_layers = false;

            ::vk::InstanceCreateInfo only_require_ci(::vk::InstanceCreateFlags(), &app_inf, 0, nullptr,
                                                     require_ext_count, require_ext_arr);
            if (handle_res(::vk::createInstance(&only_require_ci, nullptr, &inst), HP_GET_CODE_LOC) !=
                ::vk::Result::eSuccess) {
                HP_FATAL("Failed to create vulkan window with required support! Aborting!");
                std::terminate();
            }
        }
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

    devices = std::multimap<int, ::vk::PhysicalDevice>();
    std::vector<::vk::PhysicalDevice> devs = inst.enumeratePhysicalDevices();
    if (devs.empty()) {
        HP_FATAL("No physical devices available! Aborting!");
        std::terminate();
    }

    for (::vk::PhysicalDevice device : devs) {
        ::vk::PhysicalDeviceProperties props = device.getProperties();
        ::vk::PhysicalDeviceFeatures features = device.getFeatures();

        int score = -1000;
        if (features.geometryShader) { // Required features.
            score += 1000;
        }
        // Score device based on max texture size and device type.
        score += props.limits.maxImageDimension2D;
        if (props.deviceType == ::vk::PhysicalDeviceType::eDiscreteGpu) {
            score += 1000;
        }

        devices.insert(std::make_pair(score, device));
    }

    if (devices.rbegin()->first > 0) { // Choose the device with the most score (if above 0)
        phys_dev = &devices.rbegin()->second;
        HP_INFO("Successfully selected physical device.");
    } else {
        HP_FATAL("No suitable device could be found! Selecting the best one we have.");
        phys_dev = &devices.rbegin()->second;
    }

    std::vector<::vk::QueueFamilyProperties> queue_fams = phys_dev->getQueueFamilyProperties();
    for (unsigned long i = 0; i < queue_fams.size(); i++) {
        if (queue_fams.at(i).queueFlags & ::vk::QueueFlagBits::eGraphics) {
            queue_fam_indices.graphics_fam = i;
        }

        if (phys_dev->getSurfaceSupportKHR(i, surf)) {
            queue_fam_indices.present_fam = i;
        }

        if (queue_fam_indices.is_complete()) {
            break;
        }
    }

    if (!queue_fam_indices.is_complete()) {
        HP_FATAL("Queue family indicies cannot be constructed!");
    } else {
        HP_INFO("Constructed Queue family indicies!");
    }

    float queue_priority = 1.0f;  // We are using only a single queue so assign max priority.
    std::vector<::vk::DeviceQueueCreateInfo> queue_cis;
    std::set<uint32_t> unique_queue_fams = {queue_fam_indices.graphics_fam.value(),
                                            queue_fam_indices.present_fam.value()};
    for (uint32_t q_fam : unique_queue_fams) {
        ::vk::DeviceQueueCreateInfo queue_ci(::vk::DeviceQueueCreateFlags(), q_fam, 1, &queue_priority);
        queue_cis.push_back(queue_ci);
    }

    ::vk::PhysicalDeviceFeatures req_dev_features;

    // TODO: In the future, implement extension requesting system for devices..

    ::vk::DeviceCreateInfo log_dev_ci(::vk::DeviceCreateFlags(), queue_cis.size(), queue_cis.data(), 0, nullptr, 0,
                                      nullptr, &req_dev_features);
    if (support_mode == 0 && uses_validation_layers) {
        log_dev_ci.enabledLayerCount = supported_lay.size();
        log_dev_ci.ppEnabledLayerNames = avail_layers_name;
    } else if (support_mode == 1 && uses_validation_layers) {
        log_dev_ci.enabledLayerCount = supported_req_layers.size();
        log_dev_ci.ppEnabledLayerNames = supported_req_layers.data();
    } // `support_mode == 2` disables validation layers, which is already what it's set to

    if (handle_res(phys_dev->createDevice(&log_dev_ci, nullptr, &log_dev), HP_GET_CODE_LOC) != ::vk::Result::eSuccess) {
        HP_FATAL("Failed to create logical device! Aborting!");
        std::terminate();
    }
    HP_INFO("Successfully created logical device!");

    log_dev.getQueue(queue_fam_indices.graphics_fam.value(), 0, &graphics_queue);
    HP_INFO("Successfully queried graphics queue of logical device");

    log_dev.getQueue(queue_fam_indices.present_fam.value(), 0, &present_queue);
    HP_INFO("Successfully queried present queue of logical divice");

    delete[] supported_names;
    delete[] avail_layers_name;

    HP_INFO("Constructed full vkInstance (use_validation_layers={})", uses_validation_layers);
}

hp::vk::window::~window() {
    log_dev.destroy();

    if (uses_validation_layers) {
        destroyDebugUtilsMessengerEXT(debug_msgr, nullptr);
    }

    inst.destroySurfaceKHR(surf, nullptr);
    inst.destroy();
//    vkDestroyInstance(inst, nullptr);
    glfwDestroyWindow(win);
}

hp::vk::window::window(hp::vk::window &&other) noexcept {
    phys_dev = other.phys_dev;
    support_mode = other.support_mode;
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
}

hp::vk::window &hp::vk::window::operator=(hp::vk::window &&other) noexcept {
    if (&other == this) { // Self-assignment; do nothing
        return *this;
    }

    log_dev.destroy();

    if (uses_validation_layers) {
        destroyDebugUtilsMessengerEXT(debug_msgr, nullptr);
    }
    inst.destroySurfaceKHR(surf, nullptr);
    inst.destroy();

    glfwDestroyWindow(win);
//    vkDestroyInstance(inst, nullptr);

    phys_dev = other.phys_dev;
    support_mode = other.support_mode;
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

    return *this;
}

//inline bool hp::vk::window::ext_supported(const char *ext) {
//    return std::any_of(supported_ext.begin(), supported_ext.end(), __hp_vk_is_in_extension_prop_list(ext));
//}

//inline bool hp::vk::window::layer_supported(const char *lay) {
//    return std::any_of(supported_lay.begin(), supported_lay.end(), __hp_vk_is_in_layer_prop_list(lay));
//}

//inline bool hp::vk::window::should_close() {
//    return glfwWindowShouldClose(win);
//}

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

//inline bool hp::vk::__hp_vk_is_in_required_extensions::operator()(const char *other) const {
//    return strcmp(name, other) == 0;
//}

hp::vk::__hp_vk_is_in_required_extensions::__hp_vk_is_in_required_extensions(const char *name) : name(name) {}

hp::vk::__hp_vk_is_in_layer_prop_list::__hp_vk_is_in_layer_prop_list(const char *lay) : lay(lay) {}

//inline bool hp::vk::__hp_vk_is_in_layer_prop_list::operator()(::vk::LayerProperties other) const {
//    return strcmp(lay, other.layerName) == 0;
//}

hp::vk::__hp_vk_is_in_extension_prop_list::__hp_vk_is_in_extension_prop_list(const char *ext) : ext(ext) {}

//inline bool hp::vk::__hp_vk_is_in_extension_prop_list::operator()(::vk::ExtensionProperties other) const {
//    return strcmp(other.extensionName, ext) == 0;
//}

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
