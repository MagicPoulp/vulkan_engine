#ifndef ANDROID_SURFACE_VIEW_WITH_VULKAN_GRAPHICSAPPLICATION_HPP
#define ANDROID_SURFACE_VIEW_WITH_VULKAN_GRAPHICSAPPLICATION_HPP

#include <vector>
#include <string>
#include <android/asset_manager.h>
#include <android/native_window.h>
#include <util_init.hpp>

/*
To change between Android and iOS

#if defined(__ANDROID__)
#elif defined(VK_USE_PLATFORM_METAL_EXT)
then info has an extra field
void *caMetalLayer;
 */

class GraphicsApplication {
private:
    uint32_t width;
    uint32_t height;
    VkInstance instance;
    VkSurfaceKHR surface;
    struct sample_info info;

public:
    bool isResizeNeeded;

public:
    GraphicsApplication();
    ~GraphicsApplication();
    void createSurface();
    void setSize(uint32_t w, uint32_t h);
    void drawFrame();
    void sampleMain();
    virtual void init_window_size_patched(struct sample_info &info) = 0;
    virtual void init_swapchain_extension_patched(struct sample_info &info) = 0;
};

#endif //ANDROID_SURFACE_VIEW_WITH_VULKAN_GRAPHICSAPPLICATION_HPP
