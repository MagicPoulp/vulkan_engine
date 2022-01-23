//
// Created by thierry on 2021-04-24.
//

#ifndef VULKAN_ENGINE_VOLK_SETUP_H
#define VULKAN_ENGINE_VOLK_SETUP_H

#define VK_NO_PROTOTYPES
#ifndef VK_USE_PLATFORM_ANDROID_KHR
#define VK_USE_PLATFORM_ANDROID_KHR
#endif
// MoltenVK supports only 1.1
#define VULKAN_1_1
// volk allows not to ship vulkan, or define pointers
#include "../volk/volk.h"

#endif //VULKAN_ENGINE_VOLK_SETUP_H
