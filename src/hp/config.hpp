//
// Created by 25granty on 11/16/19.
//

#pragma once

#ifndef __HEPHAESTUS_CONFIG_H
#define __HEPHAESTUS_CONFIG_H

// Convenient flag to enable/disable debug features.
#define HP_DEBUG_MODE_ACTIVE

#ifdef HP_DEBUG_MODE_ACTIVE
/* Insert custom values here for debug */

// Global flag to enable/disable profiling. Requires rebuild of Hephaestus.
#define HP_PROFILING_ENABLED

// Global flag to enable/disable logging. Requires rebuild of Hephaestus and client applications.
#define HP_LOGGING_ENABLED

#elif

// Insert custom values here for release
#endif
namespace hp {
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
