#About  `run.sh`
### Options for `run.sh`:
1. `-d` Builds documentation. Requires `doxygen` and `grpahviz`
2. `-nr` Skip running Hephaestus
2. `-nb` Skip compiling/building Hephaestus
3. `-c` Clean install. Removes all existing documentation and built binaries.

#Building and Running Hephaestus

### Mac OSX
1. Download LunarG's Vulkan SDK from https://vulkan.lunarg.com/ and place it at `vendor/lunarg-vulkan-sdk`
2. Install Boost 1.71.0 by running `brew install boost` or `brew upgrade boost` to update boost.
3. Ensure that all git submodules are cloned and present with `git submodule init` and `git submodule update`
4. `cd` to the root of the project and run `bash run.sh`. (Do not run with sudo!)

### Windows, Unix, etc
Sorry. I don't have access to a Linux or Windows machine so follow instructions at https://vulkan-tutorial.com/Development_environment.
Modify `CMakeLists.txt` and `sandbox/CMakeLists.txt` accordingly.
- Also modify `run.sh`'s lines 3 & 4 to export `VK_ICD_FILENAMES` and `VK_LAYER_PATH` to point to your validation layers. Otherwise
  vulkan would fail to create the instance with `VK_ERROR_INCOMPATIBLE_DRIVER`. If you aren't using validation layers
  (i.e. They are disabled in `hp/config.hpp`), you can just skip exporting the variables.

# Building Documentation

##Mac OSX
1. Install doxygen with `brew install doxygen` (and upgrade it with `brew upgrade doxygen`).
    - *NOTE*: For some reason, doxygen's dependencies aren't installed by brew.
              If you want to build documentation with graphs, run `brew install graphviz`
2. `cd` to the root of the project and run `doxygen`. (Or use `run.sh`)

##Windows, Unix, etc
1. Install `doxygen` and `grpahviz`
2. `cd` to the root of the project and run `doxygen`. (Or use `run.sh`)