#Building and Running Hephaestus

##Mac OSX
1. Download LunarG's Vulkan SDK from https://vulkan.lunarg.com/ and place it at `vendor/lunarg-vulkan-sdk`
2. Download boost from https://www.boost.org/users/download/ and place it at `vendor/boost`
3. Ensure that all git submodules are cloned and present with `git submodule init` and `git submodule update`
4. `cd` to the root directory and run `bash run.sh`. (Do not run with sudo!)

## Linux & Windows
Sorry. I don't have access to a Linux or Windows machine so follow instructions at https://vulkan-tutorial.com/Development_environment.
Modify `CMakeLists.txt` and `sandbox/CMakeLists.txt` accordingly.
- Also modify `run.sh`'s lines 3 & 4 to export `VK_ICD_FILENAMES` and `VK_LAYER_PATH` to point to your validation layers.


## Other
WTF are you using if it's not Mac, Linux or Windows? Go die in a hole.