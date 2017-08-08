/*!
* \brief
*
* \author ddubois
* \date 27-Sep-16.
*/

#include <algorithm>
#include <easylogging++.h>
#include <regex>
#include <iomanip>
#include <sstream>
#include "mesh_store.h"
#include "../../../render/nova_renderer.h"
#include "../utils/io.h"

namespace nova {
    std::vector<render_object>& mesh_store::get_meshes_for_shader(std::string shader_name) {
        return renderables_grouped_by_shader[shader_name];
    }

    void print_buffers(const std::string &texture_name, std::vector<float>& vertex_buffer, std::vector<unsigned int>& index_buffer) {
        // debug
        LOG(DEBUG) << "texture name: " << texture_name << std::endl;
        LOG(DEBUG) << "new buffers:" << std::endl;
        for(int i = 0; i + 7 < vertex_buffer.size(); i += 8) {
            std::ostringstream ss;
            ss << "  vertex ";
            for(int k = 0; k < 8; k++) {
                ss << std::setfill(' ') << std::setw(4) << i + k << " = " << std::setfill(' ') << std::setw(12) << std::fixed << std::setprecision(5) << vertex_buffer[i + k] << "  ";
            }
            LOG(DEBUG) << ss.str();
        }
        for(int i = 0; i + 2 < index_buffer.size(); i += 3) {
            std::ostringstream ss;
            ss << "  index ";
            for(int k = 0; k < 3; k++) {
                ss << std::setfill(' ') << std::setw(4) << i + k << " = " << std::setfill(' ') << std::setw(8) << index_buffer[i + k] << "  ";
            }
            LOG(DEBUG) << ss.str();
        }
    }

    void mesh_store::add_gui_buffers(mc_gui_send_buffer_command* command) {
        std::string texture_name(command->texture_name);
        texture_name = std::regex_replace(texture_name, std::regex("^textures/"), "");
        texture_name = std::regex_replace(texture_name, std::regex(".png$"), "");
        texture_name = "minecraft:" + texture_name;
        const texture_manager::texture_location tex_location = nova_renderer::instance->get_texture_manager().get_texture_location(texture_name);
        glm::vec2 tex_size = tex_location.max - tex_location.min;

        mesh_definition cur_screen_buffer = {};
        cur_screen_buffer.vertex_data.reserve(static_cast<unsigned long>(command->vertex_buffer_size));
        for (int i = 0; i + 8 < command->vertex_buffer_size; i += 9) {
            cur_screen_buffer.vertex_data[i]   = *reinterpret_cast<int*>(&command->vertex_buffer[i]);
            cur_screen_buffer.vertex_data[i+1] = *reinterpret_cast<int*>(&command->vertex_buffer[i+1]);
            cur_screen_buffer.vertex_data[i+2] = *reinterpret_cast<int*>(&command->vertex_buffer[i+2]);
            float u = command->vertex_buffer[i+3] * tex_size.x + tex_location.min.x;
            cur_screen_buffer.vertex_data[i+3] = *reinterpret_cast<int*>(&u);
            float v = command->vertex_buffer[i+4] * tex_size.y + tex_location.min.y;
            cur_screen_buffer.vertex_data[i+4] = *reinterpret_cast<int*>(&v);
            cur_screen_buffer.vertex_data[i+5] = *reinterpret_cast<int*>(&command->vertex_buffer[i+5]);
            cur_screen_buffer.vertex_data[i+6] = *reinterpret_cast<int*>(&command->vertex_buffer[i+6]);
            cur_screen_buffer.vertex_data[i+7] = *reinterpret_cast<int*>(&command->vertex_buffer[i+7]);
            cur_screen_buffer.vertex_data[i+8] = *reinterpret_cast<int*>(&command->vertex_buffer[i+8]);
        }
        cur_screen_buffer.indices.reserve(static_cast<unsigned long>(command->index_buffer_size));
        for(int i = 0; i < command->index_buffer_size; i++) {
            cur_screen_buffer.indices[i] = (unsigned int)command->index_buffer[i];
        }

        cur_screen_buffer.vertex_format = format::POS_UV_COLOR;

        render_object gui = {};
        gui.geometry = std::make_unique<nova::gl_mesh>(cur_screen_buffer);
        gui.type = geometry_type::gui;
        gui.name = "gui";
        gui.color_texture = command->atlas_name;

        // TODO: Something more intelligent
        renderables_grouped_by_shader["gui"].push_back(std::move(gui));
    }

    void mesh_store::remove_gui_render_objects() {
        for(auto& group : renderables_grouped_by_shader) {
            auto removed_elements = std::remove_if(group.second.begin(), group.second.end(),
                                                   [](auto& render_obj) {return render_obj.type == geometry_type::gui;});
            group.second.erase(removed_elements, group.second.end());
        }
    }

    void mesh_store::remove_render_objects(std::function<bool(render_object&)> filter) {
        for(auto& group : renderables_grouped_by_shader) {
            std::remove_if(group.second.begin(), group.second.end(), filter);
        }
    }

    void mesh_store::add_chunk_render_object(std::string filter_name, mc_chunk_render_object &chunk) {
        mesh_definition def;
        for(int i = 0; i < chunk.vertex_buffer_size; i++) {
            def.vertex_data.push_back(chunk.vertex_data[i]);
        }

        for(int i = 0; i < chunk.index_buffer_size; i++) {
            def.indices.push_back(chunk.indices[i]);
        }

        def.vertex_format = format::all_values()[chunk.format];
        def.position = {chunk.x, chunk.y, chunk.z};
        def.id = chunk.id;

        chunk_parts_to_upload_lock.lock();
        chunk_parts_to_upload.emplace(filter_name, def);
        chunk_parts_to_upload_lock.unlock();
    }

    void mesh_store::generate_needed_chunk_geometry() {
        chunk_parts_to_upload_lock.lock();
        while(!chunk_parts_to_upload.empty()) {
            auto& item = chunk_parts_to_upload.front();
            chunk_parts_to_upload.pop();

            mesh_definition& def = std::get<1>(item);
            render_object obj;
            obj.parent_id = def.id;
            obj.type = geometry_type::block;
            obj.name = "chunk_part";
            obj.geometry = std::make_unique<gl_mesh>(def);
            obj.color_texture = "block_color";
            obj.position = def.position;

            std::string& shader_name = std::get<0>(item);
            renderables_grouped_by_shader[shader_name].push_back(std::move(obj));
        }
        chunk_parts_to_upload_lock.unlock();
    }

    void mesh_store::remove_render_objects_with_parent(long parent_id) {
        remove_render_objects([&](render_object& obj) { return obj.parent_id == parent_id; });
    }
}
