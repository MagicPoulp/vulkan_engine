//
// Created by Thierry Vilmart on 2022-01-08.
//

#include "Program.h"

void Program__init(struct Program* self) {
    struct VulkanDSL* vulkanDSL = (struct VulkanDSL*) calloc(1, sizeof(struct VulkanDSL));
    self->vulkanDSL = vulkanDSL;
    AssetsFetcher__init(&self->vulkanDSL->assetsFetcher);
}

struct Program* Program__create() {
    struct Program* result = (struct Program*) malloc(sizeof(struct Program));
    Program__init(result);
    return result;
}

void Program__reset(struct Program* self) {
    demo_cleanup(self->vulkanDSL);
}

void Program__destroy(struct Program* self) {
    if (self) {
        Program__reset(self);
        free(self->vulkanDSL);
        free(self);
    }
}
