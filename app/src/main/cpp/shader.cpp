//
// Created by GRAM on 2022-05-04.
//
#include <GLES3/gl32.h>

#include <android_native_app_glue.h>
#include <cstdlib>

#include "logger.h"

static bool checkGlError(const char* funcName) {
    GLint err = glGetError();
    if (err != GL_NO_ERROR) {
        LOGE("GL error after %s(): 0x%08x\n", funcName, err);
        return true;
    }
    return false;
}

GLuint createShader(AAssetManager* am, GLenum shaderType, const char* filename) {
    AAsset* asset = AAssetManager_open(am, filename, AASSET_MODE_BUFFER);
    GLuint shader = glCreateShader(shaderType);
    if (!shader) {
        checkGlError("glCreateShader");
        return 0;
    }
    GLint length = AAsset_getLength(asset);
    const char* src = (const char* )AAsset_getBuffer(asset);
    glShaderSource(shader, 1, &src, &length);

    GLint compiled = GL_FALSE;
    glCompileShader(shader);
    AAsset_close(asset);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint infoLogLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLen);
        if (infoLogLen > 0) {
            GLchar* infoLog = (GLchar*)malloc(infoLogLen);
            if (infoLog) {
                glGetShaderInfoLog(shader, infoLogLen, NULL, infoLog);
                LOGE("Could not compile %s shader:\n%s\n",
                     shaderType == GL_VERTEX_SHADER ? "vertex" : "fragment",
                     infoLog);
                free(infoLog);
            }
        }
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLuint createProgram(AAssetManager* am, const char* vtxFilename, const char* fragFilename) {
    GLuint vtxShader = 0;
    GLuint fragShader = 0;
    GLuint program = 0;
    GLint linked = GL_FALSE;

    vtxShader = createShader(am, GL_VERTEX_SHADER, vtxFilename);
    if (!vtxShader)
        goto exit;

    fragShader = createShader(am, GL_FRAGMENT_SHADER, fragFilename);
    if (!fragShader)
        goto exit;

    program = glCreateProgram();
    if (!program) {
        checkGlError("glCreateProgram");
        goto exit;
    }
    glAttachShader(program, vtxShader);
    glAttachShader(program, fragShader);

    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        LOGE("Could not link program");
        GLint infoLogLen = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLen);
        if (infoLogLen) {
            GLchar* infoLog = (GLchar*)malloc(infoLogLen);
            if (infoLog) {
                glGetProgramInfoLog(program, infoLogLen, NULL, infoLog);
                LOGE("Could not link program:\n%s\n", infoLog);
                free(infoLog);
            }
        }
        glDeleteProgram(program);
        program = 0;
    }

    exit:
    glDeleteShader(vtxShader);
    glDeleteShader(fragShader);
    return program;
}