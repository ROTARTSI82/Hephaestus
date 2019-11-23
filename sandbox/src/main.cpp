#define GLFW_INCLUDE_VULKAN

#include "GLFW/glfw3.h"
//#include "vulkan/vulkan.hpp"
#include "hp/logging.hpp"
#include "hp/vk/window.hpp"

int main() {
    hp::init_logging(true);

    glfwInit();

    HP_FATAL("Test");

    {
        hp::vk::window inst(640, 480, "Testing", 1);
        hp::vk::shader_program test_shaders = std::move(inst.new_shader_program("shader_pack"));

        while (!inst.should_close()) {
            glfwPollEvents();
        }

    }
    glfwTerminate();
}
