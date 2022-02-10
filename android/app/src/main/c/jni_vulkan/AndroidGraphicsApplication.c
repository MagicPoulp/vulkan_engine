
#include <android/asset_manager.h>
#include <android/native_window.h>
#include <android/log.h>
#include <android_native_app_glue.h>
#include "AndroidGraphicsApplication.h"
#include "Program.h"
#include "VulkanDSL.h"
#define VOLK_IMPLEMENTATION
#include "volk_setup.h"

struct Program *program;

void createSurface(ANativeWindow* window, AAssetManager* assetManager) {
    if (volkInitialize())
    {
        exit(1);
    }
    main_android(window, assetManager);
}

void main_android(struct ANativeWindow* window, AAssetManager* assetManager) {
    program = Program__create("");
    program->vulkanDSL->window = window;
    program->vulkanDSL->assetsFetcher.assetManager = assetManager;
    vulkanDSL_main(program->vulkanDSL);
}

void demoDestroy() {
    Program__destroy(program);
}

void drawFrame(double elapsedTimeS) {
    //demo_draw(program->vulkanDSL, elapsedTimeS);
}

void setSize(int32_t width, int32_t height) {
    VulkanDSL__setSize(program->vulkanDSL, width, height);
}
