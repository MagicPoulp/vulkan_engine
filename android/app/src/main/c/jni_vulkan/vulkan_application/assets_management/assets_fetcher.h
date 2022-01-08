//
// Created by Thierry Vilmart on 2022-01-07.
//

#ifndef VULKAN_ENGINE_ASSETS_FETCHER_H
#define VULKAN_ENGINE_ASSETS_FETCHER_H

struct AssetsFetcher {
#ifdef __ANDROID__
    struct AAssetManager* assetManager;
#endif
};

#ifdef __ANDROID__
void AssetsFetcher__init(struct AssetsFetcher* self, struct AAssetManager* assetManager);
#endif

void AssetsFetcher__loadObj(struct AssetsFetcher* self, const char* filename);

#endif //VULKAN_ENGINE_ASSETS_FETCHER_H
