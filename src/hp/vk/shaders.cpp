//
// Created by 25granty on 11/22/19.
//

#include "hp/vk/window.hpp"

#include <boost/algorithm/string/predicate.hpp>

#include <fstream>
#include <utility>
#include "vk_mem_alloc.h"

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

    shader_program::shader_program(const std::string &fp, const char *metadat, window *parent) {
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
        entrypoint_keepalives = std::move(rhs.entrypoint_keepalives);
    }

    shader_program::~shader_program() {
        parent->log_dev.destroyPipeline(pipeline, nullptr);
        parent->log_dev.destroyPipelineLayout(pipeline_layout, nullptr);

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

            std::string::size_type pos = line.find(':');

            if (pos == std::string::npos) {
                HP_WARN("[** SYNTAX ERROR **] [{}:{}]: Line doesn't contain a colon! Skipping!", fp + metapath,
                        line_num);
                HP_WARN("         Sample Valid code: 'fragment-shader;main: frag.spv'");
                continue;
            }

            std::string shader_file = line.substr(pos + 1, line.length());
            std::string shader_type_and_entry = line.substr(0, pos);

            std::string::size_type entry_pos = shader_type_and_entry.find(';');

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
            strcpy_s(entry_cstr, entry_point.length(), entry_point.c_str());
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

            delete[] shader_src; // Does this mess with the thingy? Should we wait to delete it? ANSWERS: No, Yes.
        }

        fs.close();

        // Pipeline layout creation

        parent->log_dev.destroyPipelineLayout(pipeline_layout, nullptr);

        ::vk::PipelineLayoutCreateInfo pipeline_lyo_ci(::vk::PipelineLayoutCreateFlags(), 0, nullptr, 0, nullptr);

        if (handle_res(parent->log_dev.createPipelineLayout(&pipeline_lyo_ci, nullptr, &pipeline_layout),
                       HP_GET_CODE_LOC) != ::vk::Result::eSuccess) {
            HP_FATAL("Failed to create pipeline layout!");
        }
        HP_DEBUG("Pipeline layout successfully constructed from '{}'", fp);

        rebuild_pipeline();
    }

    void shader_program::rebuild_pipeline() {
        parent->log_dev.destroyPipeline(pipeline, nullptr);

#ifdef HP_DEBUG_MODE_ACTIVE
        if (buffer_layout::bound_lyos.empty()) {
            HP_WARN("There are no active buffer layouts! Adding the default one!");
            buffer_layout::build_default_layout();
            buffer_layout::bound_lyos.emplace_back(buffer_layout::get_default());
            buffer_layout::rebuild_bound_info();
        } else if (buffer_layout::global_attribs.empty() || buffer_layout::global_bindings.empty()) {
            HP_WARN("Built layout info doesn't match bound buffer! Did you forget to call `hp::vk::buffer_layout::rebuild_bound_info()`?");
            buffer_layout::rebuild_bound_info();
        }

        if (buffer_layout::bound_lyos.size() != buffer_layout::global_bindings.size()) {
            HP_WARN("Built layout info doesn't match bound buffer! Did you forget to call `hp::vk::buffer_layout::rebuild_bound_info()`?");
            buffer_layout::rebuild_bound_info();
        }
#endif

        ::vk::PipelineVertexInputStateCreateInfo vert_in_ci(::vk::PipelineVertexInputStateCreateFlags(),
                                                            buffer_layout::global_bindings.size(),
                                                            buffer_layout::global_bindings.data(),
                                                            buffer_layout::global_attribs.size(),
                                                            buffer_layout::global_attribs.data());

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
                                                             ::vk::FrontFace::eCounterClockwise,  // Typically counter clockwise
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
    }
}
