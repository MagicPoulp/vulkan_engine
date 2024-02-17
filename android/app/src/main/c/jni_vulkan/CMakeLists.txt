# to run manually:
# c; cmake .. -DANDROID_NDK=$ANDROID_NDK
project(vulkan_application)
cmake_minimum_required(VERSION 3.10.2)
# message(WARNING ${ANDROID_NDK})


# https://gist.github.com/evilactually/a0d191701cb48f157b05be7f74d79396
add_custom_target(Main)

# set(MOLTENVK_PATH "/Users/thierryvilmart/VulkanSDK/1.2.198.1/macOS/")
#set(GLSLC "${MOLTENVK_PATH}/bin/glslc")

set(MOLTENVK_PATH "/home/thierry/Android/Sdk/ndk/23.1.7779620/shader-tools/linux-x86_64/")
set(GLSLC "${MOLTENVK_PATH}/glslc")
#set(GLSLC "$ENV{MOLTENVK_PATH}/bin/glslc")
file(GLOB_RECURSE GLSL_SOURCE_FILES
        "vulkan_application/shaders/*.frag"
        "vulkan_application/shaders/*.vert"
        )
message("${GLSL_SOURCE_FILES}")
foreach(GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${PROJECT_BINARY_DIR}/shaders/${FILE_NAME}.spv")
    add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/shaders/"
            COMMAND ${GLSLC} ${GLSL} -o ${SPIRV}
            DEPENDS ${GLSL})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
        Shaders
        DEPENDS ${SPIRV_BINARY_FILES}
)

add_dependencies(Main Shaders)

add_custom_command(TARGET Main POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_SOURCE_DIR}/../../assets/shaders"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_BINARY_DIR}/shaders"
        "${PROJECT_SOURCE_DIR}/../../assets/shaders"
        )

# -----------------------------------------------

include_directories(utils)
include_directories(vulkan_application)
include_directories(assets_management)

include_directories(${ANDROID_NDK}/sources/android/native_app_glue)

if (ANDROID)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_ANDROID_KHR)
elseif()
endif()

include_directories(volk)

add_library(
        jni_vulkan
        SHARED
        VulkanAppBridge.c
        AndroidGraphicsApplication.c
        vulkan_application/VulkanDSL.c
        vulkan_application/Program.c
        vulkan_application/assets_management/AssetsFetcher.c
)

SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -fpermissive -std=c99")

add_subdirectory(volk)
target_link_libraries(jni_vulkan PRIVATE log android volk)
add_dependencies(jni_vulkan Main)
