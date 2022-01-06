# to run manually:
# c; cmake .. -DANDROID_NDK=$ANDROID_NDK

cmake_minimum_required(VERSION 3.10.2)
# message(WARNING ${ANDROID_NDK})

include_directories(utils)
include_directories(vulkan_application)

include_directories(${ANDROID_NDK}/sources/android/native_app_glue)

if (ANDROID)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_ANDROID_KHR)
elseif()
endif()

include_directories(volk)

add_library(
        jni_vulkan
        SHARED
        VulkanAppBridge.c AndroidGraphicsApplication.c vulkan_application/cube.c)

SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -fpermissive -std=c99")

add_subdirectory(volk)
target_link_libraries(jni_vulkan PRIVATE log android volk)

