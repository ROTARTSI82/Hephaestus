//
// Created by 25granty on 11/16/19.
//

#pragma once

#ifndef __HEPHAESTUS_CONFIG_H
#define __HEPHAESTUS_CONFIG_H

#undef NDEBUG  // THIS LINE FORCEFULLY ENABLES DEBUGGING! REMOVE IN FUTURE

#ifndef NDEBUG
// Convenient flag to enable/disable debug features without affecting other things affected by NDEBUG
#define HP_DEBUG_MODE_ACTIVE
#elif
#undef HP_DEBUG_MODE_ACTIVE
#endif

#ifdef HP_DEBUG_MODE_ACTIVE
/* Insert custom values here for debug */

// Global flag to enable/disable profiling. Requires rebuild of Hephaestus.
#define HP_PROFILING_ENABLED

// Global flag to enable/disable logging. Requires rebuild of Hephaestus and client applications.
#define HP_LOGGING_ENABLED

#define HP_VK_VALIDATION_LAYERS_ENABLED

#elif

// Insert custom values here for release
#undef HP_PROFILING_ENABLED
#undef HP_LOGGING_ENABLED
#undef HP_VK_VALIDATION_LAYERS_ENABLED

#endif

/*
 * Boolean values representing the state of the macros.
 */
namespace hp {
    namespace vk {
#ifdef HP_VK_VALIDATION_LAYERS_ENABLED
        const bool validation_layers_enabled = true;
#else
        const bool validation_layers_enabled = false;
#endif
    }

#ifdef HP_PROFILING_ENABLED
    const bool profiling_enabled = true;
#else
    const bool profiling_enabled = false;
#endif

#ifdef HP_LOGGING_ENABLED
    const bool logging_enabled = true;
#else
    const bool logging_enabled = false
#endif

#ifdef HP_DEBUG_MODE_ACTIVE
    const bool debug_mode_active = true;
#else
    const bool debug_mode_active = false
#endif
}

#endif //HEPHAESTUSSHARED_CONFIG_H
