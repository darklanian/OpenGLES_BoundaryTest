//
// Created by GRAM on 2022-05-02.
//

#ifndef BOUNDARYTEST_RENDER_H
#define BOUNDARYTEST_RENDER_H

#include <GLES3/gl32.h>
#include <android_native_app_glue.h>

void render_init(AAssetManager* am, int width, int height);
void render_deinit();
void render_draw_frame(float move);

#endif //BOUNDARYTEST_RENDER_H
