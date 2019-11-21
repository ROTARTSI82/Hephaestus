//
// Created by 25granty on 11/18/19.
//

#pragma once

#ifndef __HEPHAESTUS_VK_HPP
#define __HEPHAESTUS_VK_HPP

#include "hp/logging.hpp"
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan.h>

namespace hp::vk {
    ::vk::Result handle_res(::vk::Result res, hp::code_location loc);
};


#endif //__HEPHAESTUS_VK_HPP
