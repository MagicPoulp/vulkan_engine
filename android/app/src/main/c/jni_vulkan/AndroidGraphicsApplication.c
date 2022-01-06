
#include <stdlib.h>
#include "AndroidGraphicsApplication.h"
#include "string.h"

extern struct demo demo;
extern int demo_main_android(struct demo *demo, struct ANativeWindow* window, int argc, char **argv);
extern void demo_draw(struct demo *demo, double elapsedTimeS);
extern void demo_cleanup(struct demo *demo);
extern void setSizeFull(struct demo *demo, int32_t width, int32_t height);
extern void set_textures_android(AAssetManager *assetManager);

void createSurface(ANativeWindow* window, AAssetManager* assetManager) {
    if (volkInitialize())
    {
        exit(1);
    }
    const char* argv[] = { "cube" };
    int argc = sizeof(argv)/sizeof(char*);
    set_textures_android(assetManager);
    demo_main_android(&demo, window, argc, (char **)argv);
}

void demoDestroy() {
    demo_cleanup(&demo);
}

void drawFrame(double elapsedTimeS) {
    demo_draw(&demo, elapsedTimeS);
}

void setSize(int32_t width, int32_t height) {
    setSizeFull(&demo, width, height);
}
