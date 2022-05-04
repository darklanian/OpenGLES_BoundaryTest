//
// Created by GRAM on 2022-05-04.
//
#include <GLES3/gl32.h>

const GLfloat c_vertex_buffer_data[] = {
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f,  1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,

        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
};

const GLint c_index_buffer_data[] = {
        0, 1, 2,
        2, 1, 3,

        1, 5, 3,
        3, 5, 7,

        5, 4, 7,
        7, 4, 6,

        4, 0, 6,
        6, 0, 2,

        2, 3, 6,
        6, 3, 7,

        4, 5, 0,
        0, 5, 1,
};

GLuint cube_vao;
GLuint cube_vbo;
GLuint cube_ibo;

void cube_init() {
    glGenVertexArrays(1, &cube_vao);
    glGenBuffers(1, &cube_vbo);
    glGenBuffers(1, &cube_ibo);

    glBindVertexArray(cube_vao);

    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(c_vertex_buffer_data), c_vertex_buffer_data, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(c_index_buffer_data), c_index_buffer_data, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);
}

void cube_deinit() {
    glDeleteVertexArrays(1, &cube_vao);
    glDeleteBuffers(1, &cube_vbo);
    glDeleteBuffers(1, &cube_ibo);
}

void cube_enable() {
    glBindVertexArray(cube_vao);
}

void cube_draw() {
    glDrawElements(GL_TRIANGLES, 6*2*3, GL_UNSIGNED_INT, 0);
}