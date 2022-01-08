//
// Created by Thierry Vilmart on 2022-01-07.
//

#ifndef VULKAN_ENGINE_ASSETSFETCHER_H
#define VULKAN_ENGINE_ASSETSFETCHER_H

struct AssetsFetcher {
#ifdef __ANDROID__
    struct AAssetManager* assetManager;
#endif
};

void AssetsFetcher__loadObj(struct AssetsFetcher* self, const char* filename);

#endif //VULKAN_ENGINE_ASSETSFETCHER_H
