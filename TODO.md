Inspiration: https://github.com/EQMG/Acid

structure of ubo layout:
  - `ubo_layout` object
       - `add_binding()`
       - `static std::vector<ubo_layout *> bound_lyos`
       
 Use lyos in `bound_lyos` at pipeline create time. Similar to buffer thingies.

# GENERAL
- Potential future bug: Swapchain format changing and triggering code that is yet to be tested.
- [FUTURE PLAN?] Add support for Instancing
# DOCUMENTATION
- Update documentation to remove refs to rebuild & stuff.
- Finish rest of documentation

# PHYSICS ENGINE
- Bullet Physics? Box2D? No clue.
- https://github.com/bulletphysics/bullet3
# NETWORKING
- Raknet?
# OPENAL

# FREETYPE

# MODLE LOADER
- Decide between `assimp` (more beefy, meant for opengl) and `tinygltf` (Vulkan tutorial uses it).