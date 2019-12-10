cmake_minimum_required(VERSION 3.11)
SET(CMAKE_CXX_STANDARD 17)
enable_language(CXX)

#if(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE Release)
#endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wdeprecated")  # For macOS
#set(CMAKE_CXX_FLAGS "-W4 -std:c++17 -EHsc")  # For windows
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3") # for mac. Comment out for windows

# ======= DEPENDENCY: GLFW =========
set(GLFW_VULKAN_STATIC ON CACHE BOOL "" FORCE)  # mac
#set(GLFW_VULKAN_STATIC OFF CACHE BOOL "" FORCE)  # Windows
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
project(HephaestusShared VERSION 0.0.4 LANGUAGES CXX)
add_library(HephaestusShared SHARED include/hp/hp.hpp src/hp/profiling.cpp include/hp/profiling.hpp include/hp/config.hpp src/hp/logging.cpp include/hp/logging.hpp src/hp/vk/window.cpp include/hp/vk/window.hpp src/hp/vk/vk.cpp include/hp/vk/vk.hpp src/hp/vk/shaders.cpp src/hp/vk/window.cpp include/hp/vk/window.hpp src/hp/multithreading.cpp include/hp/multithreading.hpp src/hp/vk/buffers.cpp)
target_link_libraries(HephaestusShared PUBLIC spdlog glm)
target_include_directories(HephaestusShared PUBLIC src include vendor/glfw/include vendor/glm vendor/spdlog/include vendor ${Boost_INCLUDE_DIR} vendor/vma/src)
target_link_libraries(HephaestusShared PUBLIC glfw)

# Mac Specific code. Comment out if on windows.
target_link_libraries(HephaestusShared PUBLIC ${Boost_LIBRARIES})
target_link_libraries(HephaestusShared PUBLIC ${PROJECT_SOURCE_DIR}/vendor/lunarg-vulkan-sdk/macOS/lib/libvulkan.1.1.126.dylib)
target_include_directories(HephaestusShared PUBLIC vendor/lunarg-vulkan-sdk/macOS/include)

# Windows specific code. Comment out if on mac
#target_link_libraries(HephaestusShared PUBLIC $ENV{VULKAN_SDK}\\Lib\\vulkan-1.lib)
#target_link_directories(HephaestusShared PUBLIC $ENV{VULKAN_SDK}\\Lib)
#target_link_libraries(HephaestusShared PUBLIC $ENV{VULKAN_SDK}\\Lib32\\vulkan-1.lib)
#target_link_directories(HephaestusShared PUBLIC $ENV{VULKAN_SDK}\\Lib32)
#target_link_directories(HephaestusShared PUBLIC C:/Users/grant/Downloads/boost_1_71_0/boost_1_71_0/stage/lib)  # CHange this to whatever you have boost.
#target_include_directories(HephaestusShared PUBLIC $ENV{VULKAN_SDK}/Include)
#message("-- Found vulkan SDK: $ENV{VULKAN_SDK}")


# ====== SANDBOX BUILDING ========
add_subdirectory(examples)

