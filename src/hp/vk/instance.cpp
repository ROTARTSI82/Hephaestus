//
// Created by 25granty on 11/18/19.
//

#include <vulkan/vulkan.h>
#include "hp/vk/instance.hpp"

hp::vk::instance::instance(const char *app_name, uint32_t version, const std::vector<const char *> &req_ext,
                           const std::vector<const char *> &req_layer) {

    std::vector<const char *> req_ext_cpy(req_ext);
    if (hp::vk::validation_layers_enabled) {
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

    if (hp::vk::validation_layers_enabled) {

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


    uses_validation_layers = hp::vk::validation_layers_enabled;
    ::vk::InstanceCreateInfo all_support_ci(::vk::InstanceCreateFlags(), &app_inf,
                                            validation_layers_enabled ? supported_lay.size() : 0,
                                            validation_layers_enabled ? avail_layers_name : nullptr,
                                            supported_ext.size(), supported_names);
    if (handle_res(::vk::createInstance(&all_support_ci, nullptr, &inst), HP_GET_CODE_LOC) != ::vk::Result::eSuccess) {
        HP_FATAL(
                "Failed to create vulkan instance with full support! Attempting instance creation with only requested features!");
        inst = ::vk::Instance();  // Clear instance, might not be necesse

        ::vk::InstanceCreateInfo all_supp_request_ci(::vk::InstanceCreateFlags(), &app_inf,
                                                     validation_layers_enabled ? supported_req_layers.size() : 0,
                                                     validation_layers_enabled ? supported_req_layers.data() : nullptr,
                                                     supported_requested.size(), supported_requested.data());
        if (handle_res(::vk::createInstance(&all_supp_request_ci, nullptr, &inst), HP_GET_CODE_LOC) !=
            ::vk::Result::eSuccess) {
            HP_FATAL(
                    "Failed to create vulkan instance with requested support! Attempting only required features! (Validation layers will be completely disabled)!");
            inst = ::vk::Instance();
            uses_validation_layers = false;

            ::vk::InstanceCreateInfo only_require_ci(::vk::InstanceCreateFlags(), &app_inf, 0, nullptr,
                                                     require_ext_count, require_ext_arr);
            if (handle_res(::vk::createInstance(&only_require_ci, nullptr, &inst), HP_GET_CODE_LOC) !=
                ::vk::Result::eSuccess) {
                HP_FATAL("Failed to create vulkan instance with required support! Aborting!");
                std::terminate();
            }
        }
    }

    debug_msgr = {};
    if (uses_validation_layers) {

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
    }

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
        default_dev = &devices.rbegin()->second;
        HP_INFO("Successfully selected physical device.");
    } else {
        HP_FATAL("No suitable device could be found! Selecting the best one we have.");
        default_dev = &devices.rbegin()->second;
    }

    std::vector<::vk::QueueFamilyProperties> queue_fams = default_dev->getQueueFamilyProperties();
    for (unsigned long i = 0; i < queue_fams.size(); i++) {
        if (queue_fams.at(i).queueFlags & ::vk::QueueFlagBits::eGraphics) {
            queue_fam.graphics_fam = i;
        }

        if (queue_fam.is_complete()) {
            break;
        }
    }

    if (!queue_fam.is_complete()) {
        HP_FATAL("Queue family indicies cannot be constructed!");
    } else {
        HP_INFO("Constructed Queue family indicies!");
    }

    HP_INFO("Successfully created vulkan instance!");

    delete[] supported_names;
}

hp::vk::instance::~instance() {
    if (uses_validation_layers) {
        destroyDebugUtilsMessengerEXT(debug_msgr, nullptr);
    }

    vkDestroyInstance(inst, nullptr);
}

hp::vk::instance::instance(hp::vk::instance &&other) noexcept {
    inst = other.inst;
    supported_ext = std::move(other.supported_ext);
    uses_validation_layers = other.uses_validation_layers;
    supported_lay = std::move(other.supported_lay);
    debug_msgr = other.debug_msgr;
}

hp::vk::instance &hp::vk::instance::operator=(hp::vk::instance &&other) noexcept {
    if (&other == this) { // Self-assignment; do nothing
        return *this;
    }

    if (uses_validation_layers) {
        destroyDebugUtilsMessengerEXT(debug_msgr, nullptr);
    }

    vkDestroyInstance(inst, nullptr);

    inst = other.inst;
    supported_ext = std::move(other.supported_ext);
    uses_validation_layers = other.uses_validation_layers;
    supported_lay = std::move(other.supported_lay);
    debug_msgr = other.debug_msgr;

    return *this;
}

bool hp::vk::instance::ext_supported(const char *ext) {
    return std::any_of(supported_ext.begin(), supported_ext.end(), __hp_vk_is_in_extension_prop_list(ext));
}

bool hp::vk::instance::layer_supported(const char *lay) {
    return std::any_of(supported_lay.begin(), supported_lay.end(), __hp_vk_is_in_layer_prop_list(lay));
}

::vk::Result hp::vk::instance::createDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                                            const VkAllocationCallbacks *pAllocator,
                                                            VkDebugUtilsMessengerEXT *pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(inst, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return static_cast<::vk::Result>(func(inst, pCreateInfo, pAllocator, pDebugMessenger));
    } else {
        return ::vk::Result::eErrorExtensionNotPresent;
    }
}

::vk::Bool32 hp::vk::instance::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                             VkDebugUtilsMessageTypeFlagsEXT messageType,
                                             const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                             void *pUserData) {
    HP_FATAL("[** VULKAN ERROR **]: {}", pCallbackData->pMessage);
    return VK_FALSE;
}

void hp::vk::instance::destroyDebugUtilsMessengerEXT(VkDebugUtilsMessengerEXT debugMessenger,
                                                     const VkAllocationCallbacks *pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(inst, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(inst, debugMessenger, pAllocator);
    }
}

inline bool hp::vk::__hp_vk_is_in_required_extensions::operator()(const char *other) const {
    return strcmp(name, other) == 0;
}

hp::vk::__hp_vk_is_in_required_extensions::__hp_vk_is_in_required_extensions(const char *name) : name(name) {}

hp::vk::__hp_vk_is_in_layer_prop_list::__hp_vk_is_in_layer_prop_list(const char *lay) : lay(lay) {}

inline bool hp::vk::__hp_vk_is_in_layer_prop_list::operator()(::vk::LayerProperties other) const {
    return strcmp(lay, other.layerName) == 0;
}

hp::vk::__hp_vk_is_in_extension_prop_list::__hp_vk_is_in_extension_prop_list(const char *ext) : ext(ext) {}

inline bool hp::vk::__hp_vk_is_in_extension_prop_list::operator()(::vk::ExtensionProperties other) const {
    return strcmp(other.extensionName, ext) == 0;
}

bool hp::vk::queue_family::is_complete() {
    return graphics_fam.has_value();
}

hp::vk::queue_family::queue_family(const hp::vk::queue_family &rhs) {
    this->graphics_fam = rhs.graphics_fam;
}

hp::vk::queue_family &hp::vk::queue_family::operator=(const hp::vk::queue_family &rhs) {
    if (this == &rhs) {
        return *this;
    }
    this->graphics_fam = rhs.graphics_fam;

    return *this;
}
