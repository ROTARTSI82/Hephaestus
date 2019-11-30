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

static void recreate_callback(::vk::Extent2D next) {
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
        inst = new hp::vk::window(640, 480, "Testing", 1);
        inst->set_swap_recreate_callback(&recreate_callback);
        shaders = inst->new_shader_program("shader_pack");

        auto ibo_vbo_buf = inst->new_buffer((sizeof(float) * 3 + sizeof(float) * 2) * 4 + sizeof(uint16_t) * 6,
                                            hp::vk::vertex_and_index_usage, hp::vk::memory_local);
        ibo = {ibo_vbo_buf, false, (sizeof(float) * 3 + sizeof(float) * 2) * 4};
        vbo = {ibo_vbo_buf, 0, 4, 0};

        sbo = inst->new_buffer((sizeof(float) * 3 + sizeof(float) * 2) * 4 + sizeof(uint16_t) * 6,
                               hp::vk::staging_usage, hp::vk::memory_host);

        const std::vector<hp::vk::vertex> vertices = {
                {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                {{0.5f,  -0.5f}, {0.0f, 1.0f, 0.0f}},
                {{0.5f,  0.5f},  {0.0f, 0.0f, 1.0f}},
                {{-0.5f, 0.5f},  {1.0f, 1.0f, 1.0f}}
        };

        const std::vector<uint16_t> indices = {
                2, 1, 0, 0, 3, 2
        };

        inst->write_buffer(sbo, reinterpret_cast<const void *>(vertices.data()), 0,
                           (sizeof(float) * 3 + sizeof(float) * 2) * 4);
        inst->write_buffer(sbo, reinterpret_cast<const void *>(indices.data()),
                           (sizeof(float) * 3 + sizeof(float) * 2) * 4, sizeof(uint16_t) * 6);
        inst->copy_buffer(sbo, ibo_vbo_buf, true, 0, 0, (sizeof(float) * 3 + sizeof(float) * 2) * 4);
        inst->copy_buffer(sbo, ibo_vbo_buf, true, (sizeof(float) * 3 + sizeof(float) * 2) * 4,
                          (sizeof(float) * 3 + sizeof(float) * 2) * 4, sizeof(uint16_t) * 6);
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
