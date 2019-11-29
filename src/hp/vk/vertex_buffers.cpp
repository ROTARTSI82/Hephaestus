//
// Created by 25granty on 11/27/19.
//

#include "hp/vk/window.hpp"
#include "vk_mem_alloc.h"

namespace hp::vk {

    void buffer_layout::push_floats(int num_floats) {
        if (complete) {
            HP_WARN("push_floats() called on already complete buffer layout! Ignoring invocation!");
            return;
        }
        ::vk::Format fmt;
        switch (num_floats) {
            case (1): {
                fmt = ::vk::Format::eR32Sfloat;
                break;
            }
            case (2): {
                fmt = ::vk::Format::eR32G32Sfloat;
                break;
            }
            case (3): {
                fmt = ::vk::Format::eR32G32B32Sfloat;
                break;
            }
            case (4): {
                fmt = ::vk::Format::eR32G32B32A32Sfloat;
                break;
            }
            default: {
                HP_WARN("hp::buffer_layout::push_floats called with invalid number of floats: {}! Ignoring invocation!",
                        num_floats);
                return;
            }
        }

        attribs.emplace_back(::vk::VertexInputAttributeDescription(attribs.size(), 0, fmt, stride));
        stride += sizeof(float) * num_floats;
    }

    void buffer_layout::finalize() {
        if (complete) {
            HP_WARN("finalize() called on already complete buffer layout! Ignoring invocation!");
            return;
        }

        binding = ::vk::VertexInputBindingDescription(0, stride, ::vk::VertexInputRate::eVertex);
        complete = true;
    }

    buffer_layout &buffer_layout::operator=(const buffer_layout &rhs) {
        if (&rhs == this) {
            return *this;
        }

        binding = rhs.binding;
        stride = rhs.stride;
        attribs = rhs.attribs;
        complete = rhs.complete;

        return *this;
    }

    buffer_layout::buffer_layout(const buffer_layout &rhs) {
        *this = rhs;
    }

    buffer_layout &buffer_layout::operator=(buffer_layout &&rhs) noexcept {
        if (&rhs == this) {
            return *this;
        }

        binding = rhs.binding;
        stride = rhs.stride;
        attribs = std::move(rhs.attribs);
        complete = rhs.complete;

        return *this;
    }

    buffer_layout::buffer_layout(buffer_layout &&rhs) noexcept {
        *this = std::move(rhs);
    }

    const buffer_layout *buffer_layout::active_lyo = nullptr;
    buffer_layout buffer_layout::default_lyo = buffer_layout();

    buffer_layout::~buffer_layout() {
        if (active_lyo == this) {
            active_lyo = nullptr;
            HP_WARN("Bound buffer layout was destroyed! Unbinding it!");
            HP_WARN("This may not be intended behaviour and may lead to segfaults; did a copy of the bound layout go out of scope?");
        }
    }

    void buffer_layout::build_default_layout() {
        if (default_lyo.complete) { // Quietly ignore this case.
            return;
        }
        default_lyo.push_floats(2); // Position;
        default_lyo.push_floats(3); // Color;
        default_lyo.finalize();
    }

    staging_buffer::staging_buffer(size_t size, unsigned num_verts, window *parent) :
            generic_buffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, parent),
            vertex_count(num_verts) {}

    void staging_buffer::write(const void *data) {
        void *dat;
        vmaMapMemory(parent->allocator, allocation, &dat);
        std::memcpy(dat, data, capacity);
        vmaUnmapMemory(parent->allocator, allocation);
    }

    vertex_buffer::vertex_buffer(size_t size, unsigned num_verts, window *parent) : vertex_count(num_verts),
                                                                                    generic_buffer(size,
                                                                                                   VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                                                                                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                                                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                                                                   parent) {}

    std::pair<::vk::Fence *, ::vk::CommandBuffer> vertex_buffer::write(staging_buffer *staging_buf, bool wait) {
        return parent->copy_buffer(staging_buf, this, wait);
    }

    generic_buffer::generic_buffer(size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags, window *parent) {
        capacity = size;
        this->parent = parent;

        VkBufferCreateInfo buffer_ci = {};
        buffer_ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_ci.flags = 0;
        buffer_ci.size = size;
        buffer_ci.usage = usage;
        buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        buffer_ci.queueFamilyIndexCount = 0;
        buffer_ci.pQueueFamilyIndices = nullptr;

        VmaAllocationCreateInfo alloc_ci = {};
        alloc_ci.usage = VMA_MEMORY_USAGE_GPU_ONLY; // VMA_MEMORY_USAGE_CPU_TO_GPU;
        alloc_ci.requiredFlags = flags;

        auto vanilla_buf = static_cast<VkBuffer>(buf);
        vmaCreateBuffer(parent->allocator, &buffer_ci, &alloc_ci, &vanilla_buf, &allocation, nullptr);
        buf = ::vk::Buffer(vanilla_buf);
    }

    generic_buffer::~generic_buffer() {
        vmaDestroyBuffer(parent->allocator, static_cast<VkBuffer>(buf), allocation);
    }
}
