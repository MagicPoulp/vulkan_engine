//
// Created by Thierry Vilmart on 2022-01-08.
//

#ifndef VULKAN_ENGINE_PROGRAM_H
#define VULKAN_ENGINE_PROGRAM_H

#include "VulkanDSL.h"
#include "assets_management/AssetsFetcher.h"

struct Program
{
    struct VulkanDSL* vulkanDSL;
};

void Program__init(struct Program* self, const char *assetsPath);
struct Program* Program__create(const char *assetsPath);
void Program__reset(struct Program* self);
void Program__destroy(struct Program* self);

#endif //VULKAN_ENGINE_PROGRAM_H
