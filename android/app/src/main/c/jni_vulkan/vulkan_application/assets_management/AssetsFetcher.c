#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include "AssetsFetcher.h"
#include "utils/tinyobj_loader_c.h"

char *tex_files_short[] = { "home8" };
char *meshes_files_short[] = { "textPanel.obj" };

void AssetsFetcher__init(struct AssetsFetcher* self) {
    self->tex_files_short = tex_files_short;
    self->meshes_files_short = meshes_files_short;
}

void AssetsFetcher__loadObj(struct AssetsFetcher* self, const char* filename) {

}
