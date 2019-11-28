#define GLFW_INCLUDE_VULKAN

#include "GLFW/glfw3.h"
//#include "vulkan/vulkan.hpp"
#include "hp/logging.hpp"
#include "hp/vk/window.hpp"

hp::vk::vertex_buffer *vbo;
hp::vk::window *inst;
hp::vk::shader_program *shaders;

static void recreate_callback(::vk::Extent2D next) {
    inst->clear_recording();
    inst->rec_bind_shader(shaders);
    inst->rec_set_default_viewport();
    inst->rec_set_default_scissor();
    inst->rec_bind_vbo(vbo);
    inst->rec_draw(vbo->vertex_count);

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

        vbo = inst->new_vbo((sizeof(float) * 3 + sizeof(float) * 2) * 3, 3);
        const std::vector<hp::vk::vertex> vertices = {
                {{0.0f,  -0.5f}, {1.0f, 0.0f, 0.0f}},
                {{0.5f,  0.5f},  {0.0f, 1.0f, 0.0f}},
                {{-0.5f, 0.5f},  {0.0f, 0.0f, 1.0f}}
        };

        vbo->write(reinterpret_cast<const void *>(vertices.data()));

        inst->clear_recording();
        inst->rec_bind_shader(shaders);
        inst->rec_set_default_viewport();
        inst->rec_set_default_scissor();
        inst->rec_bind_vbo(vbo);
        inst->rec_draw(vbo->vertex_count);
        inst->save_recording();

        while (!inst->should_close()) {
            glfwPollEvents();
            inst->draw_frame();
        }

        delete inst;

    }
    hp::vk::quit_vk();
}
