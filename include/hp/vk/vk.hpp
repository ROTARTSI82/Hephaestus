//
// Created by 25granty on 11/18/19.
//

#pragma once

#ifndef __HEPHAESTUS_VK_HPP
#define __HEPHAESTUS_VK_HPP

#include "hp/logging.hpp"

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan.h>

#include "GLFW/glfw3.h"

namespace hp::vk {
    /**
     *
     * @param res The result of the operation to check.
     * @param loc hp::code_location struct returned by macro `HP_GET_CODE_LOC`
     * @return Forwards the `res` parameter that was passed in. Success code is vk::Result::eSuccess
     */
    ::vk::Result handle_res(::vk::Result res, hp::code_location loc);

    void init_vk();

    void quit_vk();
};


#endif //__HEPHAESTUS_VK_HPP
