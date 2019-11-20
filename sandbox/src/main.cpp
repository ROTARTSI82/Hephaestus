#define GLFW_INCLUDE_VULKAN

#include "GLFW/glfw3.h"
//#include "vulkan/vulkan.hpp"
#include "hp/logging.hpp"
#include "hp/vk/instance.hpp"

int main() {
    hp::init_logging(true);

    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);  // Don't automatically create an OpenGL context
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);  // Don't allow resizing for now: Resizing requires special vulkan code.

    GLFWwindow *window = glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);

    {
        hp::vk::instance inst("Testing", 1);



        // Main loop!
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
    }


        // Cleanup.
//    vkDestroyInstance(instance, nullptr);
    }
    glfwDestroyWindow(window);
    glfwTerminate();
//    delete [] support_ext_name_arr; // This prob req by vulkan
}
