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

    /**
    * @var extern const ::vk::MemoryPropertyFlags memory_local
    * @brief Vulkan memory property flag specifying that memory should be allocated in a place quickly accessible by GPU.
    * @note This memory type is easily read by GPU, but has to be written to using a staging buffer.
    * @details Has value of `vk::MemoryPropertyFlagBits::eDeviceLocal`. Consult vulkan documentation for more details.
    *          Use this memory property flag with `hp::vk::vertex_and_index_usage`, `hp::vk::index_usage`, and `hp::vk::vertex_usage`
    */
    extern const ::vk::MemoryPropertyFlags memory_local;
    /**
     * @var extern const ::vk::MemoryPropertyFlags memory_host
     * @brief Vulkan memory property flag specifying that memory should be allocated in a place that is synchronized and writable by CPU.
     * @note This memory type could be written to directly from the CPU, but the GPU has a harder time reading it.
     * @details Has value of `vk::MemoryPropertyFlagBits::eHostVisible`. Consult vulkan documentation for more details.
     *           Use this memory property flag with `hp::vk::vertex_direct_usage`, `hp::vk::index_direct_usage`, `hp::vk::staging_usage`, and `hp::vk::vertex_and_index_direct_usage`
     */
    extern const ::vk::MemoryPropertyFlags memory_host;

    /**
     * @var extern const ::vk::BufferUsageFlags vertex_usage
     * @see hp::vk::memory_local
     * @brief Vulkan buffer usage flag specifying the buffer for usage as a vertex_buffer (VBO) written to using a staging buffer
     * @details Has value of `vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer`. Consult vulkan documentation for more details.
     */
    extern const ::vk::BufferUsageFlags vertex_usage;
    /**
     * @var extern const ::vk::BufferUsageFlags index_usage
     * @see hp::vk::memory_local
     * @brief Vulkan buffer usage flag specifying the buffer for usage as a index buffer (IBO) written to using a staging buffer.
     * @details Has value of `vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer`. Consult vulkan documentation for more details.
     */
    extern const ::vk::BufferUsageFlags index_usage;
    /**
     * @var extern const ::vk::BufferUsageFlags vertex_direct_usage
     * @see hp::vk::memory_host
     * @brief Vulkan buffer usage flag specifying the buffer for usage as a vertex buffer (VBO) written to directly from CPU.
     * @details Has value of `vk::BufferUsageFlagBits::eVertexBuffer`. Consult vulkan documentation for more details.
     */
    extern const ::vk::BufferUsageFlags vertex_direct_usage;
    /**
     * @var extern const ::vk::BufferUsageFlags index_direct_usage
     * @see hp::vk::memory_host
     * @brief Vulkan buffer usage flag specifying the buffer for usage as a index buffer (IBO) written to directly from CPU.
     * @details Has value of `vk::BufferUsageFlagBits::eIndexBuffer`. Consult vulkan documentation for more details.
     */
    extern const ::vk::BufferUsageFlags index_direct_usage;

    /**
     * @var extern const ::vk::BufferUsageFlags staging_usage
     * @see hp::vk::memory_host
     * @brief Vulkan buffer usage flag specifying the buffer for usage as a staging buffer written to directly from the CPU.
     * @details Has value of `vk::BufferUsageFlagBits::eTransferSrc`. Consult vulkan documentation for more details.
     */
    extern const ::vk::BufferUsageFlags staging_usage;
    /**
     * @var extern const ::vk::BufferUsageFlags vertex_and_index_usage
     * @see hp::vk::memory_local
     * @brief Vulkan buffer usage flag specifying the buffer for usage as a vertex buffer (VBO) *OR* index buffer (IBO) written to using a staging buffer.
     * @details Has value of `vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eVertexBuffer`. Consult vulkan documentation for more details.
     */
    extern const ::vk::BufferUsageFlags vertex_and_index_usage;
    /**
     * @var extern const ::vk::BufferUsageFlags vertex_and_index_direct_usage;
     * @see hp::vk::memory_host
     * @brief Vulkan buffer usage flag specifying the buffer for usage as a vertex buffer (VBO) *OR* index buffer (IBO) written to directly from the CPU.
     * @details Has value of `vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eVertexBuffer`. Consult vulkan documentation for more details.
     */
    extern const ::vk::BufferUsageFlags vertex_and_index_direct_usage;
};


#endif //__HEPHAESTUS_VK_HPP
