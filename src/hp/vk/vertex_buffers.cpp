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

    staging_buffer::staging_buffer(size_t size, window *parent) :
            generic_buffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, parent) {}

    void staging_buffer::write(const void *data) {
        void *dat;
        vmaMapMemory(parent->allocator, allocation, &dat);
        std::memcpy(dat, data, capacity);
        vmaUnmapMemory(parent->allocator, allocation);
    }

    vertex_buffer::vertex_buffer(size_t size, unsigned num_verts, unsigned lyo_indx, window *parent)
            : vertex_count(num_verts), layout_index(lyo_indx),
              generic_buffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, parent) {}

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
        handle_res(::vk::Result(
                vmaCreateBuffer(parent->allocator, &buffer_ci, &alloc_ci, &vanilla_buf, &allocation, nullptr)),
                   HP_GET_CODE_LOC);
        buf = ::vk::Buffer(vanilla_buf);
    }

    generic_buffer::~generic_buffer() {
        vmaDestroyBuffer(parent->allocator, static_cast<VkBuffer>(buf), allocation);
    }

    index_buffer::index_buffer(size_t size, bool is32bit, window *parent) : generic_buffer(size,
                                                                                           VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                                                                           VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                                                                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                                                           parent), is32bit(is32bit) {}

    std::pair<::vk::Fence *, ::vk::CommandBuffer> index_buffer::write(staging_buffer *staging_buf, bool wait) {
        return parent->copy_buffer(staging_buf, this, wait);
    }
}
