//
// Created by Thierry Vilmart on 2022-01-08.
//

#include "Program.h"

void Program__init(struct Program* self, const char* assetsPath) {
    struct VulkanDSL* vulkanDSL = (struct VulkanDSL*) calloc(1, sizeof(struct VulkanDSL));
    self->vulkanDSL = vulkanDSL;
    self->vulkanDSL->iosSim = false;
    self->vulkanDSL->validate = false;
    AssetsFetcher__init(&self->vulkanDSL->assetsFetcher, assetsPath);
}

struct Program* Program__create(const char* assetsPath) {
    struct Program* result = (struct Program*) malloc(sizeof(struct Program));
    Program__init(result, assetsPath);
    return result;
}

void Program__reset(struct Program* self) {
    VulkanDSL__cleanup(self->vulkanDSL);
}

void Program__destroy(struct Program* self) {
    if (self) {
        Program__reset(self);
        free(self->vulkanDSL);
        free(self);
    }
}
