#ifndef ANDROID_SURFACE_VIEW_WITH_VULKAN_ANDROIDGRAPHICSAPPLICATION_HPP
#define ANDROID_SURFACE_VIEW_WITH_VULKAN_ANDROIDGRAPHICSAPPLICATION_HPP


#include <vector>
#include <string>
#include <android/asset_manager.h>
#include <android/native_window.h>
#include <util_init.hpp>
#define VOLK_IMPLEMENTATION
#include "volk_setup.hpp"

#include "legacy/GraphicsApplication.hpp" // Base class shared with iOS/macOS/...

class AndroidGraphicsApplication : public GraphicsApplication {
private:
    AAssetManager* mAssetManager;
    ANativeWindow* mWindow;

public:
    AndroidGraphicsApplication(AAssetManager* assetManager, ANativeWindow* window);
    ~AndroidGraphicsApplication();
    std::vector<char> readFile(const std::string& filename);
    void init_window_size_patched(struct sample_info &info) override;
    void init_swapchain_extension_patched(struct sample_info &info) override;
};

#endif //ANDROID_SURFACE_VIEW_WITH_VULKAN_ANDROIDGRAPHICSAPPLICATION_HPP
