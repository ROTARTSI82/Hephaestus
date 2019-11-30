#include "hp/vk/window.hpp"

#include "boost/bind.hpp"

namespace hp::vk {
    hp::vk::__hp_vk_is_in_layer_prop_list::__hp_vk_is_in_layer_prop_list(const char *lay) : lay(lay) {}

    hp::vk::__hp_vk_is_in_extension_prop_list::__hp_vk_is_in_extension_prop_list(const char *ext) : ext(ext) {}

    bool hp::vk::queue_family_indices::is_complete() {
        return graphics_fam.has_value() && present_fam.has_value();
    }

    hp::vk::queue_family_indices::queue_family_indices(const hp::vk::queue_family_indices &rhs) {
        *this = rhs;
    }

    hp::vk::queue_family_indices &hp::vk::queue_family_indices::operator=(const hp::vk::queue_family_indices &rhs) {
        if (this == &rhs) {
            return *this;
        }
        graphics_fam = rhs.graphics_fam;
        present_fam = rhs.present_fam;

        return *this;
    }

    hp::vk::queue_family_indices::queue_family_indices(hp::vk::queue_family_indices &&rhs) noexcept {
        *this = std::move(rhs);
    }

    hp::vk::queue_family_indices &hp::vk::queue_family_indices::operator=(hp::vk::queue_family_indices &&rhs) noexcept {
        if (this == &rhs) {
            return *this;
        }
        graphics_fam = rhs.graphics_fam;
        present_fam = rhs.present_fam;
        return *this;
    }

    static void on_resize_event(GLFWwindow *win, int width, int height) {
        auto app = reinterpret_cast<window *>(glfwGetWindowUserPointer(win));
        app->swapchain_recreate_event = true;
    }

    static void on_iconify_event(GLFWwindow *win, int state) {
        if (state == GLFW_TRUE) {
            auto app = reinterpret_cast<window *>(glfwGetWindowUserPointer(win));
            app->swapchain_recreate_event = true;
        }
    }

    static void bind_vbo_helper(::vk::Buffer *vbo, unsigned lyo_indx, ::vk::CommandBuffer cmd, window *win) {
        ::vk::DeviceSize offset = 0;
        cmd.bindVertexBuffers(lyo_indx, 1, vbo, &offset);
    }

    static void bind_shader_helper(::vk::Pipeline pipeline, ::vk::CommandBuffer cmd, window *win) {
        cmd.bindPipeline(::vk::PipelineBindPoint::eGraphics, pipeline);
    }

    static void draw_cmd_helper(unsigned num_verts, ::vk::CommandBuffer cmd, window *win) {
        cmd.draw(num_verts, 1, 0, 0);
    }

    static void set_viewport_helper(::vk::Viewport vp, ::vk::CommandBuffer cmd, window *win) {
        cmd.setViewport(0, 1, &vp);
    }

    static void set_scissor_helper(::vk::Rect2D sc, ::vk::CommandBuffer cmd, window *win) {
        cmd.setScissor(0, 1, &sc);
    }

    static void bind_ibo_helper(::vk::Buffer ibo, ::vk::IndexType type, ::vk::CommandBuffer cmd, window *win) {
        cmd.bindIndexBuffer(ibo, 0, type);
    }

    static void draw_indexed_helper(uint32_t num_indices, ::vk::CommandBuffer cmd, window *win) {
        cmd.drawIndexed(num_indices, 1, 0, 0, 0);
    }

    hp::vk::queue_family_indices build_queue_fam_indices(::vk::PhysicalDevice *dev, ::vk::SurfaceKHR surf) {
        std::vector<::vk::QueueFamilyProperties> queue_fams = dev->getQueueFamilyProperties();
        queue_family_indices ret = {};
        for (size_t i = 0; i < queue_fams.size(); i++) {
            if (queue_fams.at(i).queueFlags & ::vk::QueueFlagBits::eGraphics) {
                ret.graphics_fam = i;
            }

            if (dev->getSurfaceSupportKHR(i, surf)) {
                ret.present_fam = i;
            }

            if (ret.is_complete()) {
                break;
            }
        }

        return ret;
    }

    hp::vk::swap_chain_support get_swap_chain_support(::vk::PhysicalDevice *dev, ::vk::SurfaceKHR surf) {
        hp::vk::swap_chain_support supp{};

        supp.capabilities = dev->getSurfaceCapabilitiesKHR(surf);
        supp.formats = dev->getSurfaceFormatsKHR(surf);
        supp.present_modes = dev->getSurfacePresentModesKHR(surf);

        return supp;
    }

    ::vk::Result hp::vk::window::createDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator,
                                                              VkDebugUtilsMessengerEXT *pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) inst.getProcAddr("vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return handle_res(::vk::Result(func(inst, pCreateInfo, pAllocator, pDebugMessenger)), HP_GET_CODE_LOC);
        } else {
            return ::vk::Result::eErrorExtensionNotPresent;
        }
    }

    ::vk::Bool32 hp::vk::window::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                               VkDebugUtilsMessageTypeFlagsEXT messageType,
                                               const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                               void *pUserData) {
        HP_FATAL("[** VULKAN ERROR **]: {}", pCallbackData->pMessage);
        return VK_FALSE;
    }

    void hp::vk::window::destroyDebugUtilsMessengerEXT(VkDebugUtilsMessengerEXT debugMessenger,
                                                       const VkAllocationCallbacks *pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) inst.getProcAddr("vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(inst, debugMessenger, pAllocator);
        }
    }

    hp::vk::window::window(hp::vk::window &&other) noexcept {
        *this = std::move(other);
    }

    hp::vk::window &hp::vk::window::operator=(hp::vk::window &&other) noexcept {
        if (&other == this) { // Self-assignment; do nothing
            return *this;
        }

        this->~window();

        phys_dev = other.phys_dev;
        phys_dev_ext = std::move(other.phys_dev_ext);
        queue_fam_indices = std::move(other.queue_fam_indices);
        devices = std::move(other.devices);
        log_dev = other.log_dev;
        inst = other.inst;
        supported_ext = std::move(other.supported_ext);
        uses_validation_layers = other.uses_validation_layers;
        supported_lay = std::move(other.supported_lay);
        debug_msgr = other.debug_msgr;
        win = other.win;
        surf = other.surf;
        graphics_queue = other.graphics_queue;
        present_queue = other.present_queue;
        swap_chain = other.swap_chain;
        swap_extent = other.swap_extent;
        swap_fmt = other.swap_fmt;
        swap_imgs = std::move(other.swap_imgs);
        child_shaders = std::move(other.child_shaders);
        render_pass = other.render_pass;
        framebuffers = std::move(other.framebuffers);
        cmd_pool = other.cmd_pool;
        img_avail_sms = std::move(other.img_avail_sms);
        rend_fin_sms = std::move(other.rend_fin_sms);
        current_frame = other.current_frame;
        flight_fences = std::move(other.flight_fences);
        img_fences = std::move(other.img_fences);
        swapchain_recreate_event = other.swapchain_recreate_event;
        mem_props = other.mem_props;
        child_bufs = std::move(other.child_bufs);
        record_buffer = std::move(other.record_buffer);
        cmd_bufs = std::move(other.cmd_bufs);
        swap_recreate_callback = other.swap_recreate_callback;
        allocator = other.allocator;
        child_bufs = std::move(other.child_bufs);
        child_fences = std::move(other.child_fences);
//        render_mtx = std::move(other.render_mtx);
        return *this;
    }

    void window::rec_bind_vbo(vertex_buffer *vbo) {
        record_buffer.emplace_back(boost::bind(bind_vbo_helper, &vbo->buf, vbo->layout_index, _1, _2));
    }

    void window::rec_draw(unsigned num_verts) {
        record_buffer.emplace_back(boost::bind(draw_cmd_helper, num_verts, _1, _2));
    }

    void window::rec_bind_shader(shader_program *shader) {
        record_buffer.emplace_back(boost::bind(bind_shader_helper, shader->pipeline, _1, _2));
    }

    void window::rec_set_viewport(::vk::Viewport viewport) {
        record_buffer.emplace_back(boost::bind(set_viewport_helper, viewport, _1, _2));
    }

    void window::rec_set_scissor(::vk::Rect2D scissor) {
        record_buffer.emplace_back(boost::bind(set_scissor_helper, scissor, _1, _2));
    }

    void window::rec_set_default_viewport() {
        ::vk::Viewport viewport(0.0f, 0.0f, (float) swap_extent.width, (float) swap_extent.height, 0.0f,
                                1.0f);
        record_buffer.emplace_back(boost::bind(set_viewport_helper, viewport, _1, _2));
    }

    void window::rec_set_default_scissor() {
        ::vk::Rect2D scissor(::vk::Offset2D(0, 0), swap_extent);
        record_buffer.emplace_back(boost::bind(set_scissor_helper, scissor, _1, _2));
    }

    void window::clear_recording() {
        record_buffer.clear();
    }

    void window::rec_bind_index_buffer(index_buffer *ibo) {
        record_buffer.emplace_back(boost::bind(bind_ibo_helper, ibo->buf,
                                               ibo->is32bit ? ::vk::IndexType::eUint32 : ::vk::IndexType::eUint16, _1,
                                               _2));
    }

    void window::rec_draw_indexed(uint32_t num_indices) {
        record_buffer.emplace_back(boost::bind(draw_indexed_helper, num_indices, _1, _2));
    }
}
