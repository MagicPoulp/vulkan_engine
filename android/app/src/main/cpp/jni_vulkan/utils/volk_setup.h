//
// Created by thierry on 2021-04-24.
//

#ifndef ANDROID_SURFACE_VIEW_WITH_VULKAN_VOLK_SETUP_H
#define ANDROID_SURFACE_VIEW_WITH_VULKAN_VOLK_SETUP_H

#define VK_NO_PROTOTYPES
#ifndef VK_USE_PLATFORM_ANDROID_KHR
#define VK_USE_PLATFORM_ANDROID_KHR
#endif
#include "../volk/volk.h"

#endif //ANDROID_SURFACE_VIEW_WITH_VULKAN_VOLK_SETUP_H