//
// Created by Thierry Vilmart on 2022-01-07.
//

#ifndef VULKAN_ENGINE_ASSETSFETCHER_H
#define VULKAN_ENGINE_ASSETSFETCHER_H

#include <stddef.h>

#define DEMO_TEXTURE_COUNT 1

struct ObjAsset {
    void* rawData;
    size_t length;
};

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
int AssetsFetcher__LoadObjAndConvert(float bmin[3], float bmax[3], const char* filename, struct ObjAsset* obj);
void get_file_data(
        void* ctx, const char* filename, const int is_mtl,
        const char* obj_filename, char** data, size_t* len);
#endif //VULKAN_ENGINE_ASSETSFETCHER_H
