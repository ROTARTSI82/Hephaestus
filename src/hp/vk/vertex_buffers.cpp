//
// Created by 25granty on 11/27/19.
//

#include "hp/vk/window.hpp"

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

    vertex_buffer::vertex_buffer(size_t size, unsigned num_verts, window *parent) : capacity(size),
                                                                                    vertex_count(num_verts) {
        this->parent = parent;
        ::vk::BufferCreateInfo buf_ci(::vk::BufferCreateFlags(), size, ::vk::BufferUsageFlagBits::eVertexBuffer,
                                      ::vk::SharingMode::eExclusive, 0, nullptr);

        if (handle_res(parent->log_dev.createBuffer(&buf_ci, nullptr, &buf), HP_GET_CODE_LOC) !=
            ::vk::Result::eSuccess) {
            HP_FATAL("Failed to create vertex buffer!");
            std::terminate();
        }

        ::vk::MemoryRequirements mem_req;
        parent->log_dev.getBufferMemoryRequirements(buf, &mem_req);

        uint32_t mem_type = UINT32_MAX;
        // VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        ::vk::MemoryPropertyFlags req_flags =
                ::vk::MemoryPropertyFlagBits::eHostVisible | ::vk::MemoryPropertyFlagBits::eHostCoherent;
        for (uint32_t i = 0; i < parent->mem_props.memoryTypeCount; i++) {
            if (mem_req.memoryTypeBits & (1 << i) &&
                (parent->mem_props.memoryTypes[i].propertyFlags & req_flags) == req_flags) {
                mem_type = i;
                break;
            }
        }

        if (mem_type == UINT32_MAX) {
            HP_FATAL("No suitable memory type is supported!");
            std::terminate();
        }

        ::vk::MemoryAllocateInfo alloc_inf(mem_req.size, mem_type);

        if (handle_res(parent->log_dev.allocateMemory(&alloc_inf, nullptr, &mem), HP_GET_CODE_LOC) !=
            ::vk::Result::eSuccess) {
            HP_FATAL("Cannot allocate memory required for vertex buffer! Did we run out of memory?");
            std::terminate();
        }

        parent->log_dev.bindBufferMemory(buf, mem, 0);
    }

    vertex_buffer::~vertex_buffer() {
        parent->log_dev.destroyBuffer(buf, nullptr);
        parent->log_dev.freeMemory(mem, nullptr);
    }

    void vertex_buffer::write(const void *data) {
        void *dat;
        parent->log_dev.mapMemory(mem, 0, capacity, ::vk::MemoryMapFlags(), &dat);
        std::memcpy(dat, data, capacity);
        parent->log_dev.unmapMemory(mem);
    }
}
