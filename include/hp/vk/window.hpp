/**
 * @file window.hpp
 * @brief Provide basically all Vulkan functionality, including vertex buffers, shaders, & windows.
 */

#pragma once

#ifndef __HEPHAESTUS_VK_WINDOW_HPP

/**
 * @def __HEPHAESTUS_VK_WINDOW_HPP
 * @brief This macro is defined if `window.hpp` has been included.
 */
#define __HEPHAESTUS_VK_WINDOW_HPP

#include "hp/config.hpp"
#include "hp/vk/vk.hpp"
#include "hp/hp.hpp"

#include "glm/glm.hpp"

#include <map>
#include <set>
#include <queue>
#include "vk_mem_alloc.h"

#include "boost/range/algorithm_ext/erase.hpp"

namespace hp::vk {

    /**
     * @struct __hp_vk_is_in_extension_prop_list
     * @private
     */
    struct __hp_vk_is_in_extension_prop_list { ///< @private
        const char *ext; ///< @private

        explicit __hp_vk_is_in_extension_prop_list(const char *ext); ///< @private

        /**
         * @fun inline bool operator()(::vk::ExtensionPropertie other) const
         * @private
         */
        inline bool operator()(::vk::ExtensionProperties other) const { ///< @private
            return strcmp(other.extensionName, ext) == 0;
        };
    };

    /**
     * @struct __hp_vk_is_in_layer_prop_list
     * @private
     */
    struct __hp_vk_is_in_layer_prop_list { ///< @private
        const char *lay; ///< @private

        explicit __hp_vk_is_in_layer_prop_list(const char *lay); ///< @private

        /**
         * @fun inline bool operator()(::vk::LayerProperties other) const
         * @private
         */
        inline bool operator()(::vk::LayerProperties other) const { ///< @private
            return strcmp(lay, other.layerName) == 0;
        };
    };

    /**
     * @struct vertex
     * @brief Contains vertex data in default `buffer_layout` format.
     * @details This struct contains vertex data in the format specified in the default `buffer_layout`.
     *          Therefore, simply writing an array of this struct to a `vertex_buffer` would be valid if the default `buffer_layout` was bound
     *          at the creation of the active `shader_program`. The format of `vertex` is the following:
     *          `location=0` is a `vec2` representing position, and `location=1` is a `vec3` representing color.
     */
    struct vertex {
        /**
         * @var glm::vec2 pos
         * @brief Position of the vertex in `glm::vec2` format, can be assigned to `{flaot x, float y}`
         */
        glm::vec2 pos;

        /**
         * @var glm::vec3 color
         * @brief Color data of the vertex in `glm::vec3` format, can be assigned to `{float r, float g, float b}`
         */
        glm::vec3 color;
    };

    /**
     * @class buffer_layout
     * @brief Describes the expected layout of the data of a `hp::vk::vertex_buffer`.
     * @details `shader_program`s use the `buffer_layout`s that are bound at the time of construction. If no buffer_layout is bound,
     *          the default layout is used. (If the default layout has not yet been constructed, it would automatically be built with `build_default_layout()`).
     *          The buffer_layout used by a shader_program can be changed, but requires the pipeline to be rebuilt
     *          (There is no dynamic state for vertex input states present in Vulkan).
     *          To change the layout used by a shader program, simply the modify `buffer_layout::bound_lyos`, call `buffer_layout::rebuild_bound_info()`, and call `hp::vk::shader_program::rebuild_pipeline()`.
     * @see hp::vk::shader_program
     */
    class buffer_layout {
    private:
        static buffer_layout default_lyo;  ///< @private
        std::vector<::vk::VertexInputAttributeDescription> attribs; ///< @private
        ::vk::VertexInputBindingDescription binding; ///< @private
        size_t stride = 0; ///< @private
        bool complete = false; ///< @private

        friend class shader_program;

        static std::vector<::vk::VertexInputAttributeDescription> global_attribs;
        static std::vector<::vk::VertexInputBindingDescription> global_bindings;

    public:

        /**
         * @var static std::vector<buffer_layout *> bound_lyos
         * @brief List of `buffer_layouts`s `shader_program`s should support.
         * @warning If you modify this list, `buffer_layout::rebuild_bound_info()` *MUST* be called, and the
         *          graphics pipeline *MUST* be rebuilt with `shader_program::rebuild_pipeline()`!
         */
        static std::vector<buffer_layout *> bound_lyos;


        /**
         * @fn static void rebuild_bound_info()
         * @brief "Compiles" `buffer_layout::bound_lyos` into a format usable by the graphics pipeline.
         * @warning This function *MUST* be called after modifications to `bound_lyos`, and the pipeline *MUST* be rebuilt
         *       with `shader_program::rebuild_pipeline()`.
         */
        static void rebuild_bound_info();


        /**
         * @fn buffer_layout() = default
         * @brief Standard default constructor.
         */
        buffer_layout() = default;

        /**
         * @fn virtual ~buffer_layout()
         * @brief Default virtual destructor for `buffer_layout`s.
         */
        virtual ~buffer_layout() = default;

        /**
         * @fn void push_floats(int num_floats)
         * @brief Add an attribute to the layout with n number of floats.
         * @details Any calls to this function would be ignored after calling `finalize()`.
         * @param num_floats The number of floats in the attribute. Can be between 0 and 4
         */
        void push_floats(int num_floats);

        /**
         * @fn void finalize()
         * @brief Build the binding descriptions and prevent further changes.
         * @details Any call to `push_floats()` after this is ignored.
         */
        void finalize();

        /**
         * @fn static void build_default_layout()
         * @see hp::vk::vertex
         * @see get_default()
         * @brief Build the default layout, accessible via `get_default()`. See `hp::vk::vertex`
         * @details If this function is called when the default layout is already built, then the invocation would be quietly ignored.
         */
        static void build_default_layout();

        /**
         * @fn buffer_layout &operator=(const buffer_layout &)
         * @brief Standard copy assignment operator.
         * @param rhs Right hand side of the assignment operation
         * @return Forwards the `rhs` parameter.
         */
        buffer_layout &operator=(const buffer_layout &rhs);

        /**
         * @fn buffer_layout(const buffer_layout &)
         * @brief Standard copy constructor.
         * @param rhs Right hande side of the copy construction
         */
        buffer_layout(const buffer_layout &rhs);

        /**
         * @fn buffer_layout &operator=(buffer_layout &&) noexcept
         * @brief Standard move assignment operator
         * @param rhs Right hand side of the move assignment
         * @return Forwards the `rhs` parameter
         */
        buffer_layout &operator=(buffer_layout &&rhs) noexcept;

        /**
         * @fn buffer_layout(buffer_layout &&) noexcept
         * @brief Standard move constructor
         * @param rhs Right hand side of the move construction
         */
        buffer_layout(buffer_layout &&rhs) noexcept;

        /**
         * @fn [[nodiscard]] inline bool is_complete() const
         * @brief Check if `finalize()` has been called on this buffer_layout.
         * @return Boolean value. True if `finalize()` has been called, otherwise false.
         */
        [[nodiscard]] inline bool is_complete() const {
            return complete;
        }

        /**
         * @fn static inline buffer_layout *get_default()
         * @see build_default_layout()
         * @see hp::vk::vertex
         * @details Returns the default buffer_layout. See `hp::vk::vertex` for details.
         * @return Returns pointer to the default layout.
         * @warning This function could return and undefined value if `build_default_layout()` has not yet been called!
         */
        static inline buffer_layout *get_default() {
            return &default_lyo;  // Return ptr bc otherwise it gets unbound due to it going out of scope.
        }
    };

    struct queue_family_indices {
    public:
        queue_family_indices() = default;

        std::optional<uint32_t> graphics_fam;

        std::optional<uint32_t> present_fam;

        inline bool is_complete();

        queue_family_indices(const queue_family_indices &rhs);

        queue_family_indices &operator=(const queue_family_indices &rhs);

        queue_family_indices(queue_family_indices &&rhs) noexcept;

        queue_family_indices &operator=(queue_family_indices &&rhs) noexcept;
    };

    struct swap_chain_support {
        ::vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<::vk::SurfaceFormatKHR> formats;
        std::vector<::vk::PresentModeKHR> present_modes;
    };

    static queue_family_indices
    build_queue_fam_indices(::vk::PhysicalDevice *dev, ::vk::SurfaceKHR surf);  ///< @private

    static swap_chain_support get_swap_chain_support(::vk::PhysicalDevice *dev, ::vk::SurfaceKHR surf); ///< @private


    class window;

    class generic_buffer {
    protected:
        size_t capacity = 0; ///< @private
        ::vk::Buffer buf; ///< @private
        window *parent{}; ///< @private
        VmaAllocation allocation{}; ///< @private

        generic_buffer(size_t size, const ::vk::BufferUsageFlags &usage, const ::vk::MemoryPropertyFlags &flags,
                       window *parent); ///< @private

        friend class window;

        friend class shader_program;

        friend struct index_buffer;

    public:
        generic_buffer() = default;

        virtual ~generic_buffer();

        [[nodiscard]] inline size_t get_size() const {
            return capacity;
        }
    };

    extern ::vk::MemoryPropertyFlags memory_local;
    extern ::vk::MemoryPropertyFlags memory_host;

    extern ::vk::BufferUsageFlags vertex_usage;
    extern ::vk::BufferUsageFlags index_usage;
    extern ::vk::BufferUsageFlags vertex_direct_usage;
    extern ::vk::BufferUsageFlags index_direct_usage;

    extern ::vk::BufferUsageFlags staging_usage;
    extern ::vk::BufferUsageFlags vertex_and_index_usage;
    extern ::vk::BufferUsageFlags vertex_and_index_direct_usage;


    struct vertex_buffer {
        generic_buffer *buf{};
        unsigned layout_indx = 0;
        unsigned vertex_count = 0;
        ::vk::DeviceSize offset = 0;
    };

    struct index_buffer {
        generic_buffer *buf{};
        bool is32bit{};
        ::vk::DeviceSize offset = 0;

        [[nodiscard]] inline unsigned get_num_indices() {
            if (is32bit) {
                return (buf->capacity - offset) / sizeof(uint32_t);
            } else {
                return (buf->capacity - offset) / sizeof(uint16_t);
            }
        }
    };

//    class staging_buffer : public generic_buffer {
//    private:
//
//        staging_buffer(size_t size, window *parent); ///< @private
//
//        friend class vertex_buffer;
//
//        friend class window;
//
//    public:
//        staging_buffer() = default;
//
//        void write(const void *data);
//    };
//
//    class vertex_buffer : public generic_buffer {
//    private:
//        friend class window;
//
//        vertex_buffer(size_t size, unsigned num_verts, unsigned lyo_indx, window *parent); ///< @private
//
//    public:
//        unsigned vertex_count = 0;
//        unsigned layout_index = 0;
//
//        vertex_buffer() = default;
//
//        std::pair<::vk::Fence *, ::vk::CommandBuffer> write(staging_buffer *staging_buf, bool wait = true);
//    };
//
//    class index_buffer : public generic_buffer {
//    private:
//        friend class window;
//
//        bool is32bit{};
//
//        index_buffer(size_t size, bool is32bit, window *parent); ///< @private
//
//    public:
//        index_buffer() = default;
//
//        std::pair<::vk::Fence *, ::vk::CommandBuffer> write(staging_buffer *staging_buf, bool wait = true);
//
//        [[nodiscard]] inline unsigned get_num_indices() {
//            if (is32bit) {
//                return capacity / sizeof(uint32_t);
//            } else {
//                return capacity / sizeof(uint16_t);
//            }
//        }
//    };

    /**
     * @class shader_program
     * @brief An abstraction of graphics pipelines (aka `vk::Pipeline` objects). See hp::vk::window::new_shader_program
     * @details `shader_program`s use the `buffer_layout`s that are bound at the time of construction. If no buffer_layout is bound,
     *          the default layout is used. (If the default layout has not yet been constructed, it would automatically be built with `build_default_layout()`).
     *          The buffer_layout used by a shader_program can be changed, but requires the pipeline to be rebuilt
     *          (There is no dynamic state for vertex input states present in Vulkan).
     *          To change the layout used by a shader program, simply the modify `buffer_layout::bound_lyos`, call `buffer_layout::rebuild_bound_info()`, and call `hp::vk::shader_program::rebuild_pipeline()`.
     * @see hp::vk::window::new_shader_program
     * @see hp::vk::buffer_layout
     */
    class shader_program {
    private:
        ::hp::vk::window *parent{}; ///< @private
        std::vector<::vk::PipelineShaderStageCreateInfo> stage_cis; ///< @private

        /**
         * @var std::queue<const char *> entrypoint_keepalives
         * @private
         * @details The entrypoint string's `c_str` is passed to the create info, and when the pipeline needs to be rebuilt,
         *          The string has gone out of scope, and the pointer points to unallocated memory. This prevents that.
         */
        std::queue<const char *> entrypoint_keepalives; ///< @private

        ::vk::PipelineLayout pipeline_layout;  ///< @private
        ::vk::Pipeline pipeline; ///< @private
        std::queue<::vk::ShaderModule> mods; ///< @private

        std::string fp; ///< @private
        const char *metapath{}; ///< @private

        friend class ::hp::vk::window;

        shader_program(const std::string &basicString, const char *string, ::hp::vk::window *pWindow); ///< @private

    public:
        virtual ~shader_program();

        shader_program() = default;

        shader_program &operator=(const shader_program &rhs) = delete;

        shader_program(const shader_program &rhs) = delete;

        shader_program &operator=(shader_program &&rhs) noexcept;

        shader_program(shader_program &&rhs) noexcept;

        void reload_from_file();

        void rebuild_pipeline();
    };

    static void on_resize_event(GLFWwindow *win, int width, int height); ///< @private

    static void on_iconify_event(GLFWwindow *win, int state); ///< @private

    class window {
    private:
        GLFWwindow *win{}; ///< @private
        ::vk::SurfaceKHR surf; ///< @private
        size_t current_frame = 0; ///< @private

        bool uses_validation_layers{}; ///< @private
        ::vk::Instance inst; ///< @private
        VkDebugUtilsMessengerEXT debug_msgr{}; ///< @private
        std::vector<::vk::ExtensionProperties> supported_ext; ///< @private
        std::vector<::vk::LayerProperties> supported_lay; ///< @private
        std::vector<::vk::ExtensionProperties> phys_dev_ext; ///< @private

        ::vk::PhysicalDevice *phys_dev{}; ///< @private
        ::vk::PhysicalDeviceMemoryProperties mem_props; ///< @private
        ::vk::Device log_dev; ///< @private
        std::multimap<float, ::vk::PhysicalDevice> devices; ///< @private

        ::vk::Queue graphics_queue; ///< @private
        ::vk::Queue present_queue; ///< @private

        ::vk::SwapchainKHR swap_chain; ///< @private
        ::vk::Extent2D swap_extent; ///< @private
        ::vk::Format swap_fmt{}; ///< @private
        std::vector<::vk::Image> swap_imgs; ///< @private
        std::vector<::vk::ImageView> swap_views; ///< @private
        std::vector<::vk::Framebuffer> framebuffers; ///< @private

        ::vk::CommandPool cmd_pool; ///< @private
        ::vk::RenderPass render_pass; ///< @private
        std::vector<::vk::CommandBuffer> cmd_bufs; ///< @private

        VmaAllocator allocator{}; ///< @private

        queue_family_indices queue_fam_indices; ///< @private

        std::vector<generic_buffer *> child_bufs; ///< @private
        std::vector<::hp::vk::shader_program *> child_shaders; ///< @private

        std::vector<::vk::Semaphore> img_avail_sms; ///< @private
        std::vector<::vk::Semaphore> rend_fin_sms; ///< @private
        std::vector<::vk::Fence> flight_fences; ///< @private
        std::vector<::vk::Fence> img_fences; ///< @private

        std::vector<::vk::Fence> child_fences;

        std::vector<std::function<void(::vk::CommandBuffer, window * )>> record_buffer; ///< @private
        mutable std::recursive_mutex render_mtx; ///< @private

        void (*swap_recreate_callback)(::vk::Extent2D) = nullptr; ///< @private

        bool swapchain_recreate_event = false; ///< @private

        static VKAPI_ATTR ::vk::Bool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                                VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                                const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                                void *pUserData); ///< @private

        std::vector<::vk::CommandBuffer> get_cmd_bufs(std::vector<::vk::Framebuffer> *frame_bufs,
                                                      ::vk::RenderPass *rend_pass, ::vk::Extent2D *extent,
                                                      ::vk::CommandPool *use_cmd_pool); ///< @private

        ::vk::Result createDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                                  const VkAllocationCallbacks *pAllocator,
                                                  VkDebugUtilsMessengerEXT *pDebugMessenger); ///< @private

        void
        destroyDebugUtilsMessengerEXT(VkDebugUtilsMessengerEXT debugMessenger,
                                      const VkAllocationCallbacks *pAllocator); ///< @private

        void create_swapchain(bool do_destroy = false); ///< @private

        void recreate_swapchain(); ///< @private

        friend class shader_program;

        friend class generic_buffer;

        friend class vertex_buffer;

        friend class staging_buffer;

        friend void on_resize_event(GLFWwindow *win, int width, int height); ///< @private

        friend void on_iconify_event(GLFWwindow *win, int state); ///< @private

    public:
        window() = default;

        window(int width, int height, const char *app_name, uint32_t version);

        virtual ~window();

        window(const window &other) = delete;

        window &operator=(const window &rhs) = delete;

        window(window &&other) noexcept;

        window &operator=(window &&other) noexcept;

        inline bool ext_supported(const char *ext) {
            return std::any_of(supported_ext.begin(), supported_ext.end(), __hp_vk_is_in_extension_prop_list(ext));
        };

        inline bool dev_ext_supported(const char *ext) {
            return std::any_of(phys_dev_ext.begin(), phys_dev_ext.end(), __hp_vk_is_in_extension_prop_list(ext));
        };

        inline bool layer_supported(const char *lay) {
            return std::any_of(supported_lay.begin(), supported_lay.end(), __hp_vk_is_in_layer_prop_list(lay));
        };

        inline bool should_close() {
            return glfwWindowShouldClose(win);
        };

        inline ::vk::Extent2D get_dims() {
            return swap_extent;
        }

        std::pair<::vk::Fence *, ::vk::CommandBuffer> copy_buffer(generic_buffer *source, generic_buffer *dest,
                                                                  bool wait = true, size_t src_offset = 0,
                                                                  size_t dest_offset = 0, size_t size = 0);

        void write_buffer(generic_buffer *buf, const void *data, size_t offset = 0, size_t size = 0);

        uint8_t *start_write(generic_buffer *buf);

        void write_buffer(uint8_t *dest, generic_buffer *buf, const void *src, size_t offset = 0, size_t size = 0);

        void stop_write(generic_buffer *buf);

        /**
         * @fn inline void set_swap_recreate_callback(void(*)(::vk::Extent2D))
         * @brief Set the callback that is called whenever the swapchain needs to be recreated.
         * @details Use this callback to re-record the command buffers if you wish.
         *          (The viewport and scissor would've changed, so rerecording would be necessary)
         * @param new_callback The new callback to replace the old one.
         * @warning DO NOT CALL `save_recording()` IN THE CALLBACK! SWAP CHAIN RECREATION ALREADY IMPLICITLY CALLS IT!
         */
        inline void set_swap_recreate_callback(void(*new_callback)(::vk::Extent2D)) {
            swap_recreate_callback = new_callback;
        }

        void clear_recording();

        void save_recording();

        void rec_bind_shader(shader_program *shader);

        void rec_set_viewport(::vk::Viewport viewport);

        void rec_set_scissor(::vk::Rect2D scissor);

        void rec_set_default_viewport();

        void rec_set_default_scissor();

        void rec_bind_vbo(vertex_buffer vbo);

        void rec_bind_index_buffer(index_buffer ibo);

        void rec_draw_indexed(uint32_t num_indices);

        void rec_draw(unsigned num_verts);

        /**
         * @fn inline shader_program *new_shader_program(const std::string &, const char *metapath = "/shader_metadat.txt")
         * @brief Construct and retrieve a new `hp::vk::shader_program`
         * @details Upon construction, the file at `"fp + metapath"` will be loaded and read. The expected contents of each line of the file are:
         *          `"[fragment-shader|vertex-shader];[entrypoint]: [filename]"` (Example: `"vertex-shader;main: vert.spv"`
         *          would load `fp + "/vert.spv"` as the vertex shader with entrypoint "main"). Whitespace is ignored and
         *          comments are made with the `"#"` character. Comments at the end of lines are *NOT* supported.
         *          The line `"fragment-shader;main: frag.spv  # <some comment>"` would be invalid.
         *          Further examples are available under the "Examples" tag of the documentation.
         *
         * @warning DO NOT attempt to call `delete` on pointer returned by this function! They are cleaned up when window is destroyed!
         * @param fp The path to load the shader program from. *DO NOT* INCLUDE A TRAILING SLASH OR THE LOADING OPERATION *WILL* FAIL
         * @param metapath The path to the file containing the metadata for the shader. A LEADING SLASH *MUST* BE INCLUDED OR THE LOADING OPERATION *WILL* FAIL.
         * @return A pointer to the newly constructed `shader_program`.
         */
        inline shader_program *new_shader_program(const std::string &fp, const char *metapath = "/shader_metadat.txt") {
            auto new_prog = new shader_program(fp, metapath, this);
            child_shaders.emplace_back(new_prog);
            return new_prog;
        };

        inline void delete_shader_program(shader_program *sh) {
            boost::remove_erase(child_shaders, sh);
            delete sh;
        }

        inline ::vk::Fence *new_fence() {
            child_fences.emplace_back(::vk::Fence());
            return &child_fences.at(child_fences.size() - 1);
        }

        inline void delete_fence(::vk::Fence *fence) {
            boost::remove_erase(child_fences, *fence);
            log_dev.destroyFence(*fence, nullptr);
        }

        inline generic_buffer *
        new_buffer(size_t size, const ::vk::BufferUsageFlags &usage, const ::vk::MemoryPropertyFlags &flags) {
            auto buf = new generic_buffer(size, usage, flags, this);
            child_bufs.emplace_back(buf);
            return buf;
        }

        inline void delete_buffer(generic_buffer *buf) {
            boost::remove_erase(child_bufs, buf);
            delete buf;
        }

        void draw_frame();
    };
}


#endif //__HEPHAESTUS_VK_WINDOW_HPP
