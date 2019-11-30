cmake_minimum_required(VERSION 3.11)
SET(CMAKE_CXX_STANDARD 17)
enable_language(CXX)

# ======= DEPENDENCY: GLFW =========
set(GLFW_VULKAN_STATIC ON CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
add_subdirectory(vendor/glfw)

# BOOST
FIND_PACKAGE(Boost 1.71.0 COMPONENTS system thread REQUIRED)

# ======= DEPENDENCY: GLM ===========
add_subdirectory(vendor/glm)

#add_subdirectory(vendor/tinygltf)

# ======= DEPENDENCY: SPDLOG ========
add_subdirectory(vendor/spdlog)

# ======= STATIC LIBRARY BUILDING ========  DO NOT USE THIS IS BROKEN! This compile is skipped for now...
#project(BROKEN_DO_NOT_USE_HephaestusStatic VERSION 1.0.0 LANGUAGES CXX)
#add_library(BROKEN_DO_NOT_USE_HephaestusStatic STATIC include/hp/hp.hpp src/hp/profiling.cpp include/hp/profiling.hpp include/hp/config.hpp src/hp/logging.cpp include/hp/logging.hpp src/hp/vk/window.cpp include/hp/vk/window.hpp src/hp/vk/vk.cpp include/hp/vk/vk.hpp)
#target_link_libraries(BROKEN_DO_NOT_USE_HephaestusStatic PUBLIC ${PROJECT_SOURCE_DIR}/lib/vulkan/macOS/lib/libvulkan.1.1.126.dylib spdlog)
#target_include_directories(BROKEN_DO_NOT_USE_HephaestusStatic PUBLIC include src include)
#target_link_libraries(BROKEN_DO_NOT_USE_HephaestusStatic PUBLIC glfw)

# ====== SHARED LIBRARY BUILDING ========
project(HephaestusShared VERSION 0.0.3 LANGUAGES CXX)
add_library(HephaestusShared SHARED include/hp/hp.hpp src/hp/profiling.cpp include/hp/profiling.hpp include/hp/config.hpp src/hp/logging.cpp include/hp/logging.hpp src/hp/vk/window.cpp include/hp/vk/window.hpp src/hp/vk/vk.cpp include/hp/vk/vk.hpp src/hp/vk/shaders.cpp src/hp/vk/window.cpp include/hp/vk/window.hpp src/hp/multithreading.cpp include/hp/multithreading.hpp src/hp/vk/vertex_buffers.cpp)
target_link_libraries(HephaestusShared PUBLIC ${PROJECT_SOURCE_DIR}/vendor/lunarg-vulkan-sdk/macOS/lib/libvulkan.1.1.126.dylib spdlog glm ${Boost_LIBRARIES})
target_include_directories(HephaestusShared PUBLIC src include vendor/glfw/include vendor/glm vendor/spdlog/include vendor vendor/lunarg-vulkan-sdk/macOS/include ${Boost_INCLUDE_DIR} vendor/vma/src)
target_link_libraries(HephaestusShared PUBLIC glfw)


# ====== SANDBOX BUILDING ========
add_subdirectory(sandbox)

