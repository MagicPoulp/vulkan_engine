//
// Created by Thierry Vilmart on 2022-01-07.
//

#ifndef VULKAN_ENGINE_ASSETSFETCHER_H
#define VULKAN_ENGINE_ASSETSFETCHER_H

#define DEMO_TEXTURE_COUNT 1

struct AssetsFetcher {
#ifdef __ANDROID__
    struct AAssetManager* assetManager;
#endif
    // only for png files, and on iOS the background in the header of the PNG is not interpreted
    char** tex_files_short;
    char** meshes_files_short;
};

void AssetsFetcher__init(struct AssetsFetcher* self);
void AssetsFetcher__loadObj(struct AssetsFetcher* self, const char* filename);

#endif //VULKAN_ENGINE_ASSETSFETCHER_H
