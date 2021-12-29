#ifndef ANDROID_SURFACE_VIEW_WITH_VULKAN_ANDROIDGRAPHICSAPPLICATION_H
#define ANDROID_SURFACE_VIEW_WITH_VULKAN_ANDROIDGRAPHICSAPPLICATION_H


#include <android/asset_manager.h>
#include <android/native_window.h>
#define VOLK_IMPLEMENTATION
#include "volk_setup.h"

void createSurface(ANativeWindow* window);
void setSize(uint32_t w, uint32_t h);
void drawFrame();

#endif //ANDROID_SURFACE_VIEW_WITH_VULKAN_ANDROIDGRAPHICSAPPLICATION_H
