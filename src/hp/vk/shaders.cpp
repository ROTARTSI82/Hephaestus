//
// Created by 25granty on 11/22/19.
//

#include "hp/vk/window.hpp"
//#include "hp/vk/vertex_buffers.hpp"

#include <boost/algorithm/string/predicate.hpp>

#include <fstream>
#include <utility>

namespace hp::vk {

    static ::vk::ShaderStageFlagBits get_bit_from_name(const std::string &name, bool &success) {
        if (name == "fragment-shader") {
            success = true;
            return ::vk::ShaderStageFlagBits::eFragment;
        } else if (name == "vertex-shader") {
            success = true;
            return ::vk::ShaderStageFlagBits::eVertex;
        } else if (name == "geometry-shader") {
            success = true;
            return ::vk::ShaderStageFlagBits::eGeometry;
        } else {
            success = false;
            return ::vk::ShaderStageFlagBits::eAll;
        }
    }

    shader_program::shader_program(const std::string fp, const char *metadat, window *parent) {
        this->parent = parent;
        this->fp = fp;
        metapath = metadat;

        pipeline_layout = ::vk::PipelineLayout();
        pipeline = ::vk::Pipeline();

        reload_from_file();
        // Insert shader boogies
    }

    shader_program &shader_program::operator=(shader_program &&rhs) noexcept {
        if (this == &rhs) { // Self assignment.
            return *this;
        }

        this->~shader_program();

        parent = rhs.parent;
        stage_cis = std::move(rhs.stage_cis);
        fp = std::move(rhs.fp);
        metapath = rhs.metapath;
        pipeline_layout = rhs.pipeline_layout;
        pipeline = rhs.pipeline;
        mods = std::move(rhs.mods);
        cmd_bufs = std::move(rhs.cmd_bufs);
        entrypoint_keepalives = std::move(rhs.entrypoint_keepalives);

        return *this;
    }

    shader_program::shader_program(shader_program &&rhs) noexcept {
        parent = rhs.parent;
        stage_cis = std::move(rhs.stage_cis);
        fp = std::move(rhs.fp);
        metapath = rhs.metapath;
        pipeline_layout = rhs.pipeline_layout;
        pipeline = rhs.pipeline;
        mods = std::move(rhs.mods);
        cmd_bufs = std::move(rhs.cmd_bufs);
        entrypoint_keepalives = std::move(rhs.entrypoint_keepalives);
    }

    shader_program::~shader_program() {
        if (pipeline != ::vk::Pipeline()) {
            parent->log_dev.destroyPipeline(pipeline, nullptr);
            pipeline = ::vk::Pipeline();
        }

        if (pipeline_layout != ::vk::PipelineLayout()) {
            parent->log_dev.destroyPipelineLayout(pipeline_layout, nullptr);
            pipeline_layout = ::vk::PipelineLayout();
        }

        while (!mods.empty()) {
            auto front = mods.front();
            parent->log_dev.destroyShaderModule(front, nullptr);
            mods.pop();
        }

        while (!entrypoint_keepalives.empty()) {
            auto f = entrypoint_keepalives.front();
            delete f;
            entrypoint_keepalives.pop();
        }
    }

    void shader_program::reload_from_file() {
        if (fp.find_last_not_of("/\\") + 2 == fp.length()) {
            HP_WARN("Do not include trailing slashes in shader program filepaths! Shader program '{}' will not be loaded!",
                    fp);
        }

        while (!mods.empty()) {
            auto front = mods.front();
            parent->log_dev.destroyShaderModule(front, nullptr);
            mods.pop();
        }

        std::ifstream fs(fp + metapath);
        if (!fs.is_open()) {
            HP_WARN("Requested shader program '{}' is not available! Aborting!", fp);
            return;
        }

        std::string line;
        stage_cis.clear();

        while (!entrypoint_keepalives.empty()) {
            auto f = entrypoint_keepalives.front();
            delete f;
            entrypoint_keepalives.pop();
        }

        unsigned line_num = 0;
        while (std::getline(fs, line)) {
            line_num++;

            line.erase(remove_if(line.begin(), line.end(), isspace), line.end());
            if (boost::starts_with(line, "#") || line.length() == 0) {  // Skip Comments and whitespace
                continue;
            }

            int pos = line.find(':');

            if (pos == std::string::npos) {
                HP_WARN("[** SYNTAX ERROR **] [{}:{}]: Line doesn't contain a colon! Skipping!", fp + metapath,
                        line_num);
                HP_WARN("         Sample Valid code: 'fragment-shader;main: frag.spv'");
                continue;
            }

            std::string shader_file = line.substr(pos + 1, line.length());
            std::string shader_type_and_entry = line.substr(0, pos);

            int entry_pos = shader_type_and_entry.find(';');

            if (entry_pos == std::string::npos) {
                HP_WARN("[** SYNTAX ERROR **] [{}:{}]: Line doesn't contain semi-colon! Skipping!", fp + metapath,
                        line_num);
                HP_WARN("         Sample Valid code: 'fragment-shader;main: frag.spv'");
                continue;
            }

            std::string shader_type = shader_type_and_entry.substr(0, entry_pos);
            std::string entry_point = shader_type_and_entry.substr(entry_pos + 1, shader_type_and_entry.length());

            std::string tmp_mod_fname(fp);
            tmp_mod_fname += "/";
            tmp_mod_fname += shader_file;
            std::ifstream mod_file(tmp_mod_fname, std::ios::ate | std::ios::binary);
            if (!mod_file.is_open()) {
                HP_WARN("[** IO ERROR **] [{}:{}]: Cannot open '{}'! Skipping!", fp + metapath, line_num, shader_file);
            }
            size_t mod_fsize = (size_t) mod_file.tellg();
            char *shader_src = new char[mod_fsize];

            mod_file.seekg(0);
            mod_file.read(shader_src, mod_fsize);
            mod_file.close();

            ::vk::ShaderModuleCreateInfo mod_ci(::vk::ShaderModuleCreateFlags(), mod_fsize,
                                                reinterpret_cast<const uint32_t *>(shader_src));

            ::vk::ShaderModule mod_obj;
            if (handle_res(parent->log_dev.createShaderModule(&mod_ci, nullptr, &mod_obj), HP_GET_CODE_LOC) !=
                ::vk::Result::eSuccess) {
                HP_FATAL("Shader module creation failed! Skipping!");
                delete[] shader_src;
                continue;
            }
            mods.push(mod_obj);


            char *entry_cstr = new char[entry_point.length()];
            std::strcpy(entry_cstr, entry_point.c_str());
            entrypoint_keepalives.push(entry_cstr);

            bool succ = false;
            ::vk::PipelineShaderStageCreateInfo stage_ci(::vk::PipelineShaderStageCreateFlags(),
                                                         get_bit_from_name(shader_type, succ), mod_obj,
                                                         entry_cstr);

            if (!succ) {
                HP_WARN("[** SYNTAX ERROR **] [{}:{}]: Unrecognized shader module type: '{}'", fp + metapath, line_num,
                        shader_type);
            }

            stage_cis.emplace_back(stage_ci);
            HP_DEBUG("Constructed {} module and stage from '{}/{}' with entry point of {}", shader_type, fp,
                     shader_file, entry_point);

            delete[] shader_src; // Does this mess with the thingy? Should we wait to delete it?
        }

        fs.close();

        // Pipeline layout creation

        if (pipeline_layout != ::vk::PipelineLayout()) {
            parent->log_dev.destroyPipelineLayout(pipeline_layout, nullptr);
            pipeline_layout = ::vk::PipelineLayout();
        }

        ::vk::PipelineLayoutCreateInfo pipeline_lyo_ci(::vk::PipelineLayoutCreateFlags(), 0, nullptr, 0, nullptr);

        if (handle_res(parent->log_dev.createPipelineLayout(&pipeline_lyo_ci, nullptr, &pipeline_layout),
                       HP_GET_CODE_LOC) != ::vk::Result::eSuccess) {
            HP_FATAL("Failed to create pipeline layout!");
        }
        HP_DEBUG("Pipeline layout successfully constructed from '{}'", fp);

        rebuild_pipeline();
    }

    void shader_program::rebuild_pipeline() {
        if (pipeline != ::vk::Pipeline()) {
            parent->log_dev.destroyPipeline(pipeline, nullptr);
            pipeline = ::vk::Pipeline();
        }

        if (buffer_layout::get_bound() == nullptr) {
            HP_WARN("No bound buffer layout found! Binding the default one.");
            buffer_layout::build_default_layout();
            buffer_layout::get_default()->bind();
        }

        auto current_bindings = buffer_layout::get_bound()->binding;
        auto current_attribs = buffer_layout::get_bound()->attribs;
        ::vk::PipelineVertexInputStateCreateInfo vert_in_ci(::vk::PipelineVertexInputStateCreateFlags(), 1,
                                                            &current_bindings,
                                                            current_attribs.size(), current_attribs.data());

        ::vk::PipelineInputAssemblyStateCreateInfo in_ci(::vk::PipelineInputAssemblyStateCreateFlags(),
                                                         ::vk::PrimitiveTopology::eTriangleList,
                                                         ::vk::Bool32(VK_FALSE));

        // Viewports and scissors are set in the dynamic state, so these args are ignored.
        ::vk::PipelineViewportStateCreateInfo viewport_state_ci(::vk::PipelineViewportStateCreateFlags(), 1, nullptr,
                                                                1, nullptr);

        ::vk::PipelineRasterizationStateCreateInfo raster_ci(::vk::PipelineRasterizationStateCreateFlags(),
                                                             ::vk::Bool32(VK_FALSE),
                                                             ::vk::Bool32(VK_FALSE), ::vk::PolygonMode::eFill,
                                                             ::vk::CullModeFlagBits::eBack,
                                                             ::vk::FrontFace::eClockwise,  // Typically counter clockwise
                                                             ::vk::Bool32(VK_FALSE), 0.0f, 0.0f, 0.0f, 1.0f);

        ::vk::PipelineMultisampleStateCreateInfo multisample_ci(::vk::PipelineMultisampleStateCreateFlags(),
                                                                ::vk::SampleCountFlagBits::e1, ::vk::Bool32(VK_FALSE),
                                                                1.0f, nullptr, ::vk::Bool32(VK_FALSE),
                                                                ::vk::Bool32(VK_FALSE));

        ::vk::PipelineColorBlendAttachmentState blend_attach(::vk::Bool32(VK_TRUE), ::vk::BlendFactor::eSrcAlpha,
                                                             ::vk::BlendFactor::eOneMinusSrcAlpha, ::vk::BlendOp::eAdd,
                                                             ::vk::BlendFactor::eOne, ::vk::BlendFactor::eZero,
                                                             ::vk::BlendOp::eAdd,
                                                             ::vk::ColorComponentFlagBits::eR |
                                                             ::vk::ColorComponentFlagBits::eG |
                                                             ::vk::ColorComponentFlagBits::eB |
                                                             ::vk::ColorComponentFlagBits::eA);

        ::vk::PipelineColorBlendStateCreateInfo blend_ci(::vk::PipelineColorBlendStateCreateFlags(),
                                                         ::vk::Bool32(VK_FALSE), ::vk::LogicOp::eCopy, 1, &blend_attach,
                                                         {0.0f, 0.0f, 0.0f, 0.0f});

        ::vk::DynamicState dynamic_states[] = {
                ::vk::DynamicState::eViewport,
                ::vk::DynamicState::eScissor,
                ::vk::DynamicState::eLineWidth,
                ::vk::DynamicState::eDepthBias,
                ::vk::DynamicState::eBlendConstants,
                ::vk::DynamicState::eDepthBounds,
                ::vk::DynamicState::eStencilCompareMask,
                ::vk::DynamicState::eStencilWriteMask,
                ::vk::DynamicState::eStencilReference,
        };

        ::vk::PipelineDynamicStateCreateInfo dynamic_state_ci(::vk::PipelineDynamicStateCreateFlags(), 9,
                                                              dynamic_states);

        ::vk::GraphicsPipelineCreateInfo pipeline_ci(::vk::PipelineCreateFlags(), stage_cis.size(), stage_cis.data(),
                                                     &vert_in_ci, &in_ci, nullptr, &viewport_state_ci, &raster_ci,
                                                     &multisample_ci, nullptr, &blend_ci, &dynamic_state_ci,
                                                     pipeline_layout, parent->render_pass, 0, ::vk::Pipeline(), -1);

        if (handle_res(
                parent->log_dev.createGraphicsPipelines(::vk::PipelineCache(), 1, &pipeline_ci, nullptr, &pipeline),
                HP_GET_CODE_LOC) != ::vk::Result::eSuccess) {
            HP_FATAL("Failed to create pipeline!");
        }
        HP_DEBUG("Fully constructed graphics pipeline from '{}'", fp);

        rebuild_cmd_bufs();
    }

    void shader_program::rebuild_cmd_bufs() {
        cmd_bufs = std::move(
                get_cmd_bufs(&parent->framebuffers, &parent->render_pass, &parent->swap_extent, &parent->cmd_pool));
    }

    std::vector<::vk::CommandBuffer> shader_program::get_cmd_bufs(std::vector<::vk::Framebuffer> *frame_bufs,
                                                                  ::vk::RenderPass *rend_pass, ::vk::Extent2D *extent,
                                                                  ::vk::CommandPool *cmd_pool) {
        std::vector<::vk::CommandBuffer> ret(frame_bufs->size());
        ::vk::CommandBufferAllocateInfo cmd_buf_ai(*cmd_pool, ::vk::CommandBufferLevel::ePrimary,
                                                   ret.size());

        if (handle_res(parent->log_dev.allocateCommandBuffers(&cmd_buf_ai, ret.data()), HP_GET_CODE_LOC) !=
            ::vk::Result::eSuccess) {
            HP_FATAL("Failed to allocated command buffers!");
            std::terminate();
        }

        for (size_t i = 0; i < ret.size(); i++) {
            ::vk::CommandBufferBeginInfo cmd_buf_bi(::vk::CommandBufferUsageFlags(), nullptr);

            if (handle_res(ret[i].begin(&cmd_buf_bi), HP_GET_CODE_LOC) != ::vk::Result::eSuccess) {
                HP_FATAL("Failed to begin command buffer recording!");
                std::terminate();
            }

            ::vk::ClearValue clear_col(::vk::ClearColorValue(std::array<float, 4>({0.0f, 0.0f, 0.0f, 1.0f})));

            ::vk::RenderPassBeginInfo rend_pass_bi(*rend_pass, (*frame_bufs)[i],
                                                   ::vk::Rect2D(::vk::Offset2D(0, 0), *extent), 1,
                                                   &clear_col);

            ::vk::Viewport viewport(0.0f, 0.0f, (float) extent->width, (float) extent->height, 0.0f,
                                    1.0f);

            ::vk::Rect2D scissor(::vk::Offset2D(0, 0), *extent);

            ret[i].beginRenderPass(&rend_pass_bi, ::vk::SubpassContents::eInline);
            ret[i].bindPipeline(::vk::PipelineBindPoint::eGraphics, pipeline);
            ret[i].setViewport(0, 1, &viewport);
            ret[i].setScissor(0, 1, &scissor);

            for (const auto &fn : parent->record_buffer) {
                fn(ret[i]);
            }

            ret[i].endRenderPass();

#ifdef VULKAN_HPP_DISABLE_ENHANCED_MODE
            if (handle_res(ret[i].end(), HP_GET_CODE_LOC) != ::vk::Result::eSuccess) {
                HP_FATAL("Failed to end command buffer recording!");
                std::terminate();
            }
#else
            ret[i].end();  // Enhanced mode does exception handling for us. :)
#endif
        }
        return ret;
    }
}
