/**
 * @file vk.hpp
 * @brief Provides basic functionality used across the vulkan core.
 */

#pragma once

#ifndef __HEPHAESTUS_VK_HPP
/**
 * @def __HEPHAESTUS_VK_HPP
 * @brief This macro is defined if `vk.hpp` has been included.
 */
#define __HEPHAESTUS_VK_HPP

#include "hp/logging.hpp"

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan.h>

#include "GLFW/glfw3.h"

#include "vk_mem_alloc.h"

namespace hp::vk {
    /**
     * @fn ::vk::Result hp::vk::handle_res (::vk::Result res, hp::code_location loc)
     * @brief Handle the results returned by vulkan functions, and log it if an error had occurred.
     * @param res The result of the operation to check.
     * @param loc hp::code_location struct returned by macro `HP_GET_CODE_LOC`
     * @return Forwards the `res` parameter that was passed in. Success code is `vk::Result::eSuccess`
     */
    ::vk::Result handle_res(::vk::Result res, hp::code_location loc);

    /**
     * @fn void init_vk()
     * @brief Initialize vulkan.
     * @note Call this function before using ANY Vulkan functionality!
     */
    void init_vk();

    /**
     * @fn void quit_vk()
     * @brief Destroy allocated objects by vulkan.
     * @note Best practice is to call this function upon program exit.
     */
    void quit_vk();
};


#endif //__HEPHAESTUS_VK_HPP
