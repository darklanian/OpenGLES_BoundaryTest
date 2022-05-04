//
// Created by GRAM on 2022-05-02.
//

#ifndef BOUNDARYTEST_LOGGER_H
#define BOUNDARYTEST_LOGGER_H

#include <android/log.h>

#define TAG "[LANIAN]"
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__))

#define UNUSED(x) (void)(x)

#endif //BOUNDARYTEST_LOGGER_H
