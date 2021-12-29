#include <util_init.hpp>
#include "AndroidGraphicsApplication.hpp"
#include "cube_data.h"

AndroidGraphicsApplication::AndroidGraphicsApplication(AAssetManager* assetManager, ANativeWindow* window) {
    mWindow = window;
}

AndroidGraphicsApplication::~AndroidGraphicsApplication() {
}

void AndroidGraphicsApplication::createSurface() {
    const char* argv[] = { "cube" };
    int argc = sizeof(argv)/sizeof(char*);
    demo_main_android(&demo, mWindow, argc, argv);
    demo_draw(&demo);
}

void AndroidGraphicsApplication::setSize(uint32_t w, uint32_t h) {
    width = w;
    height = h;
}

void AndroidGraphicsApplication::drawFrame() {
    // not done yet
    // redraw after SurfaceView.surfaceRedrawNeeded()
}

void AndroidGraphicsApplication::init_window_size_patched(struct sample_info &info) {
    int32_t default_width = ANativeWindow_getWidth(mWindow);
    int32_t default_height = ANativeWindow_getHeight(mWindow);
    info.width = default_width;
    info.height = default_height;
}

void AndroidGraphicsApplication::init_swapchain_extension_patched(struct sample_info &info) {
    init_swapchain_extension(info, mWindow);
}
