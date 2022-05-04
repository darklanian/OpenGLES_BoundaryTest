//
// Created by GRAM on 2022-05-04.
//
#include <GLES3/gl32.h>

const GLfloat c_vertex_buffer_data[] = {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
};

const GLint c_index_buffer_data[] = {
        0, 1, 2,
        2, 1, 3
};

GLuint wall_vao;
GLuint wall_vbo;
GLuint wall_ibo;

void wall_init() {
    glGenVertexArrays(1, &wall_vao);
    glGenBuffers(1, &wall_vbo);
    glGenBuffers(1, &wall_ibo);

    glBindVertexArray(wall_vao);

    glBindBuffer(GL_ARRAY_BUFFER, wall_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(c_vertex_buffer_data), c_vertex_buffer_data, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wall_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(c_index_buffer_data), c_index_buffer_data, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

}

void wall_deinit() {
    glDeleteVertexArrays(1, &wall_vao);
    glDeleteBuffers(1, &wall_vbo);
    glDeleteBuffers(1, &wall_ibo);
}

void wall_enable() {
    glBindVertexArray(wall_vao);
}

void wall_draw() {
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}