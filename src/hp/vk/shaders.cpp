//
// Created by 25granty on 11/22/19.
//

#include "hp/vk/window.hpp"

#include <boost/algorithm/string/predicate.hpp>

#include <fstream>

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

    shader_program::shader_program(const std::string &fp, const char *metadat, window *parent) : parent(parent) {
        if (fp.find_last_not_of("/\\") + 2 == fp.length()) {
            HP_WARN("Do not include trailing slashes in shader program filepaths! Shader program '{}' will not be loaded!",
                    fp);
        }

        std::ifstream fs(fp + metadat);
        if (!fs.is_open()) {
            HP_WARN("Requested shader program '{}' is not available! Aborting!", fp);
            return;
        }

        std::string line;

        std::vector<::vk::ShaderModule> modules = std::vector<::vk::ShaderModule>();
        std::vector<::vk::PipelineShaderStageCreateInfo> stage_cis;

        unsigned line_num = 0;
        while (std::getline(fs, line)) {
            line_num++;

            line.erase(remove_if(line.begin(), line.end(), isspace), line.end());
            if (boost::starts_with(line, "#") || line.length() == 0) {  // Skip Comments and whitespace
                continue;
            }

            HP_INFO("Line: {}", line);

            int pos = line.find(':');

            if (pos == std::string::npos) {
                HP_WARN("[** SYNTAX ERROR **] [{}:{}]: Line doesn't contain a colon! Skipping!", fp + metadat,
                        line_num);
                HP_WARN("         Sample Valid code: 'fragment-shader;main: frag.spv'");
                continue;
            }

            std::string shader_file = line.substr(pos + 1, line.length());
            std::string shader_type_and_entry = line.substr(0, pos);

            int entry_pos = shader_type_and_entry.find(';');

            if (entry_pos == std::string::npos) {
                HP_WARN("[** SYNTAX ERROR **] [{}:{}]: Line doesn't contain semi-colon! Skipping!", fp + metadat,
                        line_num);
                HP_WARN("         Sample Valid code: 'fragment-shader;main: frag.spv'");
                continue;
            }

            std::string shader_type = shader_type_and_entry.substr(0, entry_pos);
            std::string entry_point = shader_type_and_entry.substr(entry_pos + 1, shader_type_and_entry.length());

            HP_INFO("Shader_type={}, entry_point={}, Shader_file={}", shader_type, entry_point, shader_file);

            std::string tmp_mod_fname(fp);
            tmp_mod_fname += "/";
            tmp_mod_fname += shader_file;
            std::ifstream mod_file(tmp_mod_fname, std::ios::ate | std::ios::binary);
            if (!mod_file.is_open()) {
                HP_WARN("[** IO ERROR **] [{}:{}]: Cannot open '{}'! Skipping!", fp + metadat, line_num, shader_file);
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
            modules.emplace_back(mod_obj);

            bool succ = false;
            ::vk::PipelineShaderStageCreateInfo stage_ci(::vk::PipelineShaderStageCreateFlags(),
                                                         get_bit_from_name(shader_type, succ), mod_obj,
                                                         entry_point.c_str());

            if (!succ) {
                HP_WARN("[** SYNTAX ERROR **] [{}:{}]: Unrecognized shader module type: '{}'", fp + metadat, line_num,
                        shader_type);
            }

            stage_cis.emplace_back(stage_ci);

            delete[] shader_src;
        }

        fs.close();

        // Insert shader boogies

        for (auto mod : modules) {
            parent->log_dev.destroyShaderModule(mod, nullptr);
        }
    }

    shader_program &shader_program::operator=(shader_program &&rhs) noexcept {
        if (this == &rhs) { // Self assignment.
            return *this;
        }

        parent = rhs.parent;

        return *this;
    }

    shader_program::shader_program(shader_program &&rhs) noexcept : parent(rhs.parent) {}
}
