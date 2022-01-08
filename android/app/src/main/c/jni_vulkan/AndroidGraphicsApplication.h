#ifndef VULKAN_ENGINE_ANDROIDGRAPHICSAPPLICATION_H
#define VULKAN_ENGINE_ANDROIDGRAPHICSAPPLICATION_H


#include <android/asset_manager.h>
#include <android/native_window.h>

void createSurface(ANativeWindow* window, AAssetManager* assetManager);
void setSize(int32_t width, int32_t height);
void drawFrame(double elapsedTimeS);
void demoDestroy();
void main_android(struct ANativeWindow* window, AAssetManager* assetManager);

#endif //VULKAN_ENGINE_ANDROIDGRAPHICSAPPLICATION_H
