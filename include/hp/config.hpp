/**
 * @file config.hpp
 * @brief File containing configuration for Hephaestus.
 * @details Modify to your liking, but any modifications require full rebuilds.
 */

#pragma once

#ifndef __HEPHAESTUS_CONFIG_H
/**
 * @def __HEPHAESTUS_CONFIG_H
 * @brief This macro is defined if `config.hpp` has been included.
 */
#define __HEPHAESTUS_CONFIG_H

#undef NDEBUG

#ifndef NDEBUG
/**
 * @def HP_DEBUG_MODE_ACTIVE
 * @brief Convenience flag for enabling/disabling debug mode, or toggling tons of settings at once.
 */
#define HP_DEBUG_MODE_ACTIVE
#elif
/**
 * @def HP_DEBUG_MODE_ACTIVE
 * @brief Convenience flag for enabling/disabling debug mode, or toggling tons of settings at once.
 */
#undef HP_DEBUG_MODE_ACTIVE
#endif


#ifdef HP_DEBUG_MODE_ACTIVE
// Insert custom values for debug mode

/**
 * @def SPDLOG_ACTIVE_LEVEL
 * @brief Define for spdlog logging levels.
 * @details `SPDLOG_LEVEL_TRACE` enables logging of all messages.
 */
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

/**
 * @def HP_PROFILING_ENABLED
 * @brief Global flag to enable/disable profiling. See `profiling.hpp`.
 * @details Enabled by default. (Should be disabled in release, rebuild required.)
 */
#define HP_PROFILING_ENABLED

/**
 * @def HP_LOGGING_ENABLED
 * @brief Global flag to enable/disable logging. See `logging.hpp`.
 * @details Enabled by default. (Should be disabled in release, rebuild required.)
 */
#define HP_LOGGING_ENABLED

/**
 * @def HP_ASYNC_LOGGING_ENABLED
 * @breif Global flag to enable/disable asynchronous logging. See `logging.hpp`.
 * @details Disabled by default. (Rebuild required).
 * @warning Does yield faster logging, but if the program terminates unexpectedly, then not all messages may be logged.
 *          Then you would spend hours debugging when your program seemingly terminates at random points.
 */
#undef HP_ASYNC_LOGGING_ENABLED

/**
 * @def HP_VK_VALIDATION_LAYERS_ENABLED
 * @brief Global flag to enable/disable Vulkan validation layers. See `vk/window.hpp`.
 * @warning Disabling would entirely disable vulkan error checking, so any vulkan errors would just segfault.
 *          Save yourself the pain of spending hours of debugging and enable this feature while debugging.
 *          Once your program is free of errors, disable this in release to get rid of the overhead.
 * @details Enabled by default. (Should be disabled in release, rebuild required.)
 */
#define HP_VK_VALIDATION_LAYERS_ENABLED

#elif
// Insert custom values here for release

/**
 * @def SPDLOG_ACTIVE_LEVEL
 * @brief Define for spdlog logging levels.
 * @details `SPDLOG_LEVEL_TRACE` enables logging of all messages.
 */
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_OFF

/**
 * @def HP_PROFILING_ENABLED
 * @brief Global flag to enable/disable profiling. See `profiling.hpp`.
 * @details Enabled by default. (Should be disabled in release, rebuild required.)
 */
#undef HP_PROFILING_ENABLED

/**
 * @def HP_LOGGING_ENABLED
 * @brief Global flag to enable/disable logging. See `logging.hpp`.
 * @details Enabled by default. (Should be disabled in release, rebuild required.)
 */
#undef HP_LOGGING_ENABLED

/**
 * @def HP_ASYNC_LOGGING_ENABLED
 * @breif Global flag to enable/disable asynchronous logging. See `logging.hpp`.
 * @details Disabled by default. (Rebuild required).
 * @warning Does yield faster logging, but if the program terminates unexpectedly, then not all messages may be logged.
 *          Then you would spend hours debugging when your program seemingly terminates at random points.
 */
#undef HP_ASYNC_LOGGING_ENABLED

/**
 * @def HP_VK_VALIDATION_LAYERS_ENABLED
 * @brief Global flag to enable/disable Vulkan validation layers. See `vk/window.hpp`.
 * @warning Disabling would entirely disable vulkan error checking, so any vulkan errors would just segfault.
 *          Save yourself the pain of spending hours of debugging and enable this feature while debugging.
 *          Once your program is free of errors, disable this in release to get rid of the overhead.
 * @details Enabled by default. (Should be disabled in release, rebuild required.)
 */
#undef HP_VK_VALIDATION_LAYERS_ENABLED

#endif

/*
 * Boolean values representing the state of the macros.
 */
namespace hp {
    namespace vk {

#ifdef HP_VK_VALIDATION_LAYERS_ENABLED
        /**
         * @var const bool validation_layers_enabled
         * @brief `true` if `HP_VK_VALIDATION_LAYERS_ENABLED` is defined, otherwise `false`.
         */
        const bool validation_layers_enabled = true;
#else
        /**
         * @var const bool validation_layers_enabled
         * @brief `true` if `HP_VK_VALIDATION_LAYERS_ENABLED` is defined, otherwise `false`.
         */
        const bool validation_layers_enabled = false;
#endif
    }

#ifdef HP_ASYNC_LOGGING_ENABLED
    /**
     * @var const bool async_logging_enabled
     * @brief `true` if `HP_ASYNC_LOGGING_ENABLED` is defined, otherwise `false`.
     */
    const bool async_logging_enabled = true;
#else
    /**
     * @var const bool async_logging_enabled
     * @brief `true` if `HP_ASYNC_LOGGING_ENABLED` is defined, otherwise `false`.
     */
    const bool async_logging_enabled = false;
#endif

#ifdef HP_PROFILING_ENABLED
    /**
     * @var const bool profiling_enabled
     * @brief `true` if `HP_PROFILING_ENABLED` is defined, otherwise `false`.
     */
    const bool profiling_enabled = true;
#else
    /**
     * @var const bool profiling_enabled
     * @brief `true` if `HP_PROFILING_ENABLED` is defined, otherwise `false`.
     */
    const bool profiling_enabled = false;
#endif

#ifdef HP_LOGGING_ENABLED
    /**
     * @var const bool logging_enabled
     * @brief `true` if `HP_LOGGING_ENABLED` is defined, otherwise `false`.
     */
    const bool logging_enabled = true;
#else
    /**
     * @var const bool logging_enabled
     * @brief `true` if `HP_LOGGING_ENABLED` is defined, otherwise `false`.
     */
    const bool logging_enabled = false;
#endif

#ifdef HP_DEBUG_MODE_ACTIVE
    /**
     * @var const bool debug_mode_active
     * @brief `true` if `HP_DEBUG_MODE_ACTIVE` is defined, otherwise `false`.
     */
    const bool debug_mode_active = true;
#else
    /**
     * @var const bool debug_mode_active
     * @brief `true` if `HP_DEBUG_MODE_ACTIVE` is defined, otherwise `false`.
     */
    const bool debug_mode_active = false;
#endif
}

#endif //HEPHAESTUSSHARED_CONFIG_H
