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
#include <optional>
#include "vk_mem_alloc.h"

#include "boost/range/algorithm_ext/erase.hpp"

namespace hp::vk {

    /**
     * @struct __hp_vk_is_in_extension_prop_list
     * @private
     * @brief This is an implementation detail and you should NEVER touch this.
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
     * @brief This is an implementation detail and you should NEVER touch this.
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
        size_t seek_val = 0; ///< @private
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
         * @brief Deleted copy assignment operator. Use the move assignment operator instead.
         * @note The standard copy assignment operator has been deleted. You *MUST* use the `buffer_layout &operator=(buffer_layout &&rhs) noexcept`
         *       move assignment operator instead.
         */
        buffer_layout &operator=(const buffer_layout &rhs) = delete;

        /**
         * @fn buffer_layout(const buffer_layout &)
         * @brief Deleted copy constructor. Use the move constructor instead.
         * @note The standard copy constructor has been deleted. You *MUST* use the `buffer_layout(buffer_layout &&rhs) noexcept`
         *       move constructor instead.
         */
        buffer_layout(const buffer_layout &rhs) = delete;

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

        /**
         * @fn inline void seek(size_t n_seek)
         * @brief Seek the next location to output to `n_seek`.
         * @details Makes it so the next attribute in `push_float` has location of `n_seek`. Each next `push_float` would be
         *          the previous one plus one, unless seek is called again.
         * @param n_seek Location for next attrib.
         */
        inline void seek(size_t n_seek) {
            seek_val = n_seek;
        }
    };

    class ubo_layout {
    private:
        ::vk::DescriptorSetLayout desc_lyo;
        std::vector<::vk::DescriptorSetLayoutBinding> bindings;

        static std::vector<::vk::DescriptorSetLayout> lyos;

    public:
        static std::vector<ubo_layout *> bound_lyos;

        static void rebuild_bound_info();

        ubo_layout();

        virtual ~ubo_layout();

        void push_binding();

        ubo_layout(const ubo_layout &) = delete;

        ubo_layout &operator=(const ubo_layout &) = delete;

        ubo_layout(ubo_layout &&) noexcept;

        ubo_layout &operator=(ubo_layout &&) noexcept;

    };

    /**
     * @struct queue_family_indices
     * @private
     * @brief This is an implementation detail and you should NEVER touch this.
     */
    struct queue_family_indices { ///< @private
    public:
        queue_family_indices() = default; ///< @private

        std::optional<uint32_t> graphics_fam; ///< @private

        std::optional<uint32_t> present_fam; ///< @private

        inline bool is_complete(); ///< @private

        queue_family_indices(const queue_family_indices &rhs); ///< @private

        queue_family_indices &operator=(const queue_family_indices &rhs); ///< @private

        queue_family_indices(queue_family_indices &&rhs) noexcept; ///< @private

        queue_family_indices &operator=(queue_family_indices &&rhs) noexcept; ///< @private
    };

    /**
     * @struct swap_chain_support
     * @private
     * @brief This is an implementation detail and you should NEVER touch this.
     */
    struct swap_chain_support { ///< @private
        ::vk::SurfaceCapabilitiesKHR capabilities; ///< @private
        std::vector<::vk::SurfaceFormatKHR> formats; ///< @private
        std::vector<::vk::PresentModeKHR> present_modes; ///< @private
    };

    static queue_family_indices
    build_queue_fam_indices(::vk::PhysicalDevice *dev, ::vk::SurfaceKHR surf);  ///< @private

    static swap_chain_support get_swap_chain_support(::vk::PhysicalDevice *dev, ::vk::SurfaceKHR surf); ///< @private


    class window;

    /**
     * @class generic_buffer
     * @brief Offers nice wrapper around functionality of vk::Buffer (See Vulkan documentation)
     */
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

        friend class vertex_bind_info;

    public:
        /**
         * @fn generic_buffer() = default
         * @brief Standard default constructor
         */
        generic_buffer() = default;

        /**
         * @fn virtual ~generic_buffer()
         * @brief Standard virtual destructor
         */
        virtual ~generic_buffer();

        /**
         * @fn generic_buffer(const generic_buffer &cpy) = delete
         * @brief Deleted copy constructor. Use the move constructor instead.
         * @note The standard copy constructor has been deleted. You *MUST* use the `generic_buffer(generic_buffer &&rhs) noexcept`
         *       standard move constructor instead.
         */
        generic_buffer(const generic_buffer &cpy) = delete;

        /**
         * @fn generic_buffer &operator=(const generic_buffer &cpy) = delete
         * @brief Deleted copy assignment operator. Use the move assignment operator instead.
         * @note The standard copy assignment operator has been deleted. You *MUST* use the `generic_buffer &operator=(generic_buffer &&rhs) noexcept`
         *       standard move assignment operator instead.
         */
        generic_buffer &operator=(const generic_buffer &cpy) = delete;

        /**
         * @fn generic_buffer(generic_buffer &&rhs) noexcept
         * @brief Standard move constructor
         * @param rhs Right hand side of the copy construction
         */
        generic_buffer(generic_buffer &&rhs) noexcept;

        /**
         * @fn generic_buffer &operator=(generic_buffer &&rhs) noexcept
         * @brief Standard move assignment operator.
         * @param rhs Right hand side of the move assignment operation
         * @return Forwards the `rhs` parameter.
         */
        generic_buffer &operator=(generic_buffer &&rhs) noexcept;

        /**
         * @fn void flush(::vk::DeviceSize offset = 0, ::vk::DeviceSize size = VK_WHOLE_SIZE)
         * @brief Flush memory so writes are visible to GPU. Writing *MUST* be active.
         * @details Consult vulkan docs for `vkFlushMappedMemoryRanges`
         * @see invalidate()  generic_buffer::start_write()  generic_buffer::stop_write()
         * @param offset Offset (in bytes) of the range of memory to flush (0 by default)
         * @param size Size (in bytes) of the region to flush (Uses entire allocation by default)
         */
        void flush(::vk::DeviceSize offset = 0, ::vk::DeviceSize size = VK_WHOLE_SIZE);

        /**
         * @fn void invalidate(::vk::DeviceSize offset = 0, ::vk::DeviceSize size = VK_WHOLE_SIZE)
         * @brief Invalidate memory so GPU writes are visible to CPU. Writing *MUST* be active.
         * @details Consult vulkan docs for `vkInvalidateMappedMemoryRanges`
         * @see invalidate()  generic_buffer::start_write()  generic_buffer::stop_write()
         * @param offset Offset (in bytes) of the range of memory to invalidate (0 by default)
         * @param size Size (in bytes) of the region to invalidate (Uses entire allocation by default)
         */
        void invalidate(::vk::DeviceSize offset = 0, ::vk::DeviceSize size = VK_WHOLE_SIZE);

        /**
         * @fn void write_buffer(const void *data, size_t offset = 0, size_t size = 0)
         * @brief Write data to a buffer that is `eHostVisible`.
         * @details This function maps and unmaps the buffer. See `void write_buffer(uint8_t *dest, generic_buffer *buf, const void *src, size_t offset = 0, size_t size = 0)`,
         *          `generic_buffer::start_write()`, and `generic_buffer::stop_write()` for explicit mapping/unmapping.
         * @note This buffer *MUST* have the memory property `eHostVisible`! Consult vulkan docs.
         * @param data The data to write
         * @param offset The index (in bytes) of the buffer at which to start writing.
         * @param size The size of the data to write.
         */
        void write_buffer(const void *data, size_t offset = 0, size_t size = 0);

        /**
         * @fn uint8_t *start_write()
         * @brief Map a buffer so it is ready for writing.
         * @warning Mapped memory *IS NOT* automatically unmapped! Any call to `start_write()` *MUST* be accompanied
         *          by a call to `stop_write()`
         * @return Pointer to the mapped region
         */
        uint8_t *start_write();

        /**
         * @fn void write_buffer(uint8_t *dest, const void *src, size_t offset = 0, size_t size = 0)
         * @brief Write data to a buffer with `eHostCoherent` and `eHostVisible`.
         * @details This function *DOES NOT* map and unmap memory! Memory must be explicitly mapped/unmapped using
         *          `start_write()` and `stop_write()`.
         * @param dest The data to write to. Should be the value returned from `hp::vk::generic_buffer::start_write()`
         * @param src The data to write.
         * @param offset The index (in bytes) at which to start writing.
         * @param size Size of the data to write.
         */
        void write_buffer(uint8_t *dest, const void *src, size_t offset = 0, size_t size = 0);

        /**
         * @fn void stop_write()
         * @brief Unmap a buffer so it is ready for reading and use.
         */
        void stop_write();


        /**
         * @fn [[nodiscard]] inline size_t get_size() const
         * @brief Query the capacity of the generic_buffer in bytes.
         * @return The size of the buffer in bytes.
         */
        [[nodiscard]] inline size_t get_size() const {
            return capacity;
        }
    };

    /**
     * @struct vertex_buffer
     * @brief Provide metadata necessary to bind and draw a VBO
     */
    struct vertex_buffer {
        /**
         * @var generic_buffer *buf
         * @brief The underlying `generic_buffer` that this metadata describes (and is to be used as a VBO)
         */
        generic_buffer *buf{};

        /**
         * @var unsigned vertex_count
         * @brief The number of vertices to draw
         */
        unsigned vertex_count = 0;

        /**
         * @var ::vk::DeviceSize offset
         * @brief The index at which to start drawing vertices (in bytes)
         */
        ::vk::DeviceSize offset = 0;
    };

    /**
     * @struct index_buffer
     * @brief Provide the metadata necessary to bind and draw an IBO
     */
    struct index_buffer {
        /**
         * @var generic_buffer *buf{}
         * @brief The underlying `generic_buffer` that this metadata describes (and is to be used as a IBO)
         */
        generic_buffer *buf{};

        /**
         * @var bool is32bit
         * @brief True if each index entry in the buffer is 32 bits. (ie `uint32_t`). False if they are 16 bits (ie `uint16_t`)
         */
        bool is32bit{};

        /**
         * @var ::vk::DeviceSize offset
         * @brief The index at which to start drawing indices (in bytes)
         */
        ::vk::DeviceSize offset = 0;

        /**
         * @fn [[nodiscard]] inline ::vk::DeviceSize get_num_indices()
         * @brief Get the number of indices this IBO is supposed to hold.
         */
        [[nodiscard]] inline ::vk::DeviceSize get_num_indices() {
            if (is32bit) {
                return (buf->capacity - offset) / sizeof(uint32_t);
            } else {
                return (buf->capacity - offset) / sizeof(uint16_t);
            }
        }
    };

    class vertex_bind_info {
    private:
        uint32_t n_vbos;
        ::vk::Buffer *vbos = nullptr;
        ::vk::DeviceSize *offsets = nullptr;

        friend class window;

    public:
        vertex_bind_info() = default;

        vertex_bind_info(vertex_buffer *vbos, uint32_t num_vbos);

        virtual ~vertex_bind_info();

        vertex_bind_info &operator=(const vertex_bind_info &) = delete;

        vertex_bind_info(const vertex_bind_info &) = delete;

        vertex_bind_info &operator=(vertex_bind_info &&rhs) noexcept;

        vertex_bind_info(vertex_bind_info &&rhs) noexcept;

    };

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
        /**
         * @fn virtual ~shader_program()
         * @brief Standard virtual destructor.
         */
        virtual ~shader_program();

        /**
         * @fn shader_program() = default
         * @brief Standard default constructor.
         */
        shader_program() = default;

        /**
         * @fn shader_program &operator=(const shader_program &) = delete
         * @brief Deleted copy assignment operator. Use the move assignment operator instead.
         * @note The standard copy assignment operator has been deleted. You *MUST* use the `shader_program &operator=(shader_program &&rhs) noexcept`
         *       move assignment operator instead.
         */
        shader_program &operator=(const shader_program &rhs) = delete;

        /**
         * @fn shader_program(const shader_program &rhs) = delete
         * @brief Deleted copy constructor. Use the move constructor instead.
         * @note The standard copy constructor has been deleted. You *MUST* use the `shader_program(shader_program &&rhs) noexcept`
         *       move constructor instead.
         */
        shader_program(const shader_program &rhs) = delete;

        /**
         * @fn shader_program &operator=(shader_program &&rhs) noexcept
         * @brief Standard move assignment operator
         * @param rhs Right hand side of the move assignment
         * @return Forwards the `rhs` parameter.
         */
        shader_program &operator=(shader_program &&rhs) noexcept;

        /**
         * @fn shader_program(shader_program &&rhs) noexcept
         * @brief Standard move constructor
         * @param rhs Right hand side of the move construction
         */
        shader_program(shader_program &&rhs) noexcept;

        /**
         * @fn void reload_from_file()
         * @brief Fully reload the `shader_program`, including re-reading the file. See `rebuild_pipeline()`.
         * @note This function implicitly calls `rebuild_pipeline()` so there is no need to explicitly call it.
         */
        void reload_from_file();

        /**
         * @fn void rebuild_pipeline()
         * @brief Rebuild the graphics pipeline, does *NOT* re-read the file. See `reload_from_file()`.
         */
        void rebuild_pipeline();
    };

    static void on_resize_event(GLFWwindow *win, int width, int height); ///< @private

    static void on_iconify_event(GLFWwindow *win, int state); ///< @private

    /**
     * @class window
     * @brief Describes a vulkan window. Is used as a base for all operations.
     */
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
        ::vk::SurfaceFormatKHR swap_fmt{}; ///< @private
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

        void record_cmd_bufs(std::vector<::vk::Framebuffer> *frame_bufs,
                             ::vk::RenderPass *rend_pass, ::vk::Extent2D *extent); ///< @private

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

        friend struct vertex_buffer;

        friend void on_resize_event(GLFWwindow *win, int width, int height); ///< @private

        friend void on_iconify_event(GLFWwindow *win, int state); ///< @private

    public:
        /**
         * @fn window() = default
         * @brief Standard default constructor.
         */
        window() = default;

        /**
         * @fn window(int width, int height, const char *app_name, uint32_t version)
         * @brief Window constructor.
         * @param width Width of the window in pixels
         * @param height Height of the window in pixels
         * @param app_name The name of you application
         * @param version Version of you application, should be a return value of `VK_MAKE_VERSION()`. See vulkan documentation for more details.
         */
        window(int width, int height, const char *app_name, uint32_t version);

        /**
         * @fn virtual ~window()
         * @brief Standard virtual destructor.
         */
        virtual ~window();

        /**
         * @fn window(const window &other) = delete
         * @brief Deleted copy constructor. Use the move constructor instead.
         * @note The standard copy constructor is deleted. You *MUST* use the `window(window &&other) noexcept` move
         *        constructor instead!
         */
        window(const window &other) = delete;

        /**
         * @fn window &operator=(const window &rhs) = delete
         * @brief Deleted copy assignment operator. Use the move assignment operator instead.
         * @note The standard copy assignment operator is deleted. You *MUST* use the `window &operator=(window &&other) noexcept`
         *       move assignment operator instead.
         */
        window &operator=(const window &rhs) = delete;

        /**
         * @fn window(window &&other) noexcept
         * @brief Standard copy constructor
         * @param other Right hand side of the copy construction
         */
        window(window &&other) noexcept;

        /**
         * @fn window &operator=(window &&other) noexcept
         * @brief Standard move assignment operator
         * @param other Right hand side of the move assignment operation.
         * @return Forwards the `other` parameter.
         */
        window &operator=(window &&other) noexcept;

        /**
         * @fn inline bool ext_supported(const char *ext)
         * @brief Query if an extension is supported by the current instance.
         * @param ext Name of the extension to check.
         * @return True if extension is supported, otherwise false.
         */
        inline bool ext_supported(const char *ext) {
            return std::any_of(supported_ext.begin(), supported_ext.end(), __hp_vk_is_in_extension_prop_list(ext));
        };

        /**
         * @fn inline bool dev_ext_supported(const char *ext)
         * @brief Query if an extension is supported by the current physical device.
         * @param ext Name of the extension to check.
         * @return True if the extension is supported, otherwise false.
         */
        inline bool dev_ext_supported(const char *ext) {
            return std::any_of(phys_dev_ext.begin(), phys_dev_ext.end(), __hp_vk_is_in_extension_prop_list(ext));
        };

        /**
         * @fn inline bool layer_supported(const char *lay)
         * @brief Check if a validation layer is supported by the current instance.
         * @note Disabling validation layers *WILL NOT* affect the behavior of this function.
         * @param lay The name of the validation layer to check.
         * @return True if the validation layer is supported, otherwise false.
         */
        inline bool layer_supported(const char *lay) {
            return std::any_of(supported_lay.begin(), supported_lay.end(), __hp_vk_is_in_layer_prop_list(lay));
        };

        /**
         * @fn inline bool should_close()
         * @brief Poll if this window should be closed (ie. The user closed it)
         * @return True if the window should close (ie. You should terminate the program, or other appropriate behavior)
         */
        inline bool should_close() {
            return glfwWindowShouldClose(win);
        };

        /**
         * @fn inline ::vk::Extent2D get_dims()
         * @brief Get the current size of the window (in pixels).
         * @return The vulkan `Extent2D` assiciated with this window.
         */
        inline ::vk::Extent2D get_dims() {
            return swap_extent;
        }

        /**
         * @fn std::pair<::vk::Fence, ::vk::CommandBuffer> copy_buffer(generic_buffer *source, generic_buffer *dest,
         *                                                        bool wait = true, size_t src_offset = 0,
         *                                                        size_t dest_offset = 0, size_t size = 0)
         * @brief Copy data from one buffer to another
         * @see window::delete_fence  window::delete_cmd_buffers  window::wait_fences
         * @param source The source buffer of the copy operation
         * @param dest The destination buffer of the copy operation
         * @param wait Whether or not to wait for the copy operation to finish. See the return values for the function.
         * @param src_offset Index (in bytes) in the source at which to start copying data.
         * @param dest_offset Index (in bytes) in the destination at which to start writing data.
         * @param size The size (in bytes) of the data that should be copied. If set to 0, the entire sizes of the
         *              buffers are automatically copied (ie. size is set to the size of the buffers, so the sizes *MUST* be equal)
         * @return Returns a vk::Fence and vk::CommandBuffer if the `wait` parameter is set to `false`.
         *          These objects *MUST* be *EXPLICITLY DESTROYED!* If `wait` is `false` or the sizes of the buffers are mismatched
         *         while `size` is 0, then this function would return `VK_NULL_HANDLE`s.
         * @warning It is possible that the fence would never be signaled if the operation fails, so waiting for
         *          them may cause infinite blocking. Therefore, it is recommended that you set a timeout for fence waits.
         */
        std::pair<::vk::Fence, ::vk::CommandBuffer> copy_buffer(generic_buffer *source, generic_buffer *dest,
                                                                bool wait = true, size_t src_offset = 0,
                                                                size_t dest_offset = 0, size_t size = 0);

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

        /**
         * @fn void clear_recording()
         * @brief Clear the recording buffer.
         */
        void clear_recording();

        /**
         * @fn void save_recording()
         * @brief Create new command buffers according to the contents of the recording buffer.
         */
        void save_recording();

        /**
         * @fn void rec_bind_shader(shader_program *shader)
         * @brief Add a pipeline binding operation to the recording buffer.
         * @details Call this function before setting viewports, scissors, and other dynamic states.
         *          A pipeline must be bound for *ANY* draw operation.
         * @param shader Shader to bind.
         */
        void rec_bind_shader(shader_program *shader);

        /**
         * @fn void rec_set_viewport(::vk::Viewport viewport)
         * @brief Record setting the viewport. Consult vulkan docs.
         * @param viewport New viewport
         */
        void rec_set_viewport(::vk::Viewport viewport);

        /**
         * @fn void rec_set_scissor(::vk::Rect2D scissor)
         * @brief Record setting the scissor. Consult vulkan docs.
         * @param scissor New scissor
         */
        void rec_set_scissor(::vk::Rect2D scissor);

        /**
         * @fn void rec_set_default_viewport()
         * @brief Record setting the default viewport, which covers the entire screen.
         */
        void rec_set_default_viewport();

        /**
         * @fn void rec_set_default_scissor()
         * @brief Record setting the default scissor, which covers the entire screen.
         */
        void rec_set_default_scissor();

        /**
         * @fn void rec_bind_vbos(vertex_buffer *vbo)
         * @brief Record binding a *single* vertex buffer object.
         * @param vbo The vertex buffer to bind
         */
        void rec_bind_vbos(vertex_buffer *vbo);

        /**
         * @fn void rec_bind_vbos(vertex_bind_info *bi, uint32_t start = 0)
         * @brief Record binding a list of vertex buffer objects.
         * @details See documentation for `vertex_bind_info` and vulkan docs for `vkCmdBindVertexBuffers`
         * @param bi Pointer to `vertex_bind_info`. See do
         * @param start Index of the first VBO to bind in the list
         */
        void rec_bind_vbos(vertex_bind_info *bi, uint32_t start = 0);

        /**
         * @fn void rec_bind_index_buffer(index_buffer ibo)
         * @brief Record binding an index buffer
         * @param ibo Index buffer to bind.
         */
        void rec_bind_index_buffer(index_buffer ibo);

        /**
         * @fn void rec_draw_indexed(::vk::DeviceSize num_indices)
         * @brief Record issuing a draw command using the index buffer.
         * @param num_indices Number of indices from the index buffer to draw.
         */
        void rec_draw_indexed(::vk::DeviceSize num_indices);

        /**
         * @fn void rec_draw(unsigned num_verts)
         * @brief Record issuing a standard draw command.
         * @param num_verts Number of vertices to draw.
         */
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
         * @warning DO NOT attempt to call `delete` on pointer returned by this function! Use `window::delete_shader_program()` instead!
         *          It is also *NOT* necessary to call `delete_shader_program`; the shaders are automatically cleaned up when window is destroyed!
         * @param fp The path to load the shader program from. *DO NOT* INCLUDE A TRAILING SLASH OR THE LOADING OPERATION *WILL* FAIL
         * @param metapath The path to the file containing the metadata for the shader. A LEADING SLASH *MUST* BE INCLUDED OR THE LOADING OPERATION *WILL* FAIL.
         * @return A pointer to the newly constructed `shader_program`.
         */
        inline shader_program *new_shader_program(const std::string &fp, const char *metapath = "/shader_metadat.txt") {
            auto new_prog = new shader_program(fp, metapath, this);
            child_shaders.emplace_back(new_prog);
            return new_prog;
        };

        /**
         * @fn inline void delete_shader_program(shader_program *sh)
         * @brief Destroy a shader_program associated with this window
         * @note The supplied shader_program *DOES NOT* have to be associate with this window, and it *DOES NOT* have to be
         *       created by `window::new_shader_program`.
         * @param sh Pointer to the shader program to destroy
         */
        inline void delete_shader_program(shader_program *sh) {
            boost::remove_erase(child_shaders, sh);
            delete sh;
        }

        /**
         * @fn inline void delete_cmd_buffers(::vk::CommandBuffer *bufs, uint32_t num = 1)
         * @brief Destroy any command buffer associated with this window allocated to the current `cmd_pool`
         * @note The command buffers supplied to this function *MUST* be associated with the current window
         *       and command pool! As of right now, the command pool is *NEVER* recreated.
         * @param bufs Pointer to a list of command buffers to destroy
         * @param num Number of command buffers the list contains
         */
        inline void delete_cmd_buffers(::vk::CommandBuffer *bufs, uint32_t num = 1) {
            log_dev.freeCommandBuffers(cmd_pool, num, bufs);
        }

        /**
         * @fn inline void wait_fences(::vk::Fence *fences, uint32_t num = 1, uint64_t timeout = UINT64_MAX)
         * @brief Block until the fences provided are signaled
         * @note The fence provided *MUST* be associated with this window!
         * @param fences Pointer to a list of fences
         * @param num The number of fences that list contains (default to 1 for waiting on a single fence)
         * @param timeout The maximum amount of time (in nanoseconds) this would block before aborting.
         *                By default there is no timeout (Signified by the value `UINT64_MAX`).
         * @warning If no timeout is set, this function would block forever if the fence is never signaled,
         *          which *CAN* happen! Therefore, it is recommended to define a timeout. Do not set it too low,
         *          or the operation wouldn't have a change to complete. Don't set it too high, or
         *          the application would be forced to wait that number of nanoseconds if the fence is never signaled.
         */
        inline void wait_fences(::vk::Fence *fences, uint32_t num = 1, uint64_t timeout = UINT64_MAX) {
            log_dev.waitForFences(num, fences, ::vk::Bool32(VK_TRUE), timeout);
        }

        /**
         * @fn inline ::vk::Fence new_fence()
         * @brief Construct and retrieve a new vk::Fence. See vulkan documentation for more details.
         * @warning DO NOT attempt to call `delete` on pointer returned by this function! Use `window::delete_fence()` instead!
         *          It is also *NOT* necessary to call `delete_fence`; the fences are automatically cleaned up when window is destroyed!
         * @return The newly constructed vk::Fence
         */
        inline ::vk::Fence new_fence() {
            ::vk::Fence ret = ::vk::Fence();
            ::vk::FenceCreateInfo fence_ci((::vk::FenceCreateFlags()));
            if (handle_res(log_dev.createFence(&fence_ci, nullptr, &ret), HP_GET_CODE_LOC) != ::vk::Result::eSuccess) {
                HP_FATAL("Failed to create fences!");
            }
            child_fences.emplace_back(ret);
            return ret;
        }

        /**
         * @fn inline void delete_fence(::vk::Fence fence)
         * @brief Destroy a fence associated with this window
         * @note The fence provided *MUST* be associated with this window, but it *DOES NOT* have to be created
         *       with `window::new_fence()`.
         * @param fence Fence to destroy
         */
        inline void delete_fence(::vk::Fence fence) {
            log_dev.destroyFence(fence, nullptr);
            boost::remove_erase(child_fences, fence);
        }

        /**
         * @fn inline generic_buffer *new_buffer(size_t size, const ::vk::BufferUsageFlags &usage, const ::vk::MemoryPropertyFlags &flags)
         * @brief Construct and retrieve a new generic_buffer
         * @warning DO NOT attempt to call `delete` on pointer returned by this function! Use `window::delete_buffer()` instead!
         *          It is also *NOT* necessary to call `delete_buffer`; the buffers are automatically cleaned up when window is destroyed!
         * @param size The size (in bytes) of the buffer to create.
         * @param usage See `hp::vk::vertex_usage`, `hp::vk::index_usage`, etc. Consult Vulkan docs for vk::BufferUsageFlags.
         * @param flags See `hp::vk::memory_local` and `hp::vk::memory_host`. Consult Vulkan docs for vk::MemoryPropertyFlags.
         * @return Pointer to the newly constructed buffer.
         */
        inline generic_buffer *
        new_buffer(size_t size, const ::vk::BufferUsageFlags &usage, const ::vk::MemoryPropertyFlags &flags) {
            auto buf = new generic_buffer(size, usage, flags, this);
            child_bufs.emplace_back(buf);
            return buf;
        }

        /**
         * @fn inline void delete_buffer(generic_buffer *buf)
         * @brief Destroy a generic_buffer associated with this window.
         * @note The supplied generic_buffer *DOES NOT* have to be associate with this window, and it *DOES NOT* have to be
         *       created by `window::new_buffer`.
         * @param buf The buffer to destroy
         */
        inline void delete_buffer(generic_buffer *buf) {
            boost::remove_erase(child_bufs, buf);
            delete buf;
        }

        /**
         * @fn void draw_frame()
         * @brief Draw the next frame to the screen.
         * @note This function blocks until the frame is acquired, but *DOES NOT* block until it is presented.
         *        This function is also thread safe! :)
         */
        void draw_frame();
    };
}


#endif //__HEPHAESTUS_VK_WINDOW_HPP
