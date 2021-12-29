
#include <stdlib.h>
#include "AndroidGraphicsApplication.h"
#include "string.h"

extern struct demo demo;
extern int demo_main_android(struct demo *demo, struct ANativeWindow* window, int argc, char **argv);
extern void demo_draw(struct demo *demo);

void createSurface(ANativeWindow* window) {
    if (volkInitialize())
    {
        exit(1);
    }
    const char* argv[] = { "cube" };
    int argc = sizeof(argv)/sizeof(char*);
    demo_main_android(&demo, window, argc, (char **)argv);
    //demo_draw(&demo);
}

/*
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
    //info.width = default_width;
    //info.height = default_height;
}
*/
