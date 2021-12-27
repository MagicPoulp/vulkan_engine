#include <util_init.hpp>
#include "AndroidGraphicsApplication.hpp"
#include "cube_data.h"

AndroidGraphicsApplication::AndroidGraphicsApplication(AAssetManager* assetManager, ANativeWindow* window): GraphicsApplication() {
    mAssetManager = assetManager;
    mWindow = window;
}

AndroidGraphicsApplication::~AndroidGraphicsApplication() {
}

// Used to setup shaders.
std::vector<char> AndroidGraphicsApplication::readFile(const std::string& filename) {
    AAsset* file = AAssetManager_open(mAssetManager, filename.c_str(), AASSET_MODE_BUFFER);
    size_t size = AAsset_getLength(file);
    std::vector<char> data(size);
    AAsset_read(file, data.data(), size);
    AAsset_close(file);
    return data;
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
