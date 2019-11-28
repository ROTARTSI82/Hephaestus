#define GLFW_INCLUDE_VULKAN

#include "GLFW/glfw3.h"
//#include "vulkan/vulkan.hpp"
#include "hp/logging.hpp"
#include "hp/vk/window.hpp"

int main() {
    hp::init_logging(true);

    hp::vk::init_vk();

    HP_FATAL("Test");

    {
        hp::vk::window inst(640, 480, "Testing", 1);

        auto vbo = inst.new_vbo((sizeof(float) * 3 + sizeof(float) * 2) * 3, 3);
        const std::vector<hp::vk::vertex> vertices = {
                {{0.0f,  -0.5f}, {1.0f, 0.0f, 0.0f}},
                {{0.5f,  0.5f},  {0.0f, 1.0f, 0.0f}},
                {{-0.5f, 0.5f},  {0.0f, 0.0f, 1.0f}}
        };

        vbo->write(reinterpret_cast<const void *>(vertices.data()));

        inst.clear_recording();
        inst.rec_bind_vbo(vbo);
        inst.rec_draw(vbo->vertex_count);

        hp::vk::shader_program *test_shaders = inst.new_shader_program("shader_pack");
        inst.bind_shader_program(test_shaders);

        while (!inst.should_close()) {
            glfwPollEvents();
            inst.draw_frame();
        }

    }
    hp::vk::quit_vk();
}
