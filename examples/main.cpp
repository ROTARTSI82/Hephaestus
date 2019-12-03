#define GLFW_INCLUDE_VULKAN

#include "GLFW/glfw3.h"
//#include "vulkan/vulkan.hpp"
#include "hp/logging.hpp"
#include "hp/vk/window.hpp"

hp::vk::generic_buffer *sbo;
hp::vk::index_buffer ibo;
hp::vk::vertex_buffer vbo;
hp::vk::window *inst;
hp::vk::shader_program *shaders;

static void recreate_callback(::vk::Extent2D new_extent) {
    inst->clear_recording();
    inst->rec_bind_shader(shaders);
    inst->rec_set_default_viewport();
    inst->rec_set_default_scissor();
    inst->rec_bind_vbo(vbo);
    inst->rec_bind_index_buffer(ibo);
    inst->rec_draw_indexed(ibo.get_num_indices());

    HP_FATAL("YEEET RECREATE CALLBACK!");
}

int main() {
    hp::init_logging(true);

    hp::vk::init_vk();

    HP_FATAL("Test");

    {
        inst = new hp::vk::window(640, 480, "Testing", 2);
        inst->set_swap_recreate_callback(&recreate_callback);
        shaders = inst->new_shader_program("shader_pack");

        const size_t vbo_size = (sizeof(float) * 3 + sizeof(float) * 2) * 4;
        const size_t ibo_size = sizeof(uint16_t) * 6;

        auto ibo_vbo_buf = inst->new_buffer(vbo_size + ibo_size,
                                            hp::vk::vertex_and_index_usage, hp::vk::memory_local);
        ibo = {ibo_vbo_buf, false, vbo_size};
        vbo = {ibo_vbo_buf, 0, 4, 0};

        sbo = inst->new_buffer(vbo_size + ibo_size, hp::vk::staging_usage, hp::vk::memory_host);

        const std::vector<hp::vk::vertex> vertices = {
                {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                {{0.5f,  -0.5f}, {0.0f, 1.0f, 0.0f}},
                {{0.5f,  0.5f},  {0.0f, 0.0f, 1.0f}},
                {{-0.5f, 0.5f},  {1.0f, 1.0f, 1.0f}}
        };

        const std::vector<uint16_t> indices = {
                2, 1, 0, 0, 3, 2
        };

        auto write = sbo->start_write();
        sbo->write_buffer(write, reinterpret_cast<const void *>(vertices.data()), 0, vbo_size);
        sbo->write_buffer(write, reinterpret_cast<const void *>(indices.data()), vbo_size, ibo_size);
        sbo->flush();
        sbo->invalidate();
        sbo->stop_write();

        auto cpy1 = inst->copy_buffer(sbo, ibo_vbo_buf, false, 0, 0, vbo_size);
        auto cpy2 = inst->copy_buffer(sbo, ibo_vbo_buf, false, vbo_size, vbo_size, ibo_size);
        {
            // This is safe because, if it fails, null handles are safe to pass to these funcs.
            ::vk::Fence fences[] = {cpy1.first, cpy2.first};
            ::vk::CommandBuffer bufs[] = {cpy1.second, cpy2.second};
            inst->wait_fences(fences, 2);
            inst->delete_fence(cpy1.first);
            inst->delete_fence(cpy2.first);
            inst->delete_cmd_buffers(bufs, 2);
        }

        inst->delete_buffer(sbo);

        inst->clear_recording();
        inst->rec_bind_shader(shaders);
        inst->rec_set_default_viewport();
        inst->rec_set_default_scissor();
        inst->rec_bind_vbo(vbo);
        inst->rec_bind_index_buffer(ibo);
        inst->rec_draw_indexed(ibo.get_num_indices());
        inst->save_recording();

        while (!inst->should_close()) {
            glfwPollEvents();
            inst->draw_frame();
        }

        delete inst;

    }
    hp::vk::quit_vk();
}
