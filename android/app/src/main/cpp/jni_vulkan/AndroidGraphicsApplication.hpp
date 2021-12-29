#ifndef ANDROID_SURFACE_VIEW_WITH_VULKAN_ANDROIDGRAPHICSAPPLICATION_HPP
#define ANDROID_SURFACE_VIEW_WITH_VULKAN_ANDROIDGRAPHICSAPPLICATION_HPP


#include <vector>
#include <string>
#include <android/asset_manager.h>
#include <android/native_window.h>
#define VOLK_IMPLEMENTATION
#include "volk_setup.hpp"
extern "C" {
#include "vulkan_application/cube.h"
}

class AndroidGraphicsApplication {
private:
    uint32_t width;
    uint32_t height;
    VkInstance instance;
    VkSurfaceKHR surface;
    struct demo demo;
    ANativeWindow* mWindow;

public:
    AndroidGraphicsApplication(AAssetManager* assetManager, ANativeWindow* window);
    ~AndroidGraphicsApplication();
    void createSurface();
    void setSize(uint32_t w, uint32_t h);
    void drawFrame();
    void init_window_size_patched(struct sample_info &info);
};

#endif //ANDROID_SURFACE_VIEW_WITH_VULKAN_ANDROIDGRAPHICSAPPLICATION_HPP
