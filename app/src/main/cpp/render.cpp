#include <initializer_list>
#include <cstdlib>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "logger.h"
#include "render.h"

#include "wall.h"
#include "cube.h"
#include "sphere.h"

GLuint program;
GLuint matrixId;
GLuint colorId;
glm::mat4 projectionMatrix;
glm::mat4 viewMatrix;

GLuint createProgram(AAssetManager* am, const char* vtxFilename, const char* fragFilename);

void render_init(AAssetManager* am, int width, int height) {
    // Check openGL on the system
    auto opengl_info = {GL_VENDOR, GL_RENDERER, GL_VERSION, GL_EXTENSIONS};
    for (auto name : opengl_info) {
        auto info = glGetString(name);
        LOGI("OpenGL Info: %s", info);
    }

    glViewport(0, 0, width, height);

    program = createProgram(am, "vertex.shader", "fragment.shader");
    matrixId = glGetUniformLocation(program, "MVP");
    colorId = glGetUniformLocation(program, "color");

    // Projection matrix : 45° Field of View, width:height ratio, display range : 0.1 unit <-> 100 units
    projectionMatrix = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
    // Camera matrix
    viewMatrix       = glm::lookAt(
            glm::vec3(0,3,10), // Camera is at (x,y,z), in World Space
            glm::vec3(0,0,0), // and looks at the origin
            glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
    );

    wall_init();
    cube_init();
    sphere_init(am);
}

void set_color(float r, float g, float b, float a) {
    GLfloat color[] = {r, g, b, a};
    glUniform4fv(colorId, 1, color);
}

void set_matrix(glm::mat4 matrix) {
    // Our ModelViewProjection : multiplication of our 3 matrices
    glm::mat4 MVP        = projectionMatrix * viewMatrix * matrix; // Remember, matrix multiplication is the other way around
    glUniformMatrix4fv(matrixId, 1, GL_FALSE, &MVP[0][0]);
}

void draw_walls() {
    set_color(1.0f, 0.0f, 0.0f, 1.0f);
    glm::mat4 modelMatrix;

    modelMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-1.0f, 0.0f, 0.0f));
    set_matrix(modelMatrix);

    wall_enable();
    wall_draw();

    modelMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(1.0f, 0.0f, 0.0f));
    set_matrix(modelMatrix);

    wall_draw();

    set_color(0.5f, 0.0f, 0.0f, 1.0f);

    modelMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-1.0f, -1.0f, 1.0f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(1.2f, 1.2f, 1.2f));
    set_matrix(modelMatrix);

    wall_draw();

    glBindVertexArray(0);
}

float move_z = 0.0f;

void draw_cube(float scale) {
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 translateMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, move_z));
    glm::mat4 rotateMatrix = glm::rotate(modelMatrix, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 scaleMatrix = glm::scale(modelMatrix, glm::vec3(scale));

    modelMatrix = translateMatrix * rotateMatrix * scaleMatrix;
    set_matrix(modelMatrix);

    cube_enable();

    cube_draw();

    glBindVertexArray(0);
}

void draw_sphere(float scale) {
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 translateMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, move_z));
    glm::mat4 rotateMatrix = glm::mat4(1.0f);
    glm::mat4 scaleMatrix = glm::scale(modelMatrix, glm::vec3(scale));

    modelMatrix = translateMatrix * rotateMatrix * scaleMatrix;
    set_matrix(modelMatrix);

    sphere_enable();

    sphere_draw();

    glBindVertexArray(0);
}

void draw_intersection(float scale, bool cube) {
    glDisable(GL_CULL_FACE);
    glEnable(GL_STENCIL_TEST);
    glDepthFunc(GL_GEQUAL);
    glDepthMask(GL_FALSE);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    if (cube)
        draw_cube(scale);
    else
        draw_sphere(scale);

    //set_color(0.1f, 0.1f, 0.4f, 0.5f);
    glDepthFunc(GL_LEQUAL);
    glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    if (cube)
        draw_cube(scale);
    else
        draw_sphere(scale);

    glClear(GL_STENCIL_BUFFER_BIT);
    glDisable(GL_STENCIL_TEST);
    glDepthMask(GL_TRUE);
}

void draw_hole(float scale, float thickness, bool cube = false) {
    draw_intersection(scale, cube);

    set_color(0.0f, 0.0f, 0.0f, 0.0f);
    draw_intersection(scale-thickness, cube);
}

void render_draw_frame(float move) {
    move_z = move;
    // Initialize GL state.
    glDisable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Just fill the screen with a color.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glUseProgram(program);

    draw_walls();

    bool cube = false;

    set_color(1.0f, 1.0f, 1.0f, 1.0f);
    draw_hole(0.3f, 0.05f, cube);

    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    set_color(0.0f, 1.0f, 0.0f, 0.5f);
    if (cube)
        draw_cube(0.05f);
    else
        draw_sphere(0.05f);
}

void render_deinit() {
    glDeleteProgram(program);
    wall_deinit();
    cube_deinit();
    sphere_deinit();
}