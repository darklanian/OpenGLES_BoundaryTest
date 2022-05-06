/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

//BEGIN_INCLUDE(all)
#include <initializer_list>
#include <memory>
#include <cstdlib>
#include <cstring>
#include <jni.h>
#include <cerrno>
#include <cassert>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/sensor.h>
#include <android_native_app_glue.h>
#include <android/asset_manager_jni.h>

#include "logger.h"
#include "render.h"

/**
 * Shared state for our app.
 */
struct engine {
    struct android_app* app;

    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t width;
    int32_t height;

    float move;
};

AAssetManager* getAssets(android_app* app) {
    AAssetManager* am = nullptr;

    JNIEnv* env = nullptr;
    app->activity->vm->AttachCurrentThread(&env, nullptr);

    jclass android_content_Context = env->GetObjectClass(app->activity->clazz);
    jmethodID midGetAssets = env->GetMethodID(android_content_Context,
                                              "getAssets",
                                              "()Landroid/content/res/AssetManager;");
    jobject am_jobject = env->CallObjectMethod(app->activity->clazz, midGetAssets);

    am = AAssetManager_fromJava(env, am_jobject);

    app->activity->vm->DetachCurrentThread();

    return am;
}
/**
 * Initialize an EGL context for the current display.
 */
static int engine_init_display(struct engine* engine) {
    // initialize OpenGL ES and EGL

    /*
     * Here specify the attributes of the desired configuration.
     * Below, we select an EGLConfig with at least 8 bits per color
     * component compatible with on-screen windows
     */
    const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_DEPTH_SIZE, 24,
            EGL_STENCIL_SIZE, 8,
            EGL_NONE
    };
    EGLint w, h, format;
    EGLint numConfigs;
    EGLConfig config = nullptr;
    EGLSurface surface;
    EGLContext context;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(display, nullptr, nullptr);

    /* Here, the application chooses the configuration it desires.
     * find the best match if possible, otherwise use the very first one
     */
    eglChooseConfig(display, attribs, nullptr,0, &numConfigs);
    std::unique_ptr<EGLConfig[]> supportedConfigs(new EGLConfig[numConfigs]);
    assert(supportedConfigs);
    eglChooseConfig(display, attribs, supportedConfigs.get(), numConfigs, &numConfigs);
    assert(numConfigs);
    LOGI("numConfigs=%d", numConfigs);
    auto i = 0;
    for (; i < numConfigs; i++) {
        auto& cfg = supportedConfigs[i];
        bool ok = true;
        for (int j = 0; j < sizeof(attribs); ++j) {
            EGLint v;
            if (!eglGetConfigAttrib(display, cfg, attribs[j++], &v) || v != attribs[j]) {
                ok = false;
            }
        }
        if (ok) {
            config = supportedConfigs[i];
            break;
        }
        /*EGLint r, g, b, a, d, s;
        if (eglGetConfigAttrib(display, cfg, EGL_RED_SIZE, &r)   &&
            eglGetConfigAttrib(display, cfg, EGL_GREEN_SIZE, &g) &&
            eglGetConfigAttrib(display, cfg, EGL_BLUE_SIZE, &b)  &&
            eglGetConfigAttrib(display, cfg, EGL_ALPHA_SIZE, &a)  &&
            eglGetConfigAttrib(display, cfg, EGL_DEPTH_SIZE, &d) &&
            eglGetConfigAttrib(display, cfg, EGL_STENCIL_SIZE, &s) &&
            r == 8 && g == 8 && b == 8 && a == 8 && d == 24 && s == 8) {
            config = supportedConfigs[i];
            break;
        }*/
    }
    if (i == numConfigs) {
        config = supportedConfigs[0];
    }

    if (config == nullptr) {
        LOGW("Unable to initialize EGLConfig");
        return -1;
    }

    /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
     * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
     * As soon as we picked a EGLConfig, we can safely reconfigure the
     * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
    surface = eglCreateWindowSurface(display, config, engine->app->window, nullptr);
    EGLint egl_ctx_attribs[] = {
            EGL_CONTEXT_CLIENT_VERSION, 3,
            EGL_NONE,
    };
    context = eglCreateContext(display, config, nullptr, egl_ctx_attribs);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        LOGW("Unable to eglMakeCurrent");
        return -1;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    engine->display = display;
    engine->context = context;
    engine->surface = surface;
    engine->width = w;
    engine->height = h;

    return 0;
}

/**
 * Just the current frame in the display.
 */
static void engine_draw_frame(struct engine* engine) {
    if (engine->display == nullptr) {
        // No display.
        return;
    }

    render_draw_frame(engine->move);

    eglSwapBuffers(engine->display, engine->surface);
}

/**
 * Tear down the EGL context currently associated with the display.
 */
static void engine_term_display(struct engine* engine) {
    render_deinit();

    if (engine->display != EGL_NO_DISPLAY) {
        eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (engine->context != EGL_NO_CONTEXT) {
            eglDestroyContext(engine->display, engine->context);
        }
        if (engine->surface != EGL_NO_SURFACE) {
            eglDestroySurface(engine->display, engine->surface);
        }
        eglTerminate(engine->display);
    }
    engine->display = EGL_NO_DISPLAY;
    engine->context = EGL_NO_CONTEXT;
    engine->surface = EGL_NO_SURFACE;
}

/**
 * Process the next input event.
 */
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
    auto* engine = (struct engine*)app->userData;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        int m = AMotionEvent_getY(event, 0) - engine->height/2;
        engine->move = m * 3.0 / (engine->height/2);
        return 1;
    }
    return 0;
}

/**
 * Process the next main command.
 */
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
    auto* engine = (struct engine*)app->userData;
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            break;
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            if (app->window != nullptr) {
                engine_init_display(engine);
                render_init(getAssets(engine->app), engine->width, engine->height);
            }
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            engine_term_display(engine);
            break;
        case APP_CMD_GAINED_FOCUS:
            break;
        case APP_CMD_LOST_FOCUS:
            // Also stop animating.
            break;
        default:
            break;
    }
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* state) {
    struct engine engine{};

    memset(&engine, 0, sizeof(engine));
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;
    engine.app = state;

    // loop waiting for stuff to do.

    while (true) {
        // Read all pending events.
        int events;
        struct android_poll_source* source;

        // If not animating, we will block forever waiting for events.
        // If animating, we loop until all events are read, then continue
        // to draw the next frame of animation.
        while (ALooper_pollAll((engine.app->window != nullptr) ? 0 : -1, nullptr, &events,
                                      (void**)&source) >= 0) {
            // Process this event.
            if (source != nullptr) {
                source->process(state, source);
            }

            // Check if we are exiting.
            if (state->destroyRequested != 0) {
                engine_term_display(&engine);
                return;
            }
        }

        engine_draw_frame(&engine);
    }
}
//END_INCLUDE(all)
