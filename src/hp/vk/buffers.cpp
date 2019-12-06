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

        attribs.emplace_back(::vk::VertexInputAttributeDescription(seek_val, 0, fmt, stride));
        seek_val++;
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

    buffer_layout buffer_layout::default_lyo = buffer_layout();
    std::vector<buffer_layout *> buffer_layout::bound_lyos = std::vector<buffer_layout *>();
    std::vector<::vk::VertexInputAttributeDescription> buffer_layout::global_attribs = std::vector<::vk::VertexInputAttributeDescription>();
    std::vector<::vk::VertexInputBindingDescription> buffer_layout::global_bindings = std::vector<::vk::VertexInputBindingDescription>();

    void buffer_layout::build_default_layout() {
        if (default_lyo.complete) { // Quietly ignore this case.
            return;
        }
        default_lyo.push_floats(2); // Position;
        default_lyo.push_floats(3); // Color;
        default_lyo.finalize();
    }

    void buffer_layout::rebuild_bound_info() {
        global_attribs.clear();
        global_bindings.clear();

        for (size_t i = 0; i < bound_lyos.size(); i++) {
            for (auto &attr : bound_lyos[i]->attribs) {
                attr.binding = i;
                global_attribs.emplace_back(attr);
            }

            bound_lyos[i]->binding.binding = i;
            global_bindings.emplace_back(bound_lyos[i]->binding);
        }
    }

    generic_buffer::generic_buffer(size_t size, const ::vk::BufferUsageFlags &usage,
                                   const ::vk::MemoryPropertyFlags &flags, window *parent) {
        capacity = size;
        this->parent = parent;

        VkBufferCreateInfo buffer_ci = {};
        buffer_ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_ci.flags = 0;
        buffer_ci.size = size;
        buffer_ci.usage = static_cast<VkBufferUsageFlags>(usage);
        buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        buffer_ci.queueFamilyIndexCount = 0;
        buffer_ci.pQueueFamilyIndices = nullptr;

        VmaAllocationCreateInfo alloc_ci = {};
        alloc_ci.usage = VMA_MEMORY_USAGE_UNKNOWN; // VMA_MEMORY_USAGE_CPU_TO_GPU;
        alloc_ci.requiredFlags = static_cast<VkMemoryPropertyFlags>(flags);
        alloc_ci.preferredFlags = static_cast<VkMemoryPropertyFlags>(::vk::MemoryPropertyFlagBits::eHostCached);

        auto vanilla_buf = static_cast<VkBuffer>(buf);
        handle_res(::vk::Result(
                vmaCreateBuffer(parent->allocator, &buffer_ci, &alloc_ci, &vanilla_buf, &allocation, nullptr)),
                   HP_GET_CODE_LOC);
        buf = ::vk::Buffer(vanilla_buf);
    }

    generic_buffer::~generic_buffer() {
        vmaDestroyBuffer(parent->allocator, static_cast<VkBuffer>(buf), allocation);
    }

    generic_buffer::generic_buffer(generic_buffer &&rhs) noexcept {
        *this = std::move(rhs);
    }

    generic_buffer &generic_buffer::operator=(generic_buffer &&rhs) noexcept {
        if (this == &rhs) {
            return *this;
        }
        capacity = rhs.capacity;
        buf = rhs.buf;
        parent = rhs.parent;
        allocation = rhs.allocation;
        return *this;
    }

    void generic_buffer::flush(::vk::DeviceSize offset, ::vk::DeviceSize size) {
        vmaFlushAllocation(parent->allocator, allocation, offset, size);
    }

    void generic_buffer::invalidate(::vk::DeviceSize offset, ::vk::DeviceSize size) {
        vmaInvalidateAllocation(parent->allocator, allocation, offset, size);
    }

    void generic_buffer::write_buffer(const void *data, size_t offset, size_t size) {
        uint8_t *mapped_data;
        vmaMapMemory(parent->allocator, allocation, reinterpret_cast<void **>(&mapped_data));
        std::memcpy(mapped_data + offset, data, size == 0 ? capacity : size);
        vmaFlushAllocation(parent->allocator, allocation, 0, VK_WHOLE_SIZE);
        vmaInvalidateAllocation(parent->allocator, allocation, 0, VK_WHOLE_SIZE);
        vmaUnmapMemory(parent->allocator, allocation);
    }

    uint8_t *generic_buffer::start_write() {
        uint8_t *ret;
        vmaMapMemory(parent->allocator, allocation, reinterpret_cast<void **>(&ret));
        return ret;
    }

    void generic_buffer::write_buffer(uint8_t *dest, const void *src, size_t offset, size_t size) {
        std::memcpy(dest + offset, src, size == 0 ? capacity : size);
    }

    void generic_buffer::stop_write() {
        vmaUnmapMemory(parent->allocator, allocation);
    }
}
