/*!
 * \author David
 * \date 13-May-16.
 */

#include <stdexcept>
#include <easylogging++.h>
#include "gl_mesh.h"
#include "../windowing/glfw_gl_window.h"

namespace nova {
    gl_mesh::gl_mesh() : vertex_buffer(0), indices(0), vertex_array(0), num_indices(0) {
        create();
    }

    gl_mesh::gl_mesh(const mesh_definition &definition) {
        create();
        set_data(definition.vertex_data, definition.vertex_format, usage::static_draw);
        set_index_array(definition.indices, usage::static_draw);
    }

    gl_mesh::~gl_mesh() {
        destroy();
    }

    void gl_mesh::create() {
        glGenVertexArrays(1, &vertex_array);
        glBindVertexArray(vertex_array);
        glGenBuffers(1, &vertex_buffer);
        glGenBuffers(1, &indices);
    }

    void gl_mesh::destroy() {
        if(vertex_buffer != 0) {
            if(glfwGetCurrentContext() != nullptr) {
                glDeleteBuffers(1, &vertex_buffer);
            }
            vertex_buffer = 0;
        }

        if(indices != 0) {
            if(glfwGetCurrentContext() != nullptr) {
                glDeleteBuffers(1, &indices);
            }
            indices = 0;
        }
    }

    void gl_mesh::set_data(std::vector<int> data, format data_format, usage data_usage) {
        this->data_format = data_format;

        glBindVertexArray(vertex_array);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        GLenum buffer_usage = translate_usage(data_usage);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), buffer_usage);

        enable_vertex_attributes(data_format);
    }

    GLenum gl_mesh::translate_usage(const usage data_usage) const {
        switch(data_usage) {
            case usage::dynamic_draw:
                return GL_DYNAMIC_DRAW;
            case usage::static_draw:
                return GL_STATIC_DRAW;
        }
    }

    void gl_mesh::set_active() const {
        glBindVertexArray(vertex_array);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);
    }

    void gl_mesh::set_index_array(std::vector<int> data, usage data_usage) {
        glBindVertexArray(vertex_array);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);
        GLenum buffer_usage = translate_usage(data_usage);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.size() * sizeof(unsigned int), data.data(), buffer_usage);

        num_indices = (unsigned int) data.size();
    }

    void gl_mesh::draw() const {
        glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, nullptr);
    }

    void gl_mesh::enable_vertex_attributes(format data_format) {
        switch(data_format) {
            case format::POS:
                // We only need to set up positional data
                // Positions are always at vertex attribute 0
                glEnableVertexAttribArray(0);   // Position

                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

                break;

            case format::POS_UV:
                glEnableVertexAttribArray(0);   // Position
                glEnableVertexAttribArray(1);   // Texture UV

                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *) (3 * sizeof(GLfloat)));

                break;

            case format::POS_UV_COLOR:
                glEnableVertexAttribArray(0);   // Position
                glEnableVertexAttribArray(1);   // Texture UV
                glEnableVertexAttribArray(2);   // Vertex color

                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), nullptr);
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (void *) (3 * sizeof(GLfloat)));
                glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (void *) (5 * sizeof(GLfloat)));

                break;

            case format::POS_COLOR_UV_LIGHTMAPUV_NORMAL_TANGENT:
                glEnableVertexAttribArray(0);   // Position
                glEnableVertexAttribArray(1);   // Texture UV
                glEnableVertexAttribArray(2);   // Lightmap UV
                glEnableVertexAttribArray(3);   // Normal
                glEnableVertexAttribArray(4);   // Tangent
                glEnableVertexAttribArray(5);   // Color

                // position
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 13 * sizeof(GLfloat), nullptr);

                // color
                glVertexAttribPointer(5, 4, GL_UNSIGNED_BYTE, GL_FALSE, 13 * sizeof(GLfloat), (void *) (12 * sizeof(GLbyte)));

                // texture UV
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 13 * sizeof(GLfloat), (void *) (16 * sizeof(GLbyte)));

                // lightmap UV
                glVertexAttribPointer(2, 2, GL_SHORT, GL_FALSE, 13 * sizeof(GLfloat), (void *) (24 * sizeof(GLbyte)));

                // normal
                glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 13 * sizeof(GLfloat), (void *) (32 * sizeof(GLbyte)));

                // tangent
                glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 13 * sizeof(GLfloat), (void *) (44 * sizeof(GLbyte)));

                break;
        }
    }

    format gl_mesh::get_format() {
        return data_format;
    }

    bool gl_mesh::has_data() const {
        return num_indices > 0;
    }
}
