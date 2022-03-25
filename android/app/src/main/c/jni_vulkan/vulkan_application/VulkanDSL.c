/*
The implementation file is in a .h and not a .c to simplify the build on iOS.
The CMakeLists copies to a .c
*/

#include <utils/gettime.h>
#define STB_IMAGE_IMPLEMENTATION
#include "utils/stb_image.h"
#ifdef __ANDROID__
#include <android/native_activity.h>
#include <stdio.h>
#define printf(...) __android_log_print(ANDROID_LOG_DEBUG, "TAG", __VA_ARGS__);
#endif
#include "VulkanDSL.h"
#include "Program.h"


#if defined __ANDROID__
#define VARARGS_WORKS_ON_ANDROID
#include <android/log.h>
#include <android/native_activity.h>
#define ERR_EXIT(err_msg, err_class)                                           \
    do {                                                                       \
        ((void)__android_log_print(ANDROID_LOG_INFO, "Vulkan Cube", err_msg)); \
        exit(1);                                                               \
    } while (0)
#ifdef VARARGS_WORKS_ON_ANDROID
void DbgMsg(const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    __android_log_print(ANDROID_LOG_INFO, "Vulkan Cube", fmt, va);
    va_end(va);
}
#else  // VARARGS_WORKS_ON_ANDROID
#define DbgMsg(fmt, ...)                                                                  \
    do {                                                                                  \
        ((void)__android_log_print(ANDROID_LOG_INFO, "Vulkan Cube", fmt, ##__VA_ARGS__)); \
    } while (0)
#endif  // VARARGS_WORKS_ON_ANDROID
#else
#define ERR_EXIT(err_msg, err_class) \
    do {                             \
        printf("%s\n", err_msg);     \
        fflush(stdout);              \
        exit(1);                     \
    } while (0)
void DbgMsg(char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    vprintf(fmt, va);
    va_end(va);
    fflush(stdout);
}
#endif

extern struct Program *program;

void dumpMatrix(const char *note, mat4x4 MVP) {
    int i;

    printf("%s: \n", note);
    for (i = 0; i < 4; i++) {
        printf("%f, %f, %f, %f\n", MVP[i][0], MVP[i][1], MVP[i][2], MVP[i][3]);
    }
    printf("\n");
    fflush(stdout);
}

void dumpVec4(const char *note, vec4 vector) {
    printf("%s: \n", note);
    printf("%f, %f, %f, %f\n", vector[0], vector[1], vector[2], vector[3]);
    printf("\n");
    fflush(stdout);
}

VKAPI_ATTR VkBool32 VKAPI_CALL debug_messenger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                        void *pUserData) {
    char prefix[64] = "";
    char *message = (char *)malloc(strlen(pCallbackData->pMessage) + 5000);
    assert(message);
    struct VulkanDSL *vulkanDSL = (struct VulkanDSL *)pUserData;

    if (vulkanDSL->use_break) {
#ifndef WIN32
        raise(SIGTRAP);
#else
        DebugBreak();
#endif
    }

    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        strcat(prefix, "VERBOSE : ");
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        strcat(prefix, "INFO : ");
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        strcat(prefix, "WARNING : ");
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        strcat(prefix, "ERROR : ");
    }

    if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
        strcat(prefix, "GENERAL");
    } else {
        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
            strcat(prefix, "VALIDATION");
            validation_error = 1;
        }
        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
            if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
                strcat(prefix, "|");
            }
            strcat(prefix, "PERFORMANCE");
        }
    }

    sprintf(message, "%s - Message Id Number: %d | Message Id Name: %s\n\t%s\n", prefix, pCallbackData->messageIdNumber,
            pCallbackData->pMessageIdName, pCallbackData->pMessage);
    if (pCallbackData->objectCount > 0) {
        char tmp_message[500];
        sprintf(tmp_message, "\n\tObjects - %d\n", pCallbackData->objectCount);
        strcat(message, tmp_message);
        for (uint32_t object = 0; object < pCallbackData->objectCount; ++object) {
            if (NULL != pCallbackData->pObjects[object].pObjectName && strlen(pCallbackData->pObjects[object].pObjectName) > 0) {
                sprintf(tmp_message, "\t\tObject[%d] - %s, Handle %p, Name \"%s\"\n", object,
                        string_VkObjectType(pCallbackData->pObjects[object].objectType),
                        (void *)(pCallbackData->pObjects[object].objectHandle), pCallbackData->pObjects[object].pObjectName);
            } else {
                sprintf(tmp_message, "\t\tObject[%d] - %s, Handle %p\n", object,
                        string_VkObjectType(pCallbackData->pObjects[object].objectType),
                        (void *)(pCallbackData->pObjects[object].objectHandle));
            }
            strcat(message, tmp_message);
        }
    }
    if (pCallbackData->cmdBufLabelCount > 0) {
        char tmp_message[500];
        sprintf(tmp_message, "\n\tCommand Buffer Labels - %d\n", pCallbackData->cmdBufLabelCount);
        strcat(message, tmp_message);
        for (uint32_t cmd_buf_label = 0; cmd_buf_label < pCallbackData->cmdBufLabelCount; ++cmd_buf_label) {
            sprintf(tmp_message, "\t\tLabel[%d] - %s { %f, %f, %f, %f}\n", cmd_buf_label,
                    pCallbackData->pCmdBufLabels[cmd_buf_label].pLabelName, pCallbackData->pCmdBufLabels[cmd_buf_label].color[0],
                    pCallbackData->pCmdBufLabels[cmd_buf_label].color[1], pCallbackData->pCmdBufLabels[cmd_buf_label].color[2],
                    pCallbackData->pCmdBufLabels[cmd_buf_label].color[3]);
            strcat(message, tmp_message);
        }
    }

#ifdef _WIN32

    in_callback = true;
    if (!vulkanDSL->suppress_popups) MessageBox(NULL, message, "Alert", MB_OK);
    in_callback = false;

#elif defined(ANDROID)

    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        __android_log_print(ANDROID_LOG_INFO, APP_SHORT_NAME, "%s", message);
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        __android_log_print(ANDROID_LOG_WARN, APP_SHORT_NAME, "%s", message);
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        __android_log_print(ANDROID_LOG_ERROR, APP_SHORT_NAME, "%s", message);
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        __android_log_print(ANDROID_LOG_VERBOSE, APP_SHORT_NAME, "%s", message);
    } else {
        __android_log_print(ANDROID_LOG_INFO, APP_SHORT_NAME, "%s", message);
    }

#else

    printf("%s\n", message);
    fflush(stdout);

#endif

    free(message);

    // Don't bail out, but keep going.
    return false;
}

bool ActualTimeLate(uint64_t desired, uint64_t actual, uint64_t rdur) {
    // The desired time was the earliest time that the present should have
    // occured.  In almost every case, the actual time should be later than the
    // desired time.  We should only consider the actual time "late" if it is
    // after "desired + rdur".
    if (actual <= desired) {
        // The actual time was before or equal to the desired time.  This will
        // probably never happen, but in case it does, return false since the
        // present was obviously NOT late.
        return false;
    }
    uint64_t deadline = desired + rdur;
    if (actual > deadline) {
        return true;
    } else {
        return false;
    }
}
bool CanPresentEarlier(uint64_t earliest, uint64_t actual, uint64_t margin, uint64_t rdur) {
    if (earliest < actual) {
        // Consider whether this present could have occured earlier.  Make sure
        // that earliest time was at least 2msec earlier than actual time, and
        // that the margin was at least 2msec:
        uint64_t diff = actual - earliest;
        if ((diff >= (2 * MILLION)) && (margin >= (2 * MILLION))) {
            // This present could have occured earlier because both: 1) the
            // earliest time was at least 2 msec before actual time, and 2) the
            // margin was at least 2msec.
            return true;
        }
    }
    return false;
}

// Forward declarations:
void demo_resize(struct VulkanDSL *vulkanDSL);
static void demo_create_surface(struct VulkanDSL *vulkanDSL);

// modified to return the memory that has the least flags besides the required ones
static bool memory_type_from_properties(struct VulkanDSL *vulkanDSL, uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex) {
    bool result = false;
    VkMemoryPropertyFlagBits minimalFlagsFound = VK_MEMORY_PROPERTY_FLAG_BITS_MAX_ENUM;
    for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++) {
        if ((typeBits & 1) == 1) { // a compatible memory index with the structure to use
            // Type is available, does it match user properties?
            if ((vulkanDSL->memory_properties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask) {
                if (vulkanDSL->memory_properties.memoryTypes[i].propertyFlags < minimalFlagsFound) {
                    minimalFlagsFound = vulkanDSL->memory_properties.memoryTypes[i].propertyFlags;
                    *typeIndex = i;
                }
                result = true;
            }
        }
        typeBits >>= 1; // an array of bits representing all compatible memory indexes
    }
    // No memory types matched, return failure
    return result;
}

static bool memory_type_from_properties_legacy(struct VulkanDSL *vulkanDSL, uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex) {
    // Search memtypes to find first index with those properties
    for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++) {
        if (i == 1 && * typeIndex == -1) continue;
        if ((typeBits & 1) == 1) {
            // Type is available, does it match user properties?
            if ((vulkanDSL->memory_properties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask) {
                *typeIndex = i;
                return true;
            }
        }
        typeBits >>= 1;
    }
    // No memory types matched, return failure
    return false;
}

static void demo_flush_init_cmd(struct VulkanDSL *vulkanDSL) {
    VkResult U_ASSERT_ONLY err;

    // This function could get called twice if the texture uses a staging buffer
    // In that case the second call should be ignored
    if (vulkanDSL->cmd == VK_NULL_HANDLE) return;

    err = vkEndCommandBuffer(vulkanDSL->cmd);
    assert(!err);

    VkFence fence;
    VkFenceCreateInfo fence_ci = {.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .pNext = NULL, .flags = 0};
    err = vkCreateFence(vulkanDSL->device, &fence_ci, NULL, &fence);
    assert(!err);

    const VkCommandBuffer cmd_bufs[] = {vulkanDSL->cmd};
    VkSubmitInfo submit_info = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = NULL,
            .waitSemaphoreCount = 0,
            .pWaitSemaphores = NULL,
            .pWaitDstStageMask = NULL,
            .commandBufferCount = 1,
            .pCommandBuffers = cmd_bufs,
            .signalSemaphoreCount = 0,
            .pSignalSemaphores = NULL
    };

    err = vkQueueSubmit(vulkanDSL->graphics_queue, 1, &submit_info, fence);
    assert(!err);

    err = vkWaitForFences(vulkanDSL->device, 1, &fence, VK_TRUE, UINT64_MAX);
    assert(!err);
    vkDeviceWaitIdle(vulkanDSL->device);
    vkFreeCommandBuffers(vulkanDSL->device, vulkanDSL->cmd_pool, 1, cmd_bufs);
    vkDestroyFence(vulkanDSL->device, fence, NULL);
    vulkanDSL->cmd = VK_NULL_HANDLE;
}

static void demo_set_image_layout(struct VulkanDSL *vulkanDSL, VkImage image, VkImageAspectFlags aspectMask, VkImageLayout old_image_layout,
                                  VkImageLayout new_image_layout, VkAccessFlagBits srcAccessMask, VkPipelineStageFlags src_stages,
                                  VkPipelineStageFlags dest_stages) {
    assert(vulkanDSL->cmd);

    VkImageMemoryBarrier image_memory_barrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext = NULL,
            .srcAccessMask = srcAccessMask,
            .dstAccessMask = 0,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .oldLayout = old_image_layout,
            .newLayout = new_image_layout,
            .image = image,
            .subresourceRange = {aspectMask, 0, 1, 0, 1}};

    switch (new_image_layout) {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            /* Make sure anything that was copying from this image has completed */
            image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            image_memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            image_memory_barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            image_memory_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            break;

        default:
            image_memory_barrier.dstAccessMask = 0;
            break;
    }

    VkImageMemoryBarrier *pmemory_barrier = &image_memory_barrier;

    // https://www.khronos.org/blog/understanding-vulkan-synchronization
    vkCmdPipelineBarrier(vulkanDSL->cmd, src_stages, dest_stages, 0, 0, NULL, 0, NULL, 1, pmemory_barrier);
}

void VulkanDSL__draw_build_cmd(struct VulkanDSL *vulkanDSL, VkCommandBuffer cmd_buf) {
    VkDebugUtilsLabelEXT label;
    memset(&label, 0, sizeof(label));
    const VkCommandBufferBeginInfo cmd_buf_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = NULL,
            .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
            .pInheritanceInfo = NULL,
    };
    const VkClearValue clear_values[4] = {
  //          [0] = {.color.float32 = {0.2f, 0.2f, 0.2f, 0.2f}},
            [0] = {.color.float32 = {0.0f, 0.0f, 0.0f, 0.0f}},
  //          [0] = {.color.float32 = {0.0f, 0.0f, 0.0f, 1.0f}},
            [1] = {.depthStencil = {1.0f, 0}},
  //          [2] = {.color.float32 = {0.2f, 0.2f, 0.2f, 0.2f}},
            [2] = {.color.float32 = {0.0f, 0.0f, 0.0f, 0.0f}},
    };
    const VkRenderPassBeginInfo rp_begin = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = NULL,
            .renderPass = vulkanDSL->render_pass,
            .framebuffer = vulkanDSL->swapchain_image_resources[vulkanDSL->current_buffer].framebuffer,
            .renderArea.offset.x = 0,
            .renderArea.offset.y = 0,
            .renderArea.extent.width = vulkanDSL->width,
            .renderArea.extent.height = vulkanDSL->height,
            .clearValueCount = 3,
            //.clearValueCount = 1,
            .pClearValues = clear_values,
    };
    VkResult U_ASSERT_ONLY err;

    err = vkBeginCommandBuffer(cmd_buf, &cmd_buf_info);

    if (vulkanDSL->validate) {
        // Set a name for the command buffer
        VkDebugUtilsObjectNameInfoEXT cmd_buf_name = {
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = NULL,
                .objectType = VK_OBJECT_TYPE_COMMAND_BUFFER,
                .objectHandle = (uint64_t)cmd_buf,
                .pObjectName = "CubeDrawCommandBuf",
        };
        vulkanDSL->SetDebugUtilsObjectNameEXT(vulkanDSL->device, &cmd_buf_name);

        label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        label.pNext = NULL;
        label.pLabelName = "DrawBegin";
        label.color[0] = 0.4f;
        label.color[1] = 0.3f;
        label.color[2] = 0.2f;
        label.color[3] = 0.1f;
        vulkanDSL->CmdBeginDebugUtilsLabelEXT(cmd_buf, &label);
    }

    assert(!err);
    vkCmdBeginRenderPass(cmd_buf, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

    if (vulkanDSL->validate) {
        label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        label.pNext = NULL;
        label.pLabelName = "InsideRenderPass";
        label.color[0] = 8.4f;
        label.color[1] = 7.3f;
        label.color[2] = 6.2f;
        label.color[3] = 7.1f;
        vulkanDSL->CmdBeginDebugUtilsLabelEXT(cmd_buf, &label);
    }

    vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanDSL->pipeline);
    vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanDSL->pipeline_layout, 0, 1,
                            &vulkanDSL->swapchain_image_resources[vulkanDSL->current_buffer].descriptor_set, 0, NULL);
    VkViewport viewport;
    memset(&viewport, 0, sizeof(viewport));
    viewport.height = vulkanDSL->height;
    viewport.width = vulkanDSL->width;
    viewport.minDepth = (float)0.0f;
    viewport.maxDepth = (float)1.0f;
    vkCmdSetViewport(cmd_buf, 0, 1, &viewport);

    VkRect2D scissor;
    memset(&scissor, 0, sizeof(scissor));
    scissor.extent.width = vulkanDSL->width;
    scissor.extent.height = vulkanDSL->height;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    vkCmdSetScissor(cmd_buf, 0, 1, &scissor);

    if (vulkanDSL->validate) {
        label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        label.pNext = NULL;
        label.pLabelName = "ActualDraw";
        label.color[0] = -0.4f;
        label.color[1] = -0.3f;
        label.color[2] = -0.2f;
        label.color[3] = -0.1f;
        vulkanDSL->CmdBeginDebugUtilsLabelEXT(cmd_buf, &label);
    }

    VkBuffer *vertexBuffers = &vulkanDSL->vertex_buffer_resources->vertex_buffer_gpu;
    //VkBuffer vertexBuffers[] = { vulkanDSL->vertex_buffer_resources->vertex_buffer };
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmd_buf, 0, 1, vertexBuffers, offsets);
    vkCmdDraw(cmd_buf, (uint32_t)vulkanDSL->assetsFetcher.vertexCount, 1, 0, 0);

    if (vulkanDSL->validate) {
        vulkanDSL->CmdEndDebugUtilsLabelEXT(cmd_buf);
    }

    // Note that ending the renderpass changes the image's layout from
    // COLOR_ATTACHMENT_OPTIMAL to PRESENT_SRC_KHR
    vkCmdEndRenderPass(cmd_buf);
    if (vulkanDSL->validate) {
        vulkanDSL->CmdEndDebugUtilsLabelEXT(cmd_buf);
    }

    if (vulkanDSL->separate_present_queue) {
        // We have to transfer ownership from the graphics queue family to the
        // present queue family to be able to present.  Note that we don't have
        // to transfer from present queue family back to graphics queue family at
        // the start of the next frame because we don't care about the image's
        // contents at that point.
        VkImageMemoryBarrier image_ownership_barrier = {.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .pNext = NULL,
                .srcAccessMask = 0,
                .dstAccessMask = 0,
                .oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                .srcQueueFamilyIndex = vulkanDSL->graphics_queue_family_index,
                .dstQueueFamilyIndex = vulkanDSL->present_queue_family_index,
                .image = vulkanDSL->swapchain_image_resources[vulkanDSL->current_buffer].image,
                .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};

        vkCmdPipelineBarrier(cmd_buf, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0,
                             NULL, 1, &image_ownership_barrier);
    }
    if (vulkanDSL->validate) {
        vulkanDSL->CmdEndDebugUtilsLabelEXT(cmd_buf);
    }
    err = vkEndCommandBuffer(cmd_buf);
    assert(!err);
}

void demo_build_image_ownership_cmd(struct VulkanDSL *vulkanDSL, int i) {
    VkResult U_ASSERT_ONLY err;

    const VkCommandBufferBeginInfo cmd_buf_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = NULL,
            .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
            .pInheritanceInfo = NULL,
    };
    err = vkBeginCommandBuffer(vulkanDSL->swapchain_image_resources[i].graphics_to_present_cmd, &cmd_buf_info);
    assert(!err);

    VkImageMemoryBarrier image_ownership_barrier = {.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext = NULL,
            .srcAccessMask = 0,
            .dstAccessMask = 0,
            .oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .srcQueueFamilyIndex = vulkanDSL->graphics_queue_family_index,
            .dstQueueFamilyIndex = vulkanDSL->present_queue_family_index,
            .image = vulkanDSL->swapchain_image_resources[i].image,
            .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};

    vkCmdPipelineBarrier(vulkanDSL->swapchain_image_resources[i].graphics_to_present_cmd, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0, NULL, 1, &image_ownership_barrier);
    err = vkEndCommandBuffer(vulkanDSL->swapchain_image_resources[i].graphics_to_present_cmd);
    assert(!err);
}

float accumulatedAngle = 0;
void demo_update_data_buffer(struct VulkanDSL *vulkanDSL, double elapsedTimeS) {
    mat4x4 MVP, Model, VP;
    int matrixSize = sizeof(MVP);
    mat4x4_mul(VP, vulkanDSL->projection_matrix, vulkanDSL->view_matrix);
    mat4x4_dup(Model, vulkanDSL->model_matrix);
    // degrees per second
    float movedAngle = vulkanDSL->spin_angle * elapsedTimeS;
    accumulatedAngle = fmodf(accumulatedAngle + movedAngle, 360);
    mat4x4_rotate_Z(Model, vulkanDSL->model_matrix, (float)degreesToRadians(accumulatedAngle));
    mat4x4_mul(MVP, VP, Model);
    memcpy(vulkanDSL->swapchain_image_resources[vulkanDSL->current_buffer].uniform_memory_ptr, (const void *)&MVP[0][0], matrixSize);
}

void DemoUpdateTargetIPD(struct VulkanDSL *vulkanDSL) {
    // Look at what happened to previous presents, and make appropriate
    // adjustments in timing:
    VkResult U_ASSERT_ONLY err;
    VkPastPresentationTimingGOOGLE *past = NULL;
    uint32_t count = 0;

    err = vulkanDSL->fpGetPastPresentationTimingGOOGLE(vulkanDSL->device, vulkanDSL->swapchain, &count, NULL);
    assert(!err);
    if (count) {
        past = (VkPastPresentationTimingGOOGLE *)malloc(sizeof(VkPastPresentationTimingGOOGLE) * count);
        assert(past);
        err = vulkanDSL->fpGetPastPresentationTimingGOOGLE(vulkanDSL->device, vulkanDSL->swapchain, &count, past);
        assert(!err);

        bool early = false;
        bool late = false;
        bool calibrate_next = false;
        for (uint32_t i = 0; i < count; i++) {
            if (!vulkanDSL->syncd_with_actual_presents) {
                // This is the first time that we've received an
                // actualPresentTime for this swapchain.  In order to not
                // perceive these early frames as "late", we need to sync-up
                // our future desiredPresentTime's with the
                // actualPresentTime(s) that we're receiving now.
                calibrate_next = true;

                // So that we don't suspect any pending presents as late,
                // record them all as suspected-late presents:
                vulkanDSL->last_late_id = vulkanDSL->next_present_id - 1;
                vulkanDSL->last_early_id = 0;
                vulkanDSL->syncd_with_actual_presents = true;
                break;
            } else if (CanPresentEarlier(past[i].earliestPresentTime, past[i].actualPresentTime, past[i].presentMargin,
                                         vulkanDSL->refresh_duration)) {
                // This image could have been presented earlier.  We don't want
                // to decrease the target_IPD until we've seen early presents
                // for at least two seconds.
                if (vulkanDSL->last_early_id == past[i].presentID) {
                    // We've now seen two seconds worth of early presents.
                    // Flag it as such, and reset the counter:
                    early = true;
                    vulkanDSL->last_early_id = 0;
                } else if (vulkanDSL->last_early_id == 0) {
                    // This is the first early present we've seen.
                    // Calculate the presentID for two seconds from now.
                    uint64_t lastEarlyTime = past[i].actualPresentTime + (2 * BILLION);
                    uint32_t howManyPresents = (uint32_t)((lastEarlyTime - past[i].actualPresentTime) / vulkanDSL->target_IPD);
                    vulkanDSL->last_early_id = past[i].presentID + howManyPresents;
                } else {
                    // We are in the midst of a set of early images,
                    // and so we won't do anything.
                }
                late = false;
                vulkanDSL->last_late_id = 0;
            } else if (ActualTimeLate(past[i].desiredPresentTime, past[i].actualPresentTime, vulkanDSL->refresh_duration)) {
                // This image was presented after its desired time.  Since
                // there's a delay between calling vkQueuePresentKHR and when
                // we get the timing data, several presents may have been late.
                // Thus, we need to threat all of the outstanding presents as
                // being likely late, so that we only increase the target_IPD
                // once for all of those presents.
                if ((vulkanDSL->last_late_id == 0) || (vulkanDSL->last_late_id < past[i].presentID)) {
                    late = true;
                    // Record the last suspected-late present:
                    vulkanDSL->last_late_id = vulkanDSL->next_present_id - 1;
                } else {
                    // We are in the midst of a set of likely-late images,
                    // and so we won't do anything.
                }
                early = false;
                vulkanDSL->last_early_id = 0;
            } else {
                // Since this image was not presented early or late, reset
                // any sets of early or late presentIDs:
                early = false;
                late = false;
                calibrate_next = true;
                vulkanDSL->last_early_id = 0;
                vulkanDSL->last_late_id = 0;
            }
        }

        if (early) {
            // Since we've seen at least two-seconds worth of presnts that
            // could have occured earlier than desired, let's decrease the
            // target_IPD (i.e. increase the frame rate):
            //
            // TODO(ianelliott): Try to calculate a better target_IPD based
            // on the most recently-seen present (this is overly-simplistic).
            vulkanDSL->refresh_duration_multiplier--;
            if (vulkanDSL->refresh_duration_multiplier == 0) {
                // This should never happen, but in case it does, don't
                // try to go faster.
                vulkanDSL->refresh_duration_multiplier = 1;
            }
            vulkanDSL->target_IPD = vulkanDSL->refresh_duration * vulkanDSL->refresh_duration_multiplier;
        }
        if (late) {
            // Since we found a new instance of a late present, we want to
            // increase the target_IPD (i.e. decrease the frame rate):
            //
            // TODO(ianelliott): Try to calculate a better target_IPD based
            // on the most recently-seen present (this is overly-simplistic).
            vulkanDSL->refresh_duration_multiplier++;
            vulkanDSL->target_IPD = vulkanDSL->refresh_duration * vulkanDSL->refresh_duration_multiplier;
        }

        if (calibrate_next) {
            int64_t multiple = vulkanDSL->next_present_id - past[count - 1].presentID;
            vulkanDSL->prev_desired_present_time = (past[count - 1].actualPresentTime + (multiple * vulkanDSL->target_IPD));
        }
        free(past);
    }
}

void demo_draw(struct VulkanDSL *vulkanDSL, double elapsedTimeS) {
    VkResult U_ASSERT_ONLY err;

    // Ensure no more than FRAME_LAG renderings are outstanding
    VkResult res = vkWaitForFences(vulkanDSL->device, 1, &vulkanDSL->fences[vulkanDSL->frame_index], VK_TRUE, UINT64_MAX);
    vkResetFences(vulkanDSL->device, 1, &vulkanDSL->fences[vulkanDSL->frame_index]);

    do {
        // Get the index of the next available swapchain image:
        err =
                vulkanDSL->fpAcquireNextImageKHR(
                        vulkanDSL->device, vulkanDSL->swapchain, UINT64_MAX,
                        vulkanDSL->image_acquired_semaphores[vulkanDSL->frame_index], VK_NULL_HANDLE, &vulkanDSL->current_buffer);

        if (err == VK_ERROR_OUT_OF_DATE_KHR) {
            // vulkanDSL->swapchain is out of date (e.g. the window was resized) and
            // must be recreated:
            demo_resize(vulkanDSL);
        } else if (err == VK_SUBOPTIMAL_KHR) {
            // vulkanDSL->swapchain is not as optimal as it could be, but the platform's
            // presentation engine will still present the image correctly.
            break;
        } else if (err == VK_ERROR_SURFACE_LOST_KHR) {
            vkDestroySurfaceKHR(vulkanDSL->inst, vulkanDSL->surface, NULL);
            demo_create_surface(vulkanDSL);
            demo_resize(vulkanDSL);
        } else {
            assert(!err);
        }
    } while (err != VK_SUCCESS);

    demo_update_data_buffer(vulkanDSL, elapsedTimeS);

    if (vulkanDSL->VK_GOOGLE_display_timing_enabled) {
        // Look at what happened to previous presents, and make appropriate
        // adjustments in timing:
        DemoUpdateTargetIPD(vulkanDSL);

        // Note: a real application would position its geometry to that it's in
        // the correct location for when the next image is presented.  It might
        // also wait, so that there's less latency between any input and when
        // the next image is rendered/presented.  This demo program is so
        // simple that it doesn't do either of those.
    }

    // Wait for the image acquired semaphore to be signaled to ensure
    // that the image won't be rendered to until the presentation
    // engine has fully released ownership to the application, and it is
    // okay to render to the image.
    VkPipelineStageFlags pipe_stage_flags;
    VkSubmitInfo submit_info;
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = NULL;
    pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    submit_info.pWaitDstStageMask = &pipe_stage_flags;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &vulkanDSL->image_acquired_semaphores[vulkanDSL->frame_index];
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &vulkanDSL->swapchain_image_resources[vulkanDSL->current_buffer].cmd;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &vulkanDSL->draw_complete_semaphores[vulkanDSL->frame_index];
    err = vkQueueSubmit(vulkanDSL->graphics_queue, 1, &submit_info, vulkanDSL->fences[vulkanDSL->frame_index]);
    assert(!err);

    if (vulkanDSL->separate_present_queue) {
        // If we are using separate queues, change image ownership to the
        // present queue before presenting, waiting for the draw complete
        // semaphore and signalling the ownership released semaphore when finished
        VkFence nullFence = VK_NULL_HANDLE;
        pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &vulkanDSL->draw_complete_semaphores[vulkanDSL->frame_index];
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &vulkanDSL->swapchain_image_resources[vulkanDSL->current_buffer].graphics_to_present_cmd;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &vulkanDSL->image_ownership_semaphores[vulkanDSL->frame_index];
        err = vkQueueSubmit(vulkanDSL->present_queue, 1, &submit_info, nullFence);
        assert(!err);
    }

    // If we are using separate queues we have to wait for image ownership,
    // otherwise wait for draw complete
    VkPresentInfoKHR present = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = NULL,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = (vulkanDSL->separate_present_queue) ? &vulkanDSL->image_ownership_semaphores[vulkanDSL->frame_index]
                                                              : &vulkanDSL->draw_complete_semaphores[vulkanDSL->frame_index],
            .swapchainCount = 1,
            .pSwapchains = &vulkanDSL->swapchain,
            .pImageIndices = &vulkanDSL->current_buffer,
    };

    VkRectLayerKHR rect;
    VkPresentRegionKHR region;
    VkPresentRegionsKHR regions;
    if (vulkanDSL->VK_KHR_incremental_present_enabled) {
        // If using VK_KHR_incremental_present, we provide a hint of the region
        // that contains changed content relative to the previously-presented
        // image.  The implementation can use this hint in order to save
        // work/power (by only copying the region in the hint).  The
        // implementation is free to ignore the hint though, and so we must
        // ensure that the entire image has the correctly-drawn content.
        uint32_t eighthOfWidth = vulkanDSL->width / 8;
        uint32_t eighthOfHeight = vulkanDSL->height / 8;

        rect.offset.x = eighthOfWidth;
        rect.offset.y = eighthOfHeight;
        rect.extent.width = eighthOfWidth * 6;
        rect.extent.height = eighthOfHeight * 6;
        rect.layer = 0;

        region.rectangleCount = 1;
        region.pRectangles = &rect;

        regions.sType = VK_STRUCTURE_TYPE_PRESENT_REGIONS_KHR;
        regions.pNext = present.pNext;
        regions.swapchainCount = present.swapchainCount;
        regions.pRegions = &region;
        present.pNext = &regions;
    }

    if (vulkanDSL->VK_GOOGLE_display_timing_enabled) {
        VkPresentTimeGOOGLE ptime;

        if (vulkanDSL->prev_desired_present_time == 0) {
            // This must be the first present for this swapchain.
            //
            // We don't know where we are relative to the presentation engine's
            // display's refresh cycle.  We also don't know how long rendering
            // takes.  Let's make a grossly-simplified assumption that the
            // desiredPresentTime should be half way between now and
            // now+target_IPD.  We will adjust over time.
            uint64_t curtime = getTimeInNanoseconds();
            if (curtime == 0) {
                // Since we didn't find out the current time, don't give a
                // desiredPresentTime:
                ptime.desiredPresentTime = 0;
            } else {
                ptime.desiredPresentTime = curtime + (vulkanDSL->target_IPD >> 1);
            }
        } else {
            ptime.desiredPresentTime = (vulkanDSL->prev_desired_present_time + vulkanDSL->target_IPD);
        }

        //ptime.desiredPresentTime = (uint64_t)(elapsedTimeS * 1000000000);
        ptime.presentID = vulkanDSL->next_present_id++;
        vulkanDSL->prev_desired_present_time = ptime.desiredPresentTime;

        VkPresentTimesInfoGOOGLE present_time = {
                .sType = VK_STRUCTURE_TYPE_PRESENT_TIMES_INFO_GOOGLE,
                .pNext = present.pNext,
                .swapchainCount = present.swapchainCount,
                .pTimes = &ptime,
        };
        if (vulkanDSL->VK_GOOGLE_display_timing_enabled) {
            present.pNext = &present_time;
        }
    }

    err = vulkanDSL->fpQueuePresentKHR(vulkanDSL->present_queue, &present);
    vulkanDSL->frame_index += 1;
    vulkanDSL->frame_index %= FRAME_LAG;

    if (err == VK_ERROR_OUT_OF_DATE_KHR) {
        // vulkanDSL->swapchain is out of date (e.g. the window was resized) and
        // must be recreated:
        demo_resize(vulkanDSL);
    } else if (err == VK_SUBOPTIMAL_KHR) {
        // SUBOPTIMAL could be due to a resize
        VkSurfaceCapabilitiesKHR surfCapabilities;
        err = vulkanDSL->fpGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkanDSL->gpu, vulkanDSL->surface, &surfCapabilities);
        assert(!err);
        if (surfCapabilities.currentExtent.width != (uint32_t)vulkanDSL->width ||
            surfCapabilities.currentExtent.height != (uint32_t)vulkanDSL->height) {
            demo_resize(vulkanDSL);
        }
    } else if (err == VK_ERROR_SURFACE_LOST_KHR) {
        vkDestroySurfaceKHR(vulkanDSL->inst, vulkanDSL->surface, NULL);
        demo_create_surface(vulkanDSL);
        demo_resize(vulkanDSL);
    } else {
        assert(!err);
    }
}

static void demo_prepare_buffers(struct VulkanDSL *vulkanDSL) {
    VkResult U_ASSERT_ONLY err;
    VkSwapchainKHR oldSwapchain = vulkanDSL->swapchain;

    // Check the surface capabilities and formats
    VkSurfaceCapabilitiesKHR surfCapabilities;
    err = vulkanDSL->fpGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkanDSL->gpu, vulkanDSL->surface, &surfCapabilities);
    assert(!err);

    uint32_t presentModeCount;
    err = vulkanDSL->fpGetPhysicalDeviceSurfacePresentModesKHR(vulkanDSL->gpu, vulkanDSL->surface, &presentModeCount, NULL);
    assert(!err);
    VkPresentModeKHR *presentModes = (VkPresentModeKHR *)malloc(presentModeCount * sizeof(VkPresentModeKHR));
    assert(presentModes);
    err = vulkanDSL->fpGetPhysicalDeviceSurfacePresentModesKHR(vulkanDSL->gpu, vulkanDSL->surface, &presentModeCount, presentModes);
    assert(!err);

    VkExtent2D swapchainExtent;
    // width and height are either both 0xFFFFFFFF, or both not 0xFFFFFFFF.
    if (surfCapabilities.currentExtent.width == 0xFFFFFFFF) {
        // If the surface size is undefined, the size is set to the size
        // of the images requested, which must fit within the minimum and
        // maximum values.
        swapchainExtent.width = vulkanDSL->width;
        swapchainExtent.height = vulkanDSL->height;

        if (swapchainExtent.width < surfCapabilities.minImageExtent.width) {
            swapchainExtent.width = surfCapabilities.minImageExtent.width;
        } else if (swapchainExtent.width > surfCapabilities.maxImageExtent.width) {
            swapchainExtent.width = surfCapabilities.maxImageExtent.width;
        }

        if (swapchainExtent.height < surfCapabilities.minImageExtent.height) {
            swapchainExtent.height = surfCapabilities.minImageExtent.height;
        } else if (swapchainExtent.height > surfCapabilities.maxImageExtent.height) {
            swapchainExtent.height = surfCapabilities.maxImageExtent.height;
        }
    } else {
        // If the surface size is defined, the swap chain size must match
        swapchainExtent = surfCapabilities.currentExtent;
        vulkanDSL->width = surfCapabilities.currentExtent.width;
        vulkanDSL->height = surfCapabilities.currentExtent.height;
        demo_init_matrices(vulkanDSL, vulkanDSL->width, vulkanDSL->height);
    }

    if (surfCapabilities.maxImageExtent.width == 0 || surfCapabilities.maxImageExtent.height == 0) {
        vulkanDSL->is_minimized = true;
        return;
    } else {
        vulkanDSL->is_minimized = false;
    }

    // The FIFO present mode is guaranteed by the spec to be supported
    // and to have no tearing.  It's a great default present mode to use.
    VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

    //  There are times when you may wish to use another present mode.  The
    //  following code shows how to select them, and the comments provide some
    //  reasons you may wish to use them.
    //
    // It should be noted that Vulkan 1.0 doesn't provide a method for
    // synchronizing rendering with the presentation engine's display.  There
    // is a method provided for throttling rendering with the display, but
    // there are some presentation engines for which this method will not work.
    // If an application doesn't throttle its rendering, and if it renders much
    // faster than the refresh rate of the display, this can waste power on
    // mobile devices.  That is because power is being spent rendering images
    // that may never be seen.

    // VK_PRESENT_MODE_IMMEDIATE_KHR is for applications that don't care about
    // tearing, or have some way of synchronizing their rendering with the
    // display.
    // VK_PRESENT_MODE_MAILBOX_KHR may be useful for applications that
    // generally render a new presentable image every refresh cycle, but are
    // occasionally early.  In this case, the application wants the new image
    // to be displayed instead of the previously-queued-for-presentation image
    // that has not yet been displayed.
    // VK_PRESENT_MODE_FIFO_RELAXED_KHR is for applications that generally
    // render a new presentable image every refresh cycle, but are occasionally
    // late.  In this case (perhaps because of stuttering/latency concerns),
    // the application wants the late image to be immediately displayed, even
    // though that may mean some tearing.

    if (vulkanDSL->presentMode != swapchainPresentMode) {
        for (size_t i = 0; i < presentModeCount; ++i) {
            if (presentModes[i] == vulkanDSL->presentMode) {
                swapchainPresentMode = vulkanDSL->presentMode;
                break;
            }
        }
    }
    if (swapchainPresentMode != vulkanDSL->presentMode) {
        ERR_EXIT("Present mode specified is not supported\n", "Present mode unsupported");
    }

    // Determine the number of VkImages to use in the swap chain.
    // Application desires to acquire 3 images at a time for triple
    // buffering
    uint32_t desiredNumOfSwapchainImages = 3;
    if (desiredNumOfSwapchainImages < surfCapabilities.minImageCount) {
        desiredNumOfSwapchainImages = surfCapabilities.minImageCount;
    }
    // If maxImageCount is 0, we can ask for as many images as we want;
    // otherwise we're limited to maxImageCount
    if ((surfCapabilities.maxImageCount > 0) && (desiredNumOfSwapchainImages > surfCapabilities.maxImageCount)) {
        // Application must settle for fewer images than desired:
        desiredNumOfSwapchainImages = surfCapabilities.maxImageCount;
    }

    VkSurfaceTransformFlagsKHR preTransform;
    if (surfCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
        preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    } else {
        preTransform = surfCapabilities.currentTransform;
    }

    // Find a supported composite alpha mode - one of these is guaranteed to be set
    VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    VkCompositeAlphaFlagBitsKHR compositeAlphaFlags[4] = {
            VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR
    };
    for (uint32_t i = 0; i < ARRAY_SIZE(compositeAlphaFlags); i++) {
        if (surfCapabilities.supportedCompositeAlpha & compositeAlphaFlags[i]) {
            compositeAlpha = compositeAlphaFlags[i];
            break;
        }
    }

    // we do not want opaque
    if (surfCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) {
        compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    }

    VkSwapchainCreateInfoKHR swapchain_ci = {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext = NULL,
            .surface = vulkanDSL->surface,
            .minImageCount = desiredNumOfSwapchainImages,
            .imageFormat = vulkanDSL->format,
            .imageColorSpace = vulkanDSL->color_space,
            .imageExtent =
                    {
                            .width = swapchainExtent.width,
                            .height = swapchainExtent.height,
                    },
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .preTransform = preTransform,
            .compositeAlpha = compositeAlpha,
            .imageArrayLayers = 1,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = NULL,
            .presentMode = swapchainPresentMode,
            .oldSwapchain = oldSwapchain,
            .clipped = true,
    };
    uint32_t i;
    err = vulkanDSL->fpCreateSwapchainKHR(vulkanDSL->device, &swapchain_ci, NULL, &vulkanDSL->swapchain);
    assert(!err);

    // If we just re-created an existing swapchain, we should destroy the old
    // swapchain at this point.
    // Note: destroying the swapchain also cleans up all its associated
    // presentable images once the platform is done with them.
    if (oldSwapchain != VK_NULL_HANDLE) {
        vulkanDSL->fpDestroySwapchainKHR(vulkanDSL->device, oldSwapchain, NULL);
    }

    err = vulkanDSL->fpGetSwapchainImagesKHR(vulkanDSL->device, vulkanDSL->swapchain, &vulkanDSL->swapchainImageCount, NULL);
    assert(!err);

    VkImage *swapchainImages = (VkImage *)malloc(vulkanDSL->swapchainImageCount * sizeof(VkImage));
    assert(swapchainImages);
    err = vulkanDSL->fpGetSwapchainImagesKHR(vulkanDSL->device, vulkanDSL->swapchain, &vulkanDSL->swapchainImageCount, swapchainImages);
    assert(!err);

    vulkanDSL->swapchain_image_resources =
            (SwapchainImageResources *)malloc(sizeof(SwapchainImageResources) * vulkanDSL->swapchainImageCount);
    assert(vulkanDSL->swapchain_image_resources);
    vulkanDSL->vertex_buffer_resources =
            (VertexBufferResources *)malloc(sizeof(VertexBufferResources));
    assert(vulkanDSL->vertex_buffer_resources);

    for (i = 0; i < vulkanDSL->swapchainImageCount; i++) {
        VkImageViewCreateInfo color_image_view = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .pNext = NULL,
                .format = vulkanDSL->format,
                .components =
                        {
                                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                        },
                .subresourceRange =
                        {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1},
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .flags = 0,
        };

        vulkanDSL->swapchain_image_resources[i].image = swapchainImages[i];

        color_image_view.image = vulkanDSL->swapchain_image_resources[i].image;

        err = vkCreateImageView(vulkanDSL->device, &color_image_view, NULL, &vulkanDSL->swapchain_image_resources[i].view);
        assert(!err);
    }

    if (vulkanDSL->VK_GOOGLE_display_timing_enabled) {
        VkRefreshCycleDurationGOOGLE rc_dur;
        err = vulkanDSL->fpGetRefreshCycleDurationGOOGLE(vulkanDSL->device, vulkanDSL->swapchain, &rc_dur);
        assert(!err);
        vulkanDSL->refresh_duration = rc_dur.refreshDuration;

        vulkanDSL->syncd_with_actual_presents = false;
        // Initially target 1X the refresh duration:
        vulkanDSL->target_IPD = vulkanDSL->refresh_duration;
        vulkanDSL->refresh_duration_multiplier = 1;
        vulkanDSL->prev_desired_present_time = 0;
        vulkanDSL->next_present_id = 1;
    }

    if (NULL != swapchainImages) {
        free(swapchainImages);
    }

    if (NULL != presentModes) {
        free(presentModes);
    }
}

static void demo_prepare_multisample_buffer(struct VulkanDSL *vulkanDSL) {
    VkFormat colorFormat = vulkanDSL->format;

    createImage(vulkanDSL, vulkanDSL->width, vulkanDSL->height, 1, vulkanDSL->msaaSamples,
                colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &vulkanDSL->colorImageMultisample, &vulkanDSL->colorImageMemoryMultisample);
    vulkanDSL->colorImageViewMultisample = createImageView(vulkanDSL, vulkanDSL->colorImageMultisample, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

static void demo_prepare_depth(struct VulkanDSL *vulkanDSL) {
    const VkFormat depth_format = VK_FORMAT_D16_UNORM;
    const VkImageCreateInfo image = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = NULL,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = depth_format,
            .extent = {vulkanDSL->width, vulkanDSL->height, 1},
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = vulkanDSL->msaaSamples,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            .flags = 0,
    };

    VkImageViewCreateInfo view = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = NULL,
            .image = VK_NULL_HANDLE,
            .format = depth_format,
            .subresourceRange =
                    {.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1},
            .flags = 0,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
    };

    VkMemoryRequirements mem_reqs;
    VkResult U_ASSERT_ONLY err;
    bool U_ASSERT_ONLY pass;

    vulkanDSL->depth.format = depth_format;

    /* create image */
    err = vkCreateImage(vulkanDSL->device, &image, NULL, &vulkanDSL->depth.image);
    assert(!err);

    vkGetImageMemoryRequirements(vulkanDSL->device, vulkanDSL->depth.image, &mem_reqs);
    assert(!err);

    vulkanDSL->depth.mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vulkanDSL->depth.mem_alloc.pNext = NULL;
    vulkanDSL->depth.mem_alloc.allocationSize = mem_reqs.size;
    vulkanDSL->depth.mem_alloc.memoryTypeIndex = 0;

    pass = memory_type_from_properties(vulkanDSL, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                       &vulkanDSL->depth.mem_alloc.memoryTypeIndex);
    assert(pass);

    /* allocate memory */
    err = vkAllocateMemory(vulkanDSL->device, &vulkanDSL->depth.mem_alloc, NULL, &vulkanDSL->depth.mem);
    assert(!err);

    /* bind memory */
    err = vkBindImageMemory(vulkanDSL->device, vulkanDSL->depth.image, vulkanDSL->depth.mem, 0);
    assert(!err);

    /* create image view */
    view.image = vulkanDSL->depth.image;
    err = vkCreateImageView(vulkanDSL->device, &view, NULL, &vulkanDSL->depth.view);
    assert(!err);
}

bool loadTexture(struct VulkanDSL *vulkanDSL, const char *filename, uint8_t *rgba_data, VkSubresourceLayout *layout, int32_t *width, int32_t *height) {
    int texChannels;
#ifdef __ANDROID__
    AAsset* asset = AAssetManager_open(
      vulkanDSL->assetsFetcher.assetManager,
      filename,
      AASSET_MODE_BUFFER
    );
    off64_t length1 = AAsset_getLength(asset);
    void* rawData = (void*)AAsset_getBuffer(asset);
    FILE* file = fmemopen(rawData, length1, "rb");
    stbi_uc* pixels = stbi_load_from_file(file, width, height, &texChannels, STBI_rgb_alpha);
    fclose(file);
#else
    stbi_convert_iphone_png_to_rgb(1);
    stbi_uc* pixels = stbi_load(filename, width, height, &texChannels, STBI_rgb_alpha);
#endif
    if (rgba_data == NULL) {
        return true;
    }
    for (int y = 0; y < *height; y++) {
        uint8_t *rowPtr = rgba_data;
        for (int x = 0; x < *width; x++) {
            memcpy(rowPtr, pixels, 4);
            //rowPtr[3] = 255; // alhpa = 1
            rowPtr += 4;
            pixels += 4;
        }
        rgba_data += layout->rowPitch;
    }
#ifdef __ANDROID__
    AAsset_close(asset);
#endif
    return true;
}


static void demo_prepare_texture_buffer(struct VulkanDSL *vulkanDSL, const char *filename, struct texture_object *tex_obj) {
    int32_t tex_width;
    int32_t tex_height;
    VkResult U_ASSERT_ONLY err;
    bool U_ASSERT_ONLY pass;

    int texChannels;
    if (!stbi_load(filename, &tex_width, &tex_height, &texChannels, STBI_rgb_alpha)) {
        ERR_EXIT("Failed to load textures", "Load Texture Failure");
    }

    tex_obj->tex_width = tex_width;
    tex_obj->tex_height = tex_height;

    const VkBufferCreateInfo buffer_create_info = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .size = tex_width * tex_height * 4,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = NULL};

    err = vkCreateBuffer(vulkanDSL->device, &buffer_create_info, NULL, &tex_obj->buffer);
    assert(!err);

    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(vulkanDSL->device, tex_obj->buffer, &mem_reqs);

    tex_obj->mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    tex_obj->mem_alloc.pNext = NULL;
    tex_obj->mem_alloc.allocationSize = mem_reqs.size;
    tex_obj->mem_alloc.memoryTypeIndex = 0;

    VkFlags requirements = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    pass = memory_type_from_properties(vulkanDSL, mem_reqs.memoryTypeBits, requirements, &tex_obj->mem_alloc.memoryTypeIndex);
    assert(pass);

    err = vkAllocateMemory(vulkanDSL->device, &tex_obj->mem_alloc, NULL, &(tex_obj->mem));
    assert(!err);

    /* bind memory */
    err = vkBindBufferMemory(vulkanDSL->device, tex_obj->buffer, tex_obj->mem, 0);
    assert(!err);

    VkSubresourceLayout layout;
    memset(&layout, 0, sizeof(layout));
    layout.rowPitch = tex_width * 4;

    void *data;
    err = vkMapMemory(vulkanDSL->device, tex_obj->mem, 0, tex_obj->mem_alloc.allocationSize, 0, &data);
    assert(!err);

    if (!loadTexture(vulkanDSL, filename, data, &layout, &tex_width, &tex_height)) {
        fprintf(stderr, "Error loading texture: %s\n", filename);
    }

    vkUnmapMemory(vulkanDSL->device, tex_obj->mem);
}

static void demo_prepare_texture_image(struct VulkanDSL *vulkanDSL, const char *filename, struct texture_object *tex_obj,
                                       VkImageTiling tiling, VkImageUsageFlags usage, VkFlags required_props) {
    const VkFormat tex_format = VK_FORMAT_R8G8B8A8_UNORM;
    int32_t tex_width;
    int32_t tex_height;
    VkResult U_ASSERT_ONLY err;
    bool U_ASSERT_ONLY pass;
    int texChannels;
#ifdef __ANDROID__
    if (!loadTexture(vulkanDSL, filename, NULL, NULL, &tex_width, &tex_height)) {
        fprintf(stderr, "Error loading texture: %s\n", filename);
    }
#else
    if (!stbi_load(filename, &tex_width, &tex_height, &texChannels, STBI_rgb_alpha)) {
        ERR_EXIT("1 Failed to load textures", "Load Texture Failure");
    }
#endif
    tex_obj->tex_width = tex_width;
    tex_obj->tex_height = tex_height;

    const VkImageCreateInfo image_create_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = NULL,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = tex_format,
            .extent = {tex_width, tex_height, 1},
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT, // in tiling linear
            //.samples = vulkanDSL->msaaSamples,
            .tiling = tiling,
            .usage = usage,
            .flags = 0,
            .initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED,
    };

    VkMemoryRequirements mem_reqs;
    memset(&mem_reqs, 0, sizeof(VkMemoryRequirements));

    err = vkCreateImage(vulkanDSL->device, &image_create_info, NULL, &tex_obj->image);
    assert(!err);

    vkGetImageMemoryRequirements(vulkanDSL->device, tex_obj->image, &mem_reqs);

    tex_obj->mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    tex_obj->mem_alloc.pNext = NULL;
    tex_obj->mem_alloc.allocationSize = mem_reqs.size;
    tex_obj->mem_alloc.memoryTypeIndex = 0;

    pass = memory_type_from_properties(vulkanDSL, mem_reqs.memoryTypeBits, required_props, &tex_obj->mem_alloc.memoryTypeIndex);
    assert(pass);

    /* allocate memory */
    err = vkAllocateMemory(vulkanDSL->device, &tex_obj->mem_alloc, NULL, &(tex_obj->mem));
    assert(!err);

    /* bind memory */
    err = vkBindImageMemory(vulkanDSL->device, tex_obj->image, tex_obj->mem, 0);
    assert(!err);

    if (required_props & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        const VkImageSubresource subres = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .arrayLayer = 0,
        };
        VkSubresourceLayout layout;
        void *data;

        vkGetImageSubresourceLayout(vulkanDSL->device, tex_obj->image, &subres, &layout);

        err = vkMapMemory(vulkanDSL->device, tex_obj->mem, 0, tex_obj->mem_alloc.allocationSize, 0, &data);
        assert(!err);

        if (!loadTexture(vulkanDSL, filename, data, &layout, &tex_width, &tex_height)) {
            fprintf(stderr, "Error loading texture: %s\n", filename);
        }

        vkUnmapMemory(vulkanDSL->device, tex_obj->mem);
    }

    tex_obj->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

static void demo_destroy_texture(struct VulkanDSL *vulkanDSL, struct texture_object *tex_objs) {
    /* clean up staging resources */
    vkFreeMemory(vulkanDSL->device, tex_objs->mem, NULL);
    if (tex_objs->image) vkDestroyImage(vulkanDSL->device, tex_objs->image, NULL);
    if (tex_objs->buffer) vkDestroyBuffer(vulkanDSL->device, tex_objs->buffer, NULL);
}

static void demo_prepare_textures(struct VulkanDSL *vulkanDSL) {
    const VkFormat tex_format = VK_FORMAT_R8G8B8A8_UNORM;
    VkFormatProperties props;
    uint32_t i;

    vkGetPhysicalDeviceFormatProperties(vulkanDSL->gpu, tex_format, &props);
    for (i = 0; i < DEMO_TEXTURE_COUNT; i++) {
        VkResult U_ASSERT_ONLY err;

        if (!vulkanDSL->iosSim && (props.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) && !vulkanDSL->use_staging_buffer) {
            //NSLog(@"--> Linear tiling is supported");
            /* Device can texture using linear textures */
            demo_prepare_texture_image(vulkanDSL, tex_files[i], &vulkanDSL->textures[i], VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_SAMPLED_BIT,
                                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            // Nothing in the pipeline needs to be complete to start, and don't allow fragment
            // shader to run until layout transition completes
            demo_set_image_layout(vulkanDSL, vulkanDSL->textures[i].image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PREINITIALIZED,
                                  vulkanDSL->textures[i].imageLayout, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
            vulkanDSL->staging_texture.image = 0;
        } else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) {
            /* Must use staging buffer to copy linear texture to optimize */

            memset(&vulkanDSL->staging_texture, 0, sizeof(vulkanDSL->staging_texture));
            demo_prepare_texture_buffer(vulkanDSL, tex_files[i], &vulkanDSL->staging_texture);

            demo_prepare_texture_image(vulkanDSL, tex_files[i], &vulkanDSL->textures[i], VK_IMAGE_TILING_OPTIMAL,
                                       (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            demo_set_image_layout(vulkanDSL, vulkanDSL->textures[i].image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PREINITIALIZED,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                  VK_PIPELINE_STAGE_TRANSFER_BIT);

            VkBufferImageCopy copy_region = {
                    .bufferOffset = 0,
                    .bufferRowLength = vulkanDSL->staging_texture.tex_width,
                    .bufferImageHeight = vulkanDSL->staging_texture.tex_height,
                    .imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
                    .imageOffset = {0, 0, 0},
                    .imageExtent = {vulkanDSL->staging_texture.tex_width, vulkanDSL->staging_texture.tex_height, 1},
            };

            vkCmdCopyBufferToImage(vulkanDSL->cmd, vulkanDSL->staging_texture.buffer, vulkanDSL->textures[i].image,
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

            demo_set_image_layout(vulkanDSL, vulkanDSL->textures[i].image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                  vulkanDSL->textures[i].imageLayout, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

        } else {
            /* Can't support VK_FORMAT_R8G8B8A8_UNORM !? */
            assert(!"No support for R8G8B8A8_UNORM as texture image format");
        }

        // texture filtering
        // https://vulkan-tutorial.com/Texture_mapping/Image_view_and_sampler#page_Anisotropy-device-feature
        const VkSamplerCreateInfo sampler = {
                .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                .pNext = NULL,
                .magFilter = VK_FILTER_LINEAR, //VK_FILTER_LINEAR or VK_FILTER_NEAREST
                /*
GL_NEAREST
Returns the value of the texture element that is nearest (in Manhattan distance) to the center of the pixel being textured.

GL_LINEAR
Returns the weighted average of the four texture elements that are closest to the center of the pixel being textured.
                 */
                .minFilter = VK_FILTER_LINEAR,
                .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
                .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                //.anisotropyEnable = VK_FALSE,
                .anisotropyEnable = VK_TRUE,
                //.maxAnisotropy = 1,
                .maxAnisotropy = vulkanDSL->maxSamplerAnisotropy,
                .compareOp = VK_COMPARE_OP_NEVER,
                .compareEnable = VK_FALSE,
                .mipLodBias = 0.0f,
                .minLod = 0.0f,
                .maxLod = 0.0f,
                .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
                .unnormalizedCoordinates = VK_FALSE,
        };

        VkImageViewCreateInfo view = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .pNext = NULL,
                .image = VK_NULL_HANDLE,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = tex_format,
                .components =
                        {
                                VK_COMPONENT_SWIZZLE_IDENTITY,
                                VK_COMPONENT_SWIZZLE_IDENTITY,
                                VK_COMPONENT_SWIZZLE_IDENTITY,
                                VK_COMPONENT_SWIZZLE_IDENTITY,
                        },
                .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
                .flags = 0,
        };

        /* create sampler */
        err = vkCreateSampler(vulkanDSL->device, &sampler, NULL, &vulkanDSL->textures[i].sampler);
        assert(!err);

        /* create image view */
        view.image = vulkanDSL->textures[i].image;
        err = vkCreateImageView(vulkanDSL->device, &view, NULL, &vulkanDSL->textures[i].view);
        assert(!err);
    }
}

void demo_prepare_cube_data_buffers(struct VulkanDSL *vulkanDSL) {
    VkBufferCreateInfo buf_info;
    VkMemoryRequirements mem_reqs;
    VkMemoryAllocateInfo mem_alloc;
    mat4x4 MVP, VP;
    VkResult U_ASSERT_ONLY err;
    bool U_ASSERT_ONLY pass;
    struct vktexcube_vs_uniform data;

    mat4x4_mul(VP, vulkanDSL->projection_matrix, vulkanDSL->view_matrix);
    mat4x4_mul(MVP, VP, vulkanDSL->model_matrix);
    memcpy(data.mvp, MVP, sizeof(MVP));
    //    dumpMatrix("MVP", MVP);

    for (unsigned int i = 0; i < 12 * 3; i++) {
        data.position[i][0] = g_vertex_buffer_data[i * 3];
        data.position[i][1] = g_vertex_buffer_data[i * 3 + 1];
        data.position[i][2] = g_vertex_buffer_data[i * 3 + 2];
        data.position[i][3] = 1.0f;
        data.attr[i][0] = g_uv_buffer_data[2 * i];
        data.attr[i][1] = g_uv_buffer_data[2 * i + 1];
        data.attr[i][2] = 0;
        data.attr[i][3] = 0;
    }

    memset(&buf_info, 0, sizeof(buf_info));
    buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buf_info.size = sizeof(data);

    for (unsigned int i = 0; i < vulkanDSL->swapchainImageCount; i++) {
        err = vkCreateBuffer(vulkanDSL->device, &buf_info, NULL, &vulkanDSL->swapchain_image_resources[i].uniform_buffer);
        assert(!err);

        vkGetBufferMemoryRequirements(vulkanDSL->device, vulkanDSL->swapchain_image_resources[i].uniform_buffer, &mem_reqs);

        mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        mem_alloc.pNext = NULL;
        mem_alloc.allocationSize = mem_reqs.size;
        mem_alloc.memoryTypeIndex = 0;

        pass = memory_type_from_properties(vulkanDSL, mem_reqs.memoryTypeBits,
                                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                           &mem_alloc.memoryTypeIndex);
        assert(pass);

        err = vkAllocateMemory(vulkanDSL->device, &mem_alloc, NULL, &vulkanDSL->swapchain_image_resources[i].uniform_memory);
        assert(!err);

        // map to the application address space
        err = vkMapMemory(vulkanDSL->device, vulkanDSL->swapchain_image_resources[i].uniform_memory, 0, VK_WHOLE_SIZE, 0,
                          &vulkanDSL->swapchain_image_resources[i].uniform_memory_ptr);
        assert(!err);

        memcpy(vulkanDSL->swapchain_image_resources[i].uniform_memory_ptr, &data, sizeof data);

        err = vkBindBufferMemory(vulkanDSL->device, vulkanDSL->swapchain_image_resources[i].uniform_buffer,
                                 vulkanDSL->swapchain_image_resources[i].uniform_memory, 0);
        assert(!err);
    }
}

// https://vulkan-tutorial.com/Vertex_buffers/Vertex_buffer_creation
void VulkanDSL__allocate_vulkan_buffer(
        struct VulkanDSL *vulkanDSL, VkBufferCreateInfo *buf_info, VkBuffer *buffer,
        VkFlags memory_properties, VkDeviceMemory *buffer_memory,
        bool *coherentMemory) {
    VkResult U_ASSERT_ONLY err;
    bool U_ASSERT_ONLY pass;

    err = vkCreateBuffer(vulkanDSL->device, buf_info, NULL, buffer);
    assert(!err);

    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(vulkanDSL->device, *buffer, &mem_reqs);

    VkMemoryAllocateInfo mem_alloc;
    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.pNext = NULL;
    mem_alloc.allocationSize = mem_reqs.size;
    mem_alloc.memoryTypeIndex = 0;

    // mem_reqs.memoryTypeBits contains the compatible memories
    // then mem_alloc.memoryTypeIndex contains the chosen memory index
    pass = memory_type_from_properties(vulkanDSL, mem_reqs.memoryTypeBits,
                                       memory_properties, // no coherent is required
                                       &mem_alloc.memoryTypeIndex);
    if (!pass) {
        // we produce a coherent one if we could not get a non-coherent one
        // a boolean will tell the application that flush is needed if we produce a non-coherent
        if ((memory_properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == memory_properties) {
            memory_properties &= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            pass = memory_type_from_properties(vulkanDSL, mem_reqs.memoryTypeBits,
                                               memory_properties,
                                               &mem_alloc.memoryTypeIndex);
        }
    }
    assert(pass);

    *coherentMemory = (memory_properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    err = vkAllocateMemory(vulkanDSL->device, &mem_alloc, NULL, buffer_memory);
    assert(!err);

    err = vkBindBufferMemory(vulkanDSL->device, *buffer,
                             *buffer_memory, 0);
    assert(!err);
}

void VulkanDSL__fill_vulkan_buffer(
        struct VulkanDSL *vulkanDSL, VkBufferCreateInfo *buf_info,
        VkDeviceMemory *buffer_memory,
        bool coherentMemory,
        void **buffer_memory_ptr_ptr,
        void *data) {
    VkResult U_ASSERT_ONLY err;

    // map to the application address space
    err = vkMapMemory(vulkanDSL->device, *buffer_memory, 0, buf_info->size, 0,
                      buffer_memory_ptr_ptr);
    assert(!err);

    memcpy(*buffer_memory_ptr_ptr, data, buf_info->size);

    if (!coherentMemory) {
        // this is because we use non coherent memory
        VkMappedMemoryRange range;
        range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        range.pNext = 0;
        range.memory = *buffer_memory;
        range.offset = 0;
        range.size = buf_info->size;

        err = vkFlushMappedMemoryRanges(vulkanDSL->device, 1, &range);
        assert(!err);

        err = vkInvalidateMappedMemoryRanges(vulkanDSL->device, 1, &range);
        assert(!err);
    }
}

void VulkanDSL__prepare_vertex_buffer_classic(struct VulkanDSL *vulkanDSL, tinyobj_attrib_t *attrib) {
    VkResult U_ASSERT_ONLY err;

    VkBufferCreateInfo buf_info;
    memset(&buf_info, 0, sizeof(buf_info));
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    int sizeVertices = vulkanDSL->assetsFetcher.arraySize * sizeof(vulkanDSL->assetsFetcher.triangles[0]);
    buf_info.size = sizeVertices;

    vulkanDSL->vi_binding.binding = 0;
    vulkanDSL->vi_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    // 5 for 3 coordinates + 2 uv texture coordinates
    vulkanDSL->vi_binding.stride = 5 * sizeof(float);

    // this is for vertices in variables
    // a color format is used for vec3, see this link here
    // https://vulkan-tutorial.com/Vertex_buffers/Vertex_input_description#page_Attribute-descriptions
    // float: VK_FORMAT_R32_SFLOAT
    // vec2: VK_FORMAT_R32G32_SFLOAT
    // vec3: VK_FORMAT_R32G32B32_SFLOAT
    // vec4: VK_FORMAT_R32G32B32A32_SFLOAT
    vulkanDSL->vi_attribs[0].binding = 0;
    vulkanDSL->vi_attribs[0].location = 0;
    vulkanDSL->vi_attribs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vulkanDSL->vi_attribs[0].offset = 0;
    vulkanDSL->vi_attribs[1].binding = 0;
    vulkanDSL->vi_attribs[1].location = 1;
    vulkanDSL->vi_attribs[1].format = VK_FORMAT_R32G32_SFLOAT;
    // offset represents the shift compared to the start of the vertex struct
    // so it is the size of the attrib 0 for the coordinates
    vulkanDSL->vi_attribs[1].offset = 3 * sizeof(float);

    bool coherentMemory;
    VulkanDSL__allocate_vulkan_buffer(
            vulkanDSL, &buf_info, &vulkanDSL->vertex_buffer_resources->vertex_buffer,
           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &vulkanDSL->vertex_buffer_resources->vertex_memory,
            &coherentMemory);
    vulkanDSL->vertex_buffer_resources->vertex_buffer_allocated = true;
    VulkanDSL__fill_vulkan_buffer(
            vulkanDSL, &buf_info,
            &vulkanDSL->vertex_buffer_resources->vertex_memory,
            coherentMemory,
            &vulkanDSL->vertex_buffer_resources->vertex_memory_ptr,
            vulkanDSL->assetsFetcher.triangles);
    vulkanDSL->vertex_buffer_resources->vertex_memory_mapped = true;
}

// some tutorials about using a staging buffer to send vertices (and use GPU local memory)
// best full intel tutorial:
// https://www.intel.com/content/www/us/en/developer/articles/training/api-without-secrets-introduction-to-vulkan-part-5.html
// see the code at "if (isUnifiedGraphicsAndTransferQueue)"
// https://github.com/cforfang/Vulkan-Tools/wiki/Synchronization-Examples
// https://cpp-rendering.io/barriers-vulkan-not-difficult/
// https://github.com/GameTechDev/IntroductionToVulkan/blob/master/Project/Tutorials/05/Tutorial05.cpp
// See in particular the example 19 here:
// https://github.com/Overv/VulkanTutorial.git
void copyBuffer(struct VulkanDSL *vulkanDSL, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkResult U_ASSERT_ONLY err;
    const VkCommandBufferAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = NULL,
            .commandPool = vulkanDSL->cmd_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
    };

    VkCommandBuffer commandBuffer;
    err = vkAllocateCommandBuffers(vulkanDSL->device, &allocInfo, &commandBuffer);
    assert(!err);

    VkCommandBufferBeginInfo beginInfo;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pNext = 0;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion;
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    VkBufferMemoryBarrier buffer_memory_barrier = {
            VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,          // VkStructureType                        sType;
            NULL,                                             // const void                            *pNext
            VK_ACCESS_MEMORY_WRITE_BIT,                       // VkAccessFlags                          srcAccessMask
            VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,              // VkAccessFlags                          dstAccessMask
            VK_QUEUE_FAMILY_IGNORED,                          // uint32_t                               srcQueueFamilyIndex
            VK_QUEUE_FAMILY_IGNORED,                          // uint32_t                               dstQueueFamilyIndex
            dstBuffer,// VkBuffer                               buffer
            0,                                                // VkDeviceSize                           offset
            VK_WHOLE_SIZE                                     // VkDeviceSize                           size
    };
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, NULL, 1, &buffer_memory_barrier, 0, NULL );

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = NULL,
            .waitSemaphoreCount = 0,
            .pWaitSemaphores = NULL,
            .pWaitDstStageMask = NULL,
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffer,
            .signalSemaphoreCount = 0,
            .pSignalSemaphores = NULL
    };

    VkFence fence;
    VkFenceCreateInfo fence_ci = {.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .pNext = NULL, .flags = 0};
    err = vkCreateFence(vulkanDSL->device, &fence_ci, NULL, &fence);
    assert(!err);
    vkQueueSubmit(vulkanDSL->graphics_queue, 1, &submitInfo, fence);
    err = vkWaitForFences(vulkanDSL->device, 1, &fence, VK_TRUE, UINT64_MAX);
    assert(!err);
    vkQueueWaitIdle(vulkanDSL->graphics_queue);
    vkDestroyFence(vulkanDSL->device, fence, NULL);
    vkFreeCommandBuffers(vulkanDSL->device, vulkanDSL->cmd_pool, 1, &commandBuffer);
}

// https://vkguide.dev/docs/chapter-5/memory_transfers/
// https://raw.githubusercontent.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/master/include/vk_mem_alloc.h
// see also the flag USE_STAGING here:
// https://github.com/SaschaWillems/Vulkan/blob/master/examples/triangle/triangle.cpp
void VulkanDSL__prepare_vertex_buffer_gpu_only(struct VulkanDSL *vulkanDSL, tinyobj_attrib_t *attrib) {
    VkResult U_ASSERT_ONLY err;
    bool U_ASSERT_ONLY pass;

    VkBufferCreateInfo buf_info;
    VkMemoryRequirements mem_reqs;
    VkMemoryAllocateInfo mem_alloc;

    memset(&buf_info, 0, sizeof(buf_info));
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    int sizeVertices = vulkanDSL->assetsFetcher.arraySize * sizeof(vulkanDSL->assetsFetcher.triangles[0]);
    buf_info.size = sizeVertices;

    vulkanDSL->vi_binding.binding = 0;
    vulkanDSL->vi_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    // 5 for 3 coordinates + 2 uv texture coordinates
    vulkanDSL->vi_binding.stride = 5 * sizeof(float);

    // this is for vertices in variables
    // a color format is used for vec3, see this link here
    // https://vulkan-tutorial.com/Vertex_buffers/Vertex_input_description#page_Attribute-descriptions
    // float: VK_FORMAT_R32_SFLOAT
    // vec2: VK_FORMAT_R32G32_SFLOAT
    // vec3: VK_FORMAT_R32G32B32_SFLOAT
    // vec4: VK_FORMAT_R32G32B32A32_SFLOAT
    vulkanDSL->vi_attribs[0].binding = 0;
    vulkanDSL->vi_attribs[0].location = 0;
    //vulkanDSL->vi_attribs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vulkanDSL->vi_attribs[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    vulkanDSL->vi_attribs[0].offset = 0;
    vulkanDSL->vi_attribs[1].binding = 0;
    vulkanDSL->vi_attribs[1].location = 1;
    vulkanDSL->vi_attribs[1].format = VK_FORMAT_R32G32_SFLOAT;
    // offset represents the shift compared to the start of the vertex struct
    // so it is the size of the attrib 0 for the coordinates
    vulkanDSL->vi_attribs[1].offset = 3 * sizeof(float);


    bool coherentMemory;
    VulkanDSL__allocate_vulkan_buffer(
            vulkanDSL, &buf_info, &vulkanDSL->vertex_buffer_resources->vertex_buffer,
            //VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &vulkanDSL->vertex_buffer_resources->vertex_memory,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &vulkanDSL->vertex_buffer_resources->vertex_memory,
            &coherentMemory);
    vulkanDSL->vertex_buffer_resources->vertex_buffer_allocated = true;
    VulkanDSL__fill_vulkan_buffer(
            vulkanDSL, &buf_info,
            &vulkanDSL->vertex_buffer_resources->vertex_memory,
            coherentMemory,
            &vulkanDSL->vertex_buffer_resources->vertex_memory_ptr,
            vulkanDSL->assetsFetcher.triangles);
    vulkanDSL->vertex_buffer_resources->vertex_memory_mapped = true;

    // a GPU only buffer used as vertex buffer and as a transfer destination
    buf_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VulkanDSL__allocate_vulkan_buffer(
            vulkanDSL, &buf_info, &vulkanDSL->vertex_buffer_resources->vertex_buffer_gpu,

            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            //VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &vulkanDSL->vertex_buffer_resources->vertex_memory_gpu,
            &coherentMemory);
    vulkanDSL->vertex_buffer_resources->vertex_buffer_gpu_allocated = true;
    copyBuffer(vulkanDSL, vulkanDSL->vertex_buffer_resources->vertex_buffer, vulkanDSL->vertex_buffer_resources->vertex_buffer_gpu, sizeVertices);
}

// here we define the in variables in the shader that have the tag "binding"
// layout (binding = 1)
static void demo_prepare_descriptor_layout(struct VulkanDSL *vulkanDSL) {
    const VkDescriptorSetLayoutBinding layout_bindings[2] = {
            [0] =
                    {
                            .binding = 0,
                            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                            .descriptorCount = 1,
                            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                            .pImmutableSamplers = NULL,
                    },
            [1] =
                    {
                            .binding = 1,
                            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                            .descriptorCount = DEMO_TEXTURE_COUNT,
                            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                            .pImmutableSamplers = NULL,
                    },
    };
    const VkDescriptorSetLayoutCreateInfo descriptor_layout = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = NULL,
            .bindingCount = 2,
            .pBindings = layout_bindings,
    };
    VkResult U_ASSERT_ONLY err;

    err = vkCreateDescriptorSetLayout(vulkanDSL->device, &descriptor_layout, NULL, &vulkanDSL->desc_layout);
    assert(!err);

    const VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = NULL,
            .setLayoutCount = 1,
            .pSetLayouts = &vulkanDSL->desc_layout,
    };

    err = vkCreatePipelineLayout(vulkanDSL->device, &pPipelineLayoutCreateInfo, NULL, &vulkanDSL->pipeline_layout);
    assert(!err);
}

static void demo_prepare_render_pass(struct VulkanDSL *vulkanDSL) {
    // The initial layout for the color and depth attachments will be LAYOUT_UNDEFINED
    // because at the start of the renderpass, we don't care about their contents.
    // At the start of the subpass, the color attachment's layout will be transitioned
    // to LAYOUT_COLOR_ATTACHMENT_OPTIMAL and the depth stencil attachment's layout
    // will be transitioned to LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL.  At the end of
    // the renderpass, the color attachment's layout will be transitioned to
    // LAYOUT_PRESENT_SRC_KHR to be ready to present.  This is all done as part of
    // the renderpass, no barriers are necessary.
    const VkAttachmentDescription attachments[3] = {
            [0] =
                    {
                            .format = vulkanDSL->format,
                            .flags = 0,
                            .samples = vulkanDSL->msaaSamples,
                            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                    },
            [1] =
                    {
                            .format = vulkanDSL->depth.format,
                            .flags = 0,
                            .samples = vulkanDSL->msaaSamples,
                            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                    },
            [2] =
                    {
                            .format = vulkanDSL->format,
                            .flags = 0,
                            // https://vulkan-tutorial.com/Multisampling
                            .samples = VK_SAMPLE_COUNT_1_BIT, // the final image is not sampled
                            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                    },
    };
    const VkAttachmentReference color_reference = {
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };
    const VkAttachmentReference depth_reference = {
            .attachment = 1,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };
    VkAttachmentReference colorAttachmentResolveRef;
    colorAttachmentResolveRef.attachment = 2;
    colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    const VkSubpassDescription subpass = {
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .flags = 0,
            .inputAttachmentCount = 0,
            .pInputAttachments = NULL,
            .colorAttachmentCount = 1,
            .pColorAttachments = &color_reference,
            .pResolveAttachments = &colorAttachmentResolveRef,
            .pDepthStencilAttachment = &depth_reference,
            .preserveAttachmentCount = 0,
            .pPreserveAttachments = NULL,
    };

    VkSubpassDependency attachmentDependencies[2] = {
            [0] =
                    {
                            // Depth buffer is shared between swapchain images
                            .srcSubpass = VK_SUBPASS_EXTERNAL,
                            .dstSubpass = 0,
                            .srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                            .dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                            .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                            .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                            .dependencyFlags = 0,
                    },
            [1] =
                    {
                            // Image Layout Transition
                            .srcSubpass = VK_SUBPASS_EXTERNAL,
                            .dstSubpass = 0,
                            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                            .srcAccessMask = 0,
                            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
                            .dependencyFlags = 0,
                    },
    };

    const VkRenderPassCreateInfo rp_info = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .attachmentCount = 3,
            //.attachmentCount = 1,
            .pAttachments = attachments,
            .subpassCount = 1,
            .pSubpasses = &subpass,
            .dependencyCount = 2,
            //.dependencyCount = 1,
            //.pDependencies = &attachmentDependencies[1],
            .pDependencies = attachmentDependencies,
    };
    VkResult U_ASSERT_ONLY err;

    err = vkCreateRenderPass(vulkanDSL->device, &rp_info, NULL, &vulkanDSL->render_pass);
    assert(!err);
}

static VkShaderModule demo_prepare_shader_module(struct VulkanDSL *vulkanDSL, const uint32_t *code, size_t size) {
    VkShaderModule module;
    VkShaderModuleCreateInfo moduleCreateInfo;
    VkResult U_ASSERT_ONLY err;

    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.pNext = NULL;
    moduleCreateInfo.flags = 0;
    moduleCreateInfo.codeSize = size;
    moduleCreateInfo.pCode = code;

    err = vkCreateShaderModule(vulkanDSL->device, &moduleCreateInfo, NULL, &module);
    assert(!err);

    return module;
}

void VulkanDSL__read_shader(struct VulkanDSL *vulkanDSL, const char* filename, uint32_t* vs_code, size_t *length1) {
#ifdef __ANDROID__
    AAsset* asset = AAssetManager_open(
            vulkanDSL->assetsFetcher.assetManager,
            filename,
            AASSET_MODE_BUFFER
    );
    *length1 = AAsset_getLength(asset);
    if (vs_code == NULL) {
        return;
    }
    void* rawData = (void*)AAsset_getBuffer(asset);
    FILE* file = fmemopen(rawData, length1, "rb");
    fread(vs_code, 1, *length1, file);
    fclose(file);
#else
    char *filename2 = NULL;
    for (int i = 0; i < strlen(filename); i++) {
      if (filename[i] == '/') {
        filename2 = (char*)&filename[i+1];
        break;
      }
    }
    char filePath[strlen(vulkanDSL->assetsFetcher.assetsPath) + strlen(filename2) + 5];
    sprintf(filePath, "%s/%s", vulkanDSL->assetsFetcher.assetsPath, filename2);
    FILE* ptr = fopen(filePath, "rb");
    int chunk = 100000;
    void* buffer = (void*) vs_code;
  void* buffer_start = buffer;
    if (vs_code == NULL) {
        buffer = (void*) malloc(sizeof(char)* chunk);
    }
    int length = 0;
    size_t added = 0;
    do {
        added = fread(buffer, 1, chunk, ptr);
        length += added;
        buffer += chunk;
    }
    while (added == chunk);
    *length1 = length;
    fclose(ptr);
    if (vs_code == NULL) {
        free(buffer_start);
    }
#endif
}

static void demo_prepare_vs(struct VulkanDSL *vulkanDSL) {
    size_t length1;
    const char* filename = "shaders/textPanel.vert.spv";
    VulkanDSL__read_shader(vulkanDSL, filename, NULL, &length1);
    uint32_t vs_code[length1];
    VulkanDSL__read_shader(vulkanDSL, filename, vs_code, &length1);
    vulkanDSL->vert_shader_module = demo_prepare_shader_module(vulkanDSL, vs_code, length1);
}

static void demo_prepare_fs(struct VulkanDSL *vulkanDSL) {
    size_t length1;
    const char* filename = "shaders/textPanel.frag.spv";
    //const char* filename = "shaders/cube.frag.spv";
    VulkanDSL__read_shader(vulkanDSL, filename, NULL, &length1);
    uint32_t vs_code[length1];
    VulkanDSL__read_shader(vulkanDSL, filename, vs_code, &length1);
    vulkanDSL->frag_shader_module = demo_prepare_shader_module(vulkanDSL, vs_code, length1);
}

VkSampleCountFlagBits getMaxUsableSampleCount(struct VulkanDSL *vulkanDSL) {
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(vulkanDSL->gpu, &physicalDeviceProperties);

    // to improve the performance we take the MIN()
    //vulkanDSL->maxSamplerAnisotropy = physicalDeviceProperties.limits.maxSamplerAnisotropy;
    vulkanDSL->maxSamplerAnisotropy = MIN(4, physicalDeviceProperties.limits.maxSamplerAnisotropy);
    VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
    if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
    if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
    if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
    if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
    if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

    return VK_SAMPLE_COUNT_1_BIT;
}

void createImage(struct VulkanDSL* vulkanDSL, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples,
                 VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                 VkMemoryPropertyFlags properties, VkImage *image, VkDeviceMemory *imageMemory) {
    VkImageCreateInfo imageInfo;
    VkResult U_ASSERT_ONLY err;
    memset(&imageInfo, 0, sizeof(VkImageCreateInfo));
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = numSamples;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    err = vkCreateImage(vulkanDSL->device, &imageInfo, NULL, image);
    assert(!err);

    VkMemoryRequirements memRequirements;
    memset(&memRequirements, 0, sizeof(VkMemoryRequirements));
    vkGetImageMemoryRequirements(vulkanDSL->device, *image, &memRequirements);

    VkMemoryAllocateInfo allocInfo;
    memset(&allocInfo, 0, sizeof(VkMemoryAllocateInfo));
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;

    bool U_ASSERT_ONLY pass = memory_type_from_properties(vulkanDSL, memRequirements.memoryTypeBits,
                                       properties,
                                       &allocInfo.memoryTypeIndex);
    assert(pass);

    err = vkAllocateMemory(vulkanDSL->device, &allocInfo, NULL, imageMemory);
    assert(!err);

    vkBindImageMemory(vulkanDSL->device, *image, *imageMemory, 0);
}

VkImageView createImageView(struct VulkanDSL *vulkanDSL, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) {
    VkImageViewCreateInfo viewInfo;
    memset(&viewInfo, 0, sizeof(VkImageViewCreateInfo));
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    assert(!vkCreateImageView(vulkanDSL->device, &viewInfo, NULL, &imageView));

    return imageView;
}

static void demo_prepare_pipeline(struct VulkanDSL *vulkanDSL) {
#define NUM_DYNAMIC_STATES 2 /*Viewport + Scissor*/

    VkGraphicsPipelineCreateInfo pipeline;
    VkPipelineCacheCreateInfo pipelineCache;
    VkPipelineVertexInputStateCreateInfo vi;
    VkPipelineInputAssemblyStateCreateInfo ia;
    VkPipelineRasterizationStateCreateInfo rs;
    VkPipelineColorBlendStateCreateInfo cb;
    VkPipelineDepthStencilStateCreateInfo ds;
    VkPipelineViewportStateCreateInfo vp;
    VkPipelineMultisampleStateCreateInfo ms;
    VkDynamicState dynamicStateEnables[NUM_DYNAMIC_STATES];
    VkPipelineDynamicStateCreateInfo dynamicState;
    VkResult U_ASSERT_ONLY err;

    memset(dynamicStateEnables, 0, sizeof dynamicStateEnables);
    memset(&dynamicState, 0, sizeof dynamicState);
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pDynamicStates = dynamicStateEnables;

    memset(&pipeline, 0, sizeof(pipeline));
    pipeline.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline.layout = vulkanDSL->pipeline_layout;

    memset(&vi, 0, sizeof(vi));
    vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    bool include_vi = true; // use vertex as input
    if (include_vi) {
        vi.pNext = NULL;
        vi.flags = 0;
        vi.vertexBindingDescriptionCount = 1;
        vi.pVertexBindingDescriptions = &vulkanDSL->vi_binding;
        vi.vertexAttributeDescriptionCount = 2;
        vi.pVertexAttributeDescriptions = vulkanDSL->vi_attribs;
    }

    memset(&ia, 0, sizeof(ia));
    ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    memset(&rs, 0, sizeof(rs));
    rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs.polygonMode = VK_POLYGON_MODE_FILL;
    rs.cullMode = VK_CULL_MODE_BACK_BIT;
    rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rs.depthClampEnable = VK_FALSE;
    rs.rasterizerDiscardEnable = VK_FALSE;
    rs.depthBiasEnable = VK_FALSE;
    rs.lineWidth = 1.0f;

    memset(&cb, 0, sizeof(cb));
    cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    VkPipelineColorBlendAttachmentState att_state[1];
    memset(att_state, 0, sizeof(att_state));
    att_state[0].colorWriteMask = 0xf;
    att_state[0].blendEnable = VK_FALSE;
    cb.attachmentCount = 1;
    cb.pAttachments = att_state;

    memset(&vp, 0, sizeof(vp));
    vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vp.viewportCount = 1;
    dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
    vp.scissorCount = 1;
    dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;

    memset(&ds, 0, sizeof(ds));
    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds.depthTestEnable = VK_TRUE;
    ds.depthWriteEnable = VK_TRUE;
    ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    ds.depthBoundsTestEnable = VK_FALSE;
    ds.back.failOp = VK_STENCIL_OP_KEEP;
    ds.back.passOp = VK_STENCIL_OP_KEEP;
    ds.back.compareOp = VK_COMPARE_OP_ALWAYS;
    ds.stencilTestEnable = VK_FALSE;
    ds.front = ds.back;

    memset(&ms, 0, sizeof(ms));
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.pSampleMask = NULL;
    //ms.sampleShadingEnable = VK_FALSE;
    ms.rasterizationSamples = vulkanDSL->msaaSamples;

    // https://vulkan-tutorial.com/Multisampling
    // https://www.khronos.org/registry/vulkan/specs/1.2/html/chap25.html#primsrast-sampleshading
    //ms.sampleShadingEnable = VK_FALSE; // NOT SURE
    ms.sampleShadingEnable = VK_TRUE; // enable sample shading in the pipeline
    ms.minSampleShading = .2f; // min fraction for sample shading; closer to one

    demo_prepare_vs(vulkanDSL);
    demo_prepare_fs(vulkanDSL);

    // Two stages: vs and fs
    VkPipelineShaderStageCreateInfo shaderStages[2];
    memset(&shaderStages, 0, 2 * sizeof(VkPipelineShaderStageCreateInfo));

    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = vulkanDSL->vert_shader_module;
    shaderStages[0].pName = "main";

    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = vulkanDSL->frag_shader_module;
    shaderStages[1].pName = "main";

    memset(&pipelineCache, 0, sizeof(pipelineCache));
    pipelineCache.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

    err = vkCreatePipelineCache(vulkanDSL->device, &pipelineCache, NULL, &vulkanDSL->pipelineCache);
    assert(!err);

    pipeline.pVertexInputState = &vi;
    pipeline.pInputAssemblyState = &ia;
    pipeline.pRasterizationState = &rs;
    pipeline.pColorBlendState = &cb;
    pipeline.pMultisampleState = &ms;
    pipeline.pViewportState = &vp;
    pipeline.pDepthStencilState = &ds;
    pipeline.stageCount = ARRAY_SIZE(shaderStages);
    pipeline.pStages = shaderStages;
    pipeline.renderPass = vulkanDSL->render_pass;
    pipeline.pDynamicState = &dynamicState;

    pipeline.renderPass = vulkanDSL->render_pass;

    err = vkCreateGraphicsPipelines(vulkanDSL->device, vulkanDSL->pipelineCache, 1, &pipeline, NULL, &vulkanDSL->pipeline);
    //err = vkCreateGraphicsPipelines(vulkanDSL->device, VK_NULL_HANDLE, 1, &pipeline, NULL, &vulkanDSL->pipeline);
    assert(!err);

    vkDestroyShaderModule(vulkanDSL->device, vulkanDSL->frag_shader_module, NULL);
    vkDestroyShaderModule(vulkanDSL->device, vulkanDSL->vert_shader_module, NULL);
}

static void demo_prepare_descriptor_pool(struct VulkanDSL *vulkanDSL) {
    const VkDescriptorPoolSize type_counts[2] = {
            [0] =
                    {
                            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                            .descriptorCount = vulkanDSL->swapchainImageCount,
                    },
            [1] =
                    {
                            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                            .descriptorCount = vulkanDSL->swapchainImageCount * DEMO_TEXTURE_COUNT,
                    },
    };
    const VkDescriptorPoolCreateInfo descriptor_pool = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = NULL,
            .maxSets = vulkanDSL->swapchainImageCount,
            .poolSizeCount = 2,
            .pPoolSizes = type_counts,
    };
    VkResult U_ASSERT_ONLY err;

    err = vkCreateDescriptorPool(vulkanDSL->device, &descriptor_pool, NULL, &vulkanDSL->desc_pool);
    assert(!err);
}

static void demo_prepare_descriptor_set(struct VulkanDSL *vulkanDSL) {
    VkDescriptorImageInfo tex_descs[DEMO_TEXTURE_COUNT];
    VkWriteDescriptorSet writes[2];
    VkResult U_ASSERT_ONLY err;

    VkDescriptorSetAllocateInfo alloc_info = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = NULL,
            .descriptorPool = vulkanDSL->desc_pool,
            .descriptorSetCount = 1,
            .pSetLayouts = &vulkanDSL->desc_layout};

    VkDescriptorBufferInfo buffer_info;
    buffer_info.offset = 0;
    buffer_info.range = sizeof(struct vktexcube_vs_uniform);

    memset(&tex_descs, 0, sizeof(tex_descs));
    for (unsigned int i = 0; i < DEMO_TEXTURE_COUNT; i++) {
        tex_descs[i].sampler = vulkanDSL->textures[i].sampler;
        tex_descs[i].imageView = vulkanDSL->textures[i].view;
        tex_descs[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    memset(&writes, 0, sizeof(writes));

    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].descriptorCount = 1;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writes[0].pBufferInfo = &buffer_info;

    writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[1].dstBinding = 1;
    writes[1].descriptorCount = DEMO_TEXTURE_COUNT;
    writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[1].pImageInfo = tex_descs;

    for (unsigned int i = 0; i < vulkanDSL->swapchainImageCount; i++) {
        err = vkAllocateDescriptorSets(vulkanDSL->device, &alloc_info, &vulkanDSL->swapchain_image_resources[i].descriptor_set);
        assert(!err);
        buffer_info.buffer = vulkanDSL->swapchain_image_resources[i].uniform_buffer;
        writes[0].dstSet = vulkanDSL->swapchain_image_resources[i].descriptor_set;
        writes[1].dstSet = vulkanDSL->swapchain_image_resources[i].descriptor_set;
        vkUpdateDescriptorSets(vulkanDSL->device, 2, writes, 0, NULL);
    }
}

static void demo_prepare_framebuffers(struct VulkanDSL *vulkanDSL) {
    VkImageView attachments[3];
    attachments[0] = vulkanDSL->colorImageViewMultisample;
    attachments[1] = vulkanDSL->depth.view;

    const VkFramebufferCreateInfo fb_info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext = NULL,
            .renderPass = vulkanDSL->render_pass,
            .attachmentCount = 3,
            //.attachmentCount = 2,
            //.attachmentCount = 1,
            .pAttachments = attachments,
            .width = vulkanDSL->width,
            .height = vulkanDSL->height,
            .layers = 1,
    };
    VkResult U_ASSERT_ONLY err;
    uint32_t i;

    for (i = 0; i < vulkanDSL->swapchainImageCount; i++) {
        attachments[2] = vulkanDSL->swapchain_image_resources[i].view;
        err = vkCreateFramebuffer(vulkanDSL->device, &fb_info, NULL, &vulkanDSL->swapchain_image_resources[i].framebuffer);
        assert(!err);
    }
}

void demo_prepare(struct VulkanDSL *vulkanDSL) {
    demo_prepare_buffers(vulkanDSL);

    if (vulkanDSL->is_minimized) {
        vulkanDSL->prepared = false;
        return;
    }

    VkResult U_ASSERT_ONLY err;
    if (vulkanDSL->cmd_pool == VK_NULL_HANDLE) {
        const VkCommandPoolCreateInfo cmd_pool_info = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .pNext = NULL,
                .queueFamilyIndex = vulkanDSL->graphics_queue_family_index,
                .flags = 0,
        };
        err = vkCreateCommandPool(vulkanDSL->device, &cmd_pool_info, NULL, &vulkanDSL->cmd_pool);
        assert(!err);
    }

    tinyobj_attrib_t* outAttrib;
#ifdef __ANDROID__
    const char* objFile = "meshes/textPanel.obj";
#else
    const char* objFile = "textPanel.obj";
#endif
    AssetsFetcher__loadObj(&vulkanDSL->assetsFetcher, objFile, &outAttrib);

    // when change this also change the vkBindVertexBuffer
    //VulkanDSL__prepare_vertex_buffer_classic(vulkanDSL, outAttrib);
    VulkanDSL__prepare_vertex_buffer_gpu_only(vulkanDSL, outAttrib);

    const VkCommandBufferAllocateInfo cmd = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = NULL,
            .commandPool = vulkanDSL->cmd_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
    };
    err = vkAllocateCommandBuffers(vulkanDSL->device, &cmd, &vulkanDSL->cmd);
    assert(!err);
    VkCommandBufferBeginInfo cmd_buf_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = NULL,
            //.flags = 0,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            .pInheritanceInfo = NULL,
    };
    err = vkBeginCommandBuffer(vulkanDSL->cmd, &cmd_buf_info);
    assert(!err);

    // best full intel tutorial:
    // https://www.intel.com/content/www/us/en/developer/articles/training/api-without-secrets-introduction-to-vulkan-part-5.html
    // see the code at "if (isUnifiedGraphicsAndTransferQueue)"
    // https://github.com/cforfang/Vulkan-Tools/wiki/Synchronization-Examples
    // https://cpp-rendering.io/barriers-vulkan-not-difficult/
    // https://github.com/GameTechDev/IntroductionToVulkan/blob/master/Project/Tutorials/05/Tutorial05.cpp
    VkBufferCopy copy;
    copy.dstOffset = 0;
    copy.srcOffset = 0;
    //copy.size = vulkanDSL->assetsFetcher.arraySize;
    copy.size = 8 * sizeof(float) * 3 * 3;
    //vkCmdCopyBuffer(vulkanDSL->cmd, vulkanDSL->vertex_buffer_resources->vertex_buffer, vulkanDSL->vertex_buffer_resources->vertex_buffer_gpu, 1, &copy);
    VkBufferCopy copy2;
    copy.dstOffset = 8 * 6;
    copy.srcOffset = 8 * 6;
    copy.size = 32 * 8;
    //vkCmdCopyBuffer(vulkanDSL->cmd, vulkanDSL->vertex_buffer_resources->vertex_buffer, vulkanDSL->vertex_buffer_resources->vertex_buffer_gpu, 1, &copy2);

    VkBufferMemoryBarrier buffer_memory_barrier = {
            VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,          // VkStructureType                        sType;
            NULL,                                             // const void                            *pNext
            VK_ACCESS_MEMORY_WRITE_BIT,                       // VkAccessFlags                          srcAccessMask
            VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,              // VkAccessFlags                          dstAccessMask
            VK_QUEUE_FAMILY_IGNORED,                          // uint32_t                               srcQueueFamilyIndex
            VK_QUEUE_FAMILY_IGNORED,                          // uint32_t                               dstQueueFamilyIndex
 //           vulkanDSL->vertex_buffer_resources->vertex_buffer,// VkBuffer                               buffer
            vulkanDSL->vertex_buffer_resources->vertex_buffer_gpu,// VkBuffer                               buffer
            0,                                                // VkDeviceSize                           offset
            VK_WHOLE_SIZE                                     // VkDeviceSize                           size
    };
    //vkCmdPipelineBarrier(vulkanDSL->cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, NULL, 1, &buffer_memory_barrier, 0, NULL );

    VkMemoryBarrier memoryBarrier;
    memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    memoryBarrier.pNext = NULL;
    memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    memoryBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    //VkBufferMemoryBarrier
    //VkMemoryBarrier
    vkCmdPipelineBarrier(vulkanDSL->cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 1,
                         (const VkMemoryBarrier *) &memoryBarrier, 0, NULL, 0, NULL);

    /*
    VkMemoryBarrier memoryBarrier;
    memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    memoryBarrier.pNext = NULL;
    memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    memoryBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    //VkBufferMemoryBarrier
    //VkMemoryBarrier
    vkCmdPipelineBarrier(vulkanDSL->cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 0, NULL);
*/
    //vkCmdPipelineBarrier(vulkanDSL->cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
    //vkCmdPipelineBarrier(vulkanDSL->cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 1,
    //vkCmdPipelineBarrier(vulkanDSL->cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 1,
    //                     (const VkMemoryBarrier *) &memoryBarrier, 0, NULL, 0, NULL);

    demo_prepare_multisample_buffer(vulkanDSL);
    demo_prepare_depth(vulkanDSL);
    demo_prepare_textures(vulkanDSL);
    demo_prepare_cube_data_buffers(vulkanDSL);

    demo_prepare_descriptor_layout(vulkanDSL);
    demo_prepare_render_pass(vulkanDSL);
    demo_prepare_pipeline(vulkanDSL);

    for (uint32_t i = 0; i < vulkanDSL->swapchainImageCount; i++) {
        err = vkAllocateCommandBuffers(vulkanDSL->device, &cmd, &vulkanDSL->swapchain_image_resources[i].cmd);
        assert(!err);
    }

    if (vulkanDSL->separate_present_queue) {
        const VkCommandPoolCreateInfo present_cmd_pool_info = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .pNext = NULL,
                .queueFamilyIndex = vulkanDSL->present_queue_family_index,
                .flags = 0,
        };
        err = vkCreateCommandPool(vulkanDSL->device, &present_cmd_pool_info, NULL, &vulkanDSL->present_cmd_pool);
        assert(!err);
        const VkCommandBufferAllocateInfo present_cmd_info = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .pNext = NULL,
                .commandPool = vulkanDSL->present_cmd_pool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1,
        };
        for (uint32_t i = 0; i < vulkanDSL->swapchainImageCount; i++) {
            err = vkAllocateCommandBuffers(vulkanDSL->device, &present_cmd_info,
                                           &vulkanDSL->swapchain_image_resources[i].graphics_to_present_cmd);
            assert(!err);
            demo_build_image_ownership_cmd(vulkanDSL, i);
        }
    }

    demo_prepare_descriptor_pool(vulkanDSL);
    demo_prepare_descriptor_set(vulkanDSL);

    demo_prepare_framebuffers(vulkanDSL);

    for (uint32_t i = 0; i < vulkanDSL->swapchainImageCount; i++) {
        vulkanDSL->current_buffer = i;
        VulkanDSL__draw_build_cmd(vulkanDSL, vulkanDSL->swapchain_image_resources[i].cmd);
    }

     /*
     * Prepare functions above may generate pipeline commands
     * that need to be flushed before beginning the render loop.
     */
    // vulkanDSL->cmd is used to transfer the images and wait for them before the fragment shader
    demo_flush_init_cmd(vulkanDSL);

    if (vulkanDSL->staging_texture.buffer) {
        demo_destroy_texture(vulkanDSL, &vulkanDSL->staging_texture);
    }

    vulkanDSL->current_buffer = 0;
    vulkanDSL->prepared = true;
}

void VulkanDSL__half_cleanup(struct VulkanDSL *vulkanDSL) {
    int i;
    for (i = 0; i < vulkanDSL->swapchainImageCount; i++) {
        vkDestroyFramebuffer(vulkanDSL->device, vulkanDSL->swapchain_image_resources[i].framebuffer, NULL);
    }
    vkDestroyDescriptorPool(vulkanDSL->device, vulkanDSL->desc_pool, NULL);

    vkDestroyPipeline(vulkanDSL->device, vulkanDSL->pipeline, NULL);
    vkDestroyPipelineCache(vulkanDSL->device, vulkanDSL->pipelineCache, NULL);
    vkDestroyRenderPass(vulkanDSL->device, vulkanDSL->render_pass, NULL);
    vkDestroyPipelineLayout(vulkanDSL->device, vulkanDSL->pipeline_layout, NULL);
    vkDestroyDescriptorSetLayout(vulkanDSL->device, vulkanDSL->desc_layout, NULL);

    for (i = 0; i < DEMO_TEXTURE_COUNT; i++) {
        vkDestroyImageView(vulkanDSL->device, vulkanDSL->textures[i].view, NULL);
        vkDestroyImage(vulkanDSL->device, vulkanDSL->textures[i].image, NULL);
        vkFreeMemory(vulkanDSL->device, vulkanDSL->textures[i].mem, NULL);
        vkDestroySampler(vulkanDSL->device, vulkanDSL->textures[i].sampler, NULL);
        //vkDestroySampler(vulkanDSL->device, vulkanDSL->textures[i].sampler, NULL);
    }

    vkDestroyImageView(vulkanDSL->device, vulkanDSL->depth.view, NULL);
    vkDestroyImage(vulkanDSL->device, vulkanDSL->depth.image, NULL);
    vkFreeMemory(vulkanDSL->device, vulkanDSL->depth.mem, NULL);

    vkDestroyImageView(vulkanDSL->device, vulkanDSL->colorImageViewMultisample, NULL);
    vkDestroyImage(vulkanDSL->device, vulkanDSL->colorImageMultisample, NULL);
    vkFreeMemory(vulkanDSL->device, vulkanDSL->colorImageMemoryMultisample, NULL);

    for (i = 0; i < vulkanDSL->swapchainImageCount; i++) {
        vkDestroyImageView(vulkanDSL->device, vulkanDSL->swapchain_image_resources[i].view, NULL);
        vkFreeCommandBuffers(vulkanDSL->device, vulkanDSL->cmd_pool, 1, &vulkanDSL->swapchain_image_resources[i].cmd);
        vkDestroyBuffer(vulkanDSL->device, vulkanDSL->swapchain_image_resources[i].uniform_buffer, NULL);
        vkUnmapMemory(vulkanDSL->device, vulkanDSL->swapchain_image_resources[i].uniform_memory);
        vkFreeMemory(vulkanDSL->device, vulkanDSL->swapchain_image_resources[i].uniform_memory, NULL);
    }
    if (vulkanDSL->vertex_buffer_resources->vertex_buffer_allocated) {
        vkDestroyBuffer(vulkanDSL->device, vulkanDSL->vertex_buffer_resources->vertex_buffer,
                        NULL);
        if (vulkanDSL->vertex_buffer_resources->vertex_memory_mapped) {
            vkUnmapMemory(vulkanDSL->device, vulkanDSL->vertex_buffer_resources->vertex_memory);
        }
        vkFreeMemory(vulkanDSL->device, vulkanDSL->vertex_buffer_resources->vertex_memory,
                     NULL);
    }
    if (vulkanDSL->vertex_buffer_resources->vertex_buffer_gpu_allocated) {
        vkDestroyBuffer(vulkanDSL->device, vulkanDSL->vertex_buffer_resources->vertex_buffer_gpu,
                        NULL);
        vkFreeMemory(vulkanDSL->device, vulkanDSL->vertex_buffer_resources->vertex_memory_gpu,
                     NULL);
    }
    vkDestroyCommandPool(vulkanDSL->device, vulkanDSL->cmd_pool, NULL);
    vulkanDSL->cmd_pool = VK_NULL_HANDLE;
    if (vulkanDSL->separate_present_queue) {
        vkDestroyCommandPool(vulkanDSL->device, vulkanDSL->present_cmd_pool, NULL);
    }
    free(vulkanDSL->swapchain_image_resources);
    free(vulkanDSL->vertex_buffer_resources);
}

void VulkanDSL__cleanup(struct VulkanDSL *vulkanDSL) {
    VulkanDSL__freeResources(vulkanDSL);

    uint32_t i;

    vulkanDSL->prepared = false;
    vkDeviceWaitIdle(vulkanDSL->device);

    // Wait for fences from present operations
    for (i = 0; i < FRAME_LAG; i++) {
        vkWaitForFences(vulkanDSL->device, 1, &vulkanDSL->fences[i], VK_TRUE, UINT64_MAX);
        vkDestroyFence(vulkanDSL->device, vulkanDSL->fences[i], NULL);
        vkDestroySemaphore(vulkanDSL->device, vulkanDSL->image_acquired_semaphores[i], NULL);
        vkDestroySemaphore(vulkanDSL->device, vulkanDSL->draw_complete_semaphores[i], NULL);
        if (vulkanDSL->separate_present_queue) {
            vkDestroySemaphore(vulkanDSL->device, vulkanDSL->image_ownership_semaphores[i], NULL);
        }
    }

    // If the window is currently minimized, demo_resize has already done some cleanup for us.
    if (!vulkanDSL->is_minimized) {
        VulkanDSL__half_cleanup(vulkanDSL);
    }
    vkDeviceWaitIdle(vulkanDSL->device);
    vkDestroyDevice(vulkanDSL->device, NULL);
    if (vulkanDSL->validate) {
        vulkanDSL->DestroyDebugUtilsMessengerEXT(vulkanDSL->inst, vulkanDSL->dbg_messenger, NULL);
    }
    vkDestroySurfaceKHR(vulkanDSL->inst, vulkanDSL->surface, NULL);
    vkDestroyInstance(vulkanDSL->inst, NULL);
}

void demo_resize(struct VulkanDSL *vulkanDSL) {
    uint32_t i;

    // Don't react to resize until after first initialization.
    if (!vulkanDSL->prepared) {
        if (vulkanDSL->is_minimized) {
            demo_prepare(vulkanDSL);
        }
        return;
    }
    // In order to properly resize the window, we must re-create the swapchain
    // AND redo the command buffers, etc.
    //
    // First, perform part of the demo_cleanup() function:
    vulkanDSL->prepared = false;
    vkDeviceWaitIdle(vulkanDSL->device);

    VulkanDSL__half_cleanup(vulkanDSL);

    // Second, re-perform the demo_prepare() function, which will re-create the
    // swapchain:
    demo_prepare(vulkanDSL);
}

/*
 * Return 1 (true) if all layer names specified in check_names
 * can be found in given layer properties.
 */
static VkBool32 demo_check_layers(uint32_t check_count, char **check_names, uint32_t layer_count, VkLayerProperties *layers) {
    for (uint32_t i = 0; i < check_count; i++) {
        VkBool32 found = 0;
        for (uint32_t j = 0; j < layer_count; j++) {
            if (!strcmp(check_names[i], layers[j].layerName)) {
                found = 1;
                break;
            }
        }
        if (!found) {
            fprintf(stderr, "Cannot find layer: %s\n", check_names[i]);
            return 0;
        }
    }
    return 1;
}
#if defined(VK_USE_PLATFORM_DISPLAY_KHR)
int find_display_gpu(int gpu_number, uint32_t gpu_count, VkPhysicalDevice *physical_devices) {
    uint32_t display_count = 0;
    VkResult result;
    int gpu_return = gpu_number;
    if (gpu_number >= 0) {
        result = vkGetPhysicalDeviceDisplayPropertiesKHR(physical_devices[gpu_number], &display_count, NULL);
        assert(!result);
    } else {
        for (uint32_t i = 0; i < gpu_count; i++) {
            result = vkGetPhysicalDeviceDisplayPropertiesKHR(physical_devices[i], &display_count, NULL);
            assert(!result);
            if (display_count) {
                gpu_return = i;
                break;
            }
        }
    }
    if (display_count > 0)
        return gpu_return;
    else
        return -1;
}
#endif
static void demo_init_vk(struct VulkanDSL *vulkanDSL) {
    VkResult err;
    uint32_t instance_extension_count = 0;
    uint32_t instance_layer_count = 0;
    char *instance_validation_layers[] = {"VK_LAYER_KHRONOS_validation"};
    vulkanDSL->enabled_extension_count = 0;
    vulkanDSL->enabled_layer_count = 0;
    vulkanDSL->is_minimized = false;
    vulkanDSL->cmd_pool = VK_NULL_HANDLE;

    // Look for validation layers
    VkBool32 validation_found = 0;
    if (vulkanDSL->validate) {
        err = vkEnumerateInstanceLayerProperties(&instance_layer_count, NULL);
        assert(!err);

        if (instance_layer_count > 0) {
            VkLayerProperties *instance_layers = malloc(sizeof(VkLayerProperties) * instance_layer_count);
            err = vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layers);
            assert(!err);

            validation_found = demo_check_layers(ARRAY_SIZE(instance_validation_layers), instance_validation_layers,
                                                 instance_layer_count, instance_layers);
            if (validation_found) {
                vulkanDSL->enabled_layer_count = ARRAY_SIZE(instance_validation_layers);
                vulkanDSL->enabled_layers[0] = "VK_LAYER_KHRONOS_validation";
            }
            free(instance_layers);
        }

        if (!validation_found) {
            ERR_EXIT(
                    "vkEnumerateInstanceLayerProperties failed to find required validation layer.\n\n"
                    "Please look at the Getting Started guide for additional information.\n",
                    "vkCreateInstance Failure");
        }
    }

    /* Look for instance extensions */
    VkBool32 surfaceExtFound = 0;
    VkBool32 platformSurfaceExtFound = 0;
    memset(vulkanDSL->extension_names, 0, sizeof(vulkanDSL->extension_names));

    err = vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, NULL);
    assert(!err);

    if (instance_extension_count > 0) {
        VkExtensionProperties *instance_extensions = malloc(sizeof(VkExtensionProperties) * instance_extension_count);
        err = vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, instance_extensions);
        assert(!err);
        for (uint32_t i = 0; i < instance_extension_count; i++) {
            //fprintf(stdout, "%s\n", instance_extensions[i].extensionName);
            if (!strcmp(VK_KHR_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName)) {
                surfaceExtFound = 1;
                vulkanDSL->extension_names[vulkanDSL->enabled_extension_count++] = VK_KHR_SURFACE_EXTENSION_NAME;
            }
#if defined(VK_USE_PLATFORM_WIN32_KHR)
            if (!strcmp(VK_KHR_WIN32_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName)) {
                platformSurfaceExtFound = 1;
                vulkanDSL->extension_names[vulkanDSL->enabled_extension_count++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
            }
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
            if (!strcmp(VK_KHR_XLIB_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName)) {
                platformSurfaceExtFound = 1;
                vulkanDSL->extension_names[vulkanDSL->enabled_extension_count++] = VK_KHR_XLIB_SURFACE_EXTENSION_NAME;
            }
#elif defined(VK_USE_PLATFORM_XCB_KHR)
            if (!strcmp(VK_KHR_XCB_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName)) {
                platformSurfaceExtFound = 1;
                vulkanDSL->extension_names[vulkanDSL->enabled_extension_count++] = VK_KHR_XCB_SURFACE_EXTENSION_NAME;
            }
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
            if (!strcmp(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName)) {
                platformSurfaceExtFound = 1;
                vulkanDSL->extension_names[vulkanDSL->enabled_extension_count++] = VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME;
            }
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
            if (!strcmp(VK_EXT_DIRECTFB_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName)) {
                platformSurfaceExtFound = 1;
                vulkanDSL->extension_names[vulkanDSL->enabled_extension_count++] = VK_EXT_DIRECTFB_SURFACE_EXTENSION_NAME;
            }
#elif defined(VK_USE_PLATFORM_DISPLAY_KHR)
            if (!strcmp(VK_KHR_DISPLAY_EXTENSION_NAME, instance_extensions[i].extensionName)) {
                platformSurfaceExtFound = 1;
                vulkanDSL->extension_names[vulkanDSL->enabled_extension_count++] = VK_KHR_DISPLAY_EXTENSION_NAME;
            }
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
            if (!strcmp(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName)) {
                platformSurfaceExtFound = 1;
                vulkanDSL->extension_names[vulkanDSL->enabled_extension_count++] = VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
            }
#elif defined(VK_USE_PLATFORM_METAL_EXT)
            if (!strcmp(VK_EXT_METAL_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName)) {
                platformSurfaceExtFound = 1;
                vulkanDSL->extension_names[vulkanDSL->enabled_extension_count++] = VK_EXT_METAL_SURFACE_EXTENSION_NAME;
            }
#endif
            if (!strcmp(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, instance_extensions[i].extensionName)) {
                vulkanDSL->extension_names[vulkanDSL->enabled_extension_count++] = VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME;
            }
            if (!strcmp(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, instance_extensions[i].extensionName)) {
                if (vulkanDSL->validate) {
                    vulkanDSL->extension_names[vulkanDSL->enabled_extension_count++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
                }
            }
            assert(vulkanDSL->enabled_extension_count < 64);
        }

        free(instance_extensions);
    }

    if (!surfaceExtFound) {
        ERR_EXIT("vkEnumerateInstanceExtensionProperties failed to find the " VK_KHR_SURFACE_EXTENSION_NAME
                         " extension.\n\n"
                         "Do you have a compatible Vulkan installable client driver (ICD) installed?\n"
                         "Please look at the Getting Started guide for additional information.\n",
                 "vkCreateInstance Failure");
    }
    if (!platformSurfaceExtFound) {
#if defined(VK_USE_PLATFORM_WIN32_KHR)
        ERR_EXIT("vkEnumerateInstanceExtensionProperties failed to find the " VK_KHR_WIN32_SURFACE_EXTENSION_NAME
                 " extension.\n\n"
                 "Do you have a compatible Vulkan installable client driver (ICD) installed?\n"
                 "Please look at the Getting Started guide for additional information.\n",
                 "vkCreateInstance Failure");
#elif defined(VK_USE_PLATFORM_METAL_EXT)
        ERR_EXIT("vkEnumerateInstanceExtensionProperties failed to find the " VK_EXT_METAL_SURFACE_EXTENSION_NAME
                 " extension.\n\n"
                 "Do you have a compatible Vulkan installable client driver (ICD) installed?\n"
                 "Please look at the Getting Started guide for additional information.\n",
                 "vkCreateInstance Failure");
#elif defined(VK_USE_PLATFORM_XCB_KHR)
        ERR_EXIT("vkEnumerateInstanceExtensionProperties failed to find the " VK_KHR_XCB_SURFACE_EXTENSION_NAME
                 " extension.\n\n"
                 "Do you have a compatible Vulkan installable client driver (ICD) installed?\n"
                 "Please look at the Getting Started guide for additional information.\n",
                 "vkCreateInstance Failure");
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
        ERR_EXIT("vkEnumerateInstanceExtensionProperties failed to find the " VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME
                 " extension.\n\n"
                 "Do you have a compatible Vulkan installable client driver (ICD) installed?\n"
                 "Please look at the Getting Started guide for additional information.\n",
                 "vkCreateInstance Failure");
#elif defined(VK_USE_PLATFORM_DISPLAY_KHR)
        ERR_EXIT("vkEnumerateInstanceExtensionProperties failed to find the " VK_KHR_DISPLAY_EXTENSION_NAME
                 " extension.\n\n"
                 "Do you have a compatible Vulkan installable client driver (ICD) installed?\n"
                 "Please look at the Getting Started guide for additional information.\n",
                 "vkCreateInstance Failure");
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
        ERR_EXIT("vkEnumerateInstanceExtensionProperties failed to find the " VK_KHR_ANDROID_SURFACE_EXTENSION_NAME
                         " extension.\n\n"
                         "Do you have a compatible Vulkan installable client driver (ICD) installed?\n"
                         "Please look at the Getting Started guide for additional information.\n",
                 "vkCreateInstance Failure");
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
        ERR_EXIT("vkEnumerateInstanceExtensionProperties failed to find the " VK_KHR_XLIB_SURFACE_EXTENSION_NAME
                 " extension.\n\n"
                 "Do you have a compatible Vulkan installable client driver (ICD) installed?\n"
                 "Please look at the Getting Started guide for additional information.\n",
                 "vkCreateInstance Failure");
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
        ERR_EXIT("vkEnumerateInstanceExtensionProperties failed to find the " VK_EXT_DIRECTFB_SURFACE_EXTENSION_NAME
                 " extension.\n\n"
                 "Do you have a compatible Vulkan installable client driver (ICD) installed?\n"
                 "Please look at the Getting Started guide for additional information.\n",
                 "vkCreateInstance Failure");
#endif
    }
    const VkApplicationInfo app = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = NULL,
            .pApplicationName = APP_SHORT_NAME,
            .applicationVersion = 0,
            .pEngineName = APP_SHORT_NAME,
            .engineVersion = 0,
            // MoltenVK supports only 1.1
            // #define VULKAN_1_1 is defined in the volk_setup.h
            .apiVersion = VK_API_VERSION_1_1,
    };
    // spec 1.1, different from the latest one
    // https://www.khronos.org/registry/vulkan/specs/1.1/html/vkspec.html
    VkInstanceCreateInfo inst_info = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = NULL,
            .pApplicationInfo = &app,
            .enabledLayerCount = vulkanDSL->enabled_layer_count,
            .ppEnabledLayerNames = (const char *const *)instance_validation_layers,
            .enabledExtensionCount = vulkanDSL->enabled_extension_count,
            .ppEnabledExtensionNames = (const char *const *)vulkanDSL->extension_names,
    };

    /*
     * This is info for a temp callback to use during CreateInstance.
     * After the instance is created, we use the instance-based
     * function to register the final callback.
     */
    VkDebugUtilsMessengerCreateInfoEXT dbg_messenger_create_info;
    if (vulkanDSL->validate) {
        // VK_EXT_debug_utils style
        dbg_messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        dbg_messenger_create_info.pNext = NULL;
        dbg_messenger_create_info.flags = 0;
        dbg_messenger_create_info.messageSeverity =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        dbg_messenger_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        dbg_messenger_create_info.pfnUserCallback = debug_messenger_callback;
        dbg_messenger_create_info.pUserData = vulkanDSL;
        inst_info.pNext = &dbg_messenger_create_info;
    }

    err = vkCreateInstance(&inst_info, NULL, &vulkanDSL->inst);
    if (err == VK_ERROR_INCOMPATIBLE_DRIVER) {
        ERR_EXIT(
                "Cannot find a compatible Vulkan installable client driver (ICD).\n\n"
                "Please look at the Getting Started guide for additional information.\n",
                "vkCreateInstance Failure");
    } else if (err == VK_ERROR_EXTENSION_NOT_PRESENT) {
        ERR_EXIT(
                "Cannot find a specified extension library.\n"
                "Make sure your layers path is set appropriately.\n",
                "vkCreateInstance Failure");
    } else if (err) {
        ERR_EXIT(
                "vkCreateInstance failed.\n\n"
                "Do you have a compatible Vulkan installable client driver (ICD) installed?\n"
                "Please look at the Getting Started guide for additional information.\n",
                "vkCreateInstance Failure");
    }

#ifdef __ANDROID__
    volkLoadInstance(vulkanDSL->inst);
#endif

    /* Make initial call to query gpu_count, then second call for gpu info */
    uint32_t gpu_count = 0;
    err = vkEnumeratePhysicalDevices(vulkanDSL->inst, &gpu_count, NULL);
    assert(!err);

    if (gpu_count <= 0) {
        ERR_EXIT(
                "vkEnumeratePhysicalDevices reported zero accessible devices.\n\n"
                "Do you have a compatible Vulkan installable client driver (ICD) installed?\n"
                "Please look at the Getting Started guide for additional information.\n",
                "vkEnumeratePhysicalDevices Failure");
    }

    VkPhysicalDevice *physical_devices = malloc(sizeof(VkPhysicalDevice) * gpu_count);
    err = vkEnumeratePhysicalDevices(vulkanDSL->inst, &gpu_count, physical_devices);
    assert(!err);
    if (vulkanDSL->gpu_number >= 0 && !((uint32_t)vulkanDSL->gpu_number < gpu_count)) {
        fprintf(stderr, "GPU %d specified is not present, GPU count = %u\n", vulkanDSL->gpu_number, gpu_count);
        ERR_EXIT("Specified GPU number is not present", "User Error");
    }

#if defined(VK_USE_PLATFORM_DISPLAY_KHR)
    vulkanDSL->gpu_number = find_display_gpu(vulkanDSL->gpu_number, gpu_count, physical_devices);
    if (vulkanDSL->gpu_number < 0) {
        printf("Cannot find any display!\n");
        fflush(stdout);
        exit(1);
    }
#else
    /* Try to auto select most suitable device */
    if (vulkanDSL->gpu_number == -1) {
        uint32_t count_device_type[VK_PHYSICAL_DEVICE_TYPE_CPU + 1];
        memset(count_device_type, 0, sizeof(count_device_type));

        VkPhysicalDeviceProperties physicalDeviceProperties;
        for (uint32_t i = 0; i < gpu_count; i++) {
            vkGetPhysicalDeviceProperties(physical_devices[i], &physicalDeviceProperties);
            assert(physicalDeviceProperties.deviceType <= VK_PHYSICAL_DEVICE_TYPE_CPU);
            count_device_type[physicalDeviceProperties.deviceType]++;
        }
        // nonCoherentAtomSize is the size and alignment in bytes that bounds concurrent access
        // to host-mapped device memory. The value must be a power of two.
        // size_t nonCoherentAtomSize = physicalDeviceProperties.limits.nonCoherentAtomSize;
        VkPhysicalDeviceType search_for_device_type = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
        if (count_device_type[VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU]) {
            search_for_device_type = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
        } else if (count_device_type[VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU]) {
            search_for_device_type = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
        } else if (count_device_type[VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU]) {
            search_for_device_type = VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU;
        } else if (count_device_type[VK_PHYSICAL_DEVICE_TYPE_CPU]) {
            search_for_device_type = VK_PHYSICAL_DEVICE_TYPE_CPU;
        } else if (count_device_type[VK_PHYSICAL_DEVICE_TYPE_OTHER]) {
            search_for_device_type = VK_PHYSICAL_DEVICE_TYPE_OTHER;
        }

        for (uint32_t i = 0; i < gpu_count; i++) {
            vkGetPhysicalDeviceProperties(physical_devices[i], &physicalDeviceProperties);
            if (physicalDeviceProperties.deviceType == search_for_device_type) {
                vulkanDSL->gpu_number = i;
                break;
            }
        }
    }
#endif
    assert(vulkanDSL->gpu_number >= 0);
    vulkanDSL->gpu = physical_devices[vulkanDSL->gpu_number];
    {
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(vulkanDSL->gpu, &physicalDeviceProperties);
        fprintf(stderr, "Selected GPU %d: %s, type: %u\n", vulkanDSL->gpu_number, physicalDeviceProperties.deviceName,
                physicalDeviceProperties.deviceType);
    }
    vulkanDSL->msaaSamples = getMaxUsableSampleCount(vulkanDSL);
    free(physical_devices);

    /* Look for device extensions */
    uint32_t device_extension_count = 0;
    VkBool32 swapchainExtFound = 0;
    vulkanDSL->enabled_extension_count = 0;
    memset(vulkanDSL->extension_names, 0, sizeof(vulkanDSL->extension_names));

    err = vkEnumerateDeviceExtensionProperties(vulkanDSL->gpu, NULL, &device_extension_count, NULL);
    assert(!err);

    if (device_extension_count > 0) {
        VkExtensionProperties *device_extensions = malloc(sizeof(VkExtensionProperties) * device_extension_count);
        err = vkEnumerateDeviceExtensionProperties(vulkanDSL->gpu, NULL, &device_extension_count, device_extensions);
        assert(!err);

        for (uint32_t i = 0; i < device_extension_count; i++) {
            if (!strcmp(VK_KHR_SWAPCHAIN_EXTENSION_NAME, device_extensions[i].extensionName)) {
                swapchainExtFound = 1;
                vulkanDSL->extension_names[vulkanDSL->enabled_extension_count++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
            }
            if (!strcmp("VK_KHR_portability_subset", device_extensions[i].extensionName)) {
                vulkanDSL->extension_names[vulkanDSL->enabled_extension_count++] = "VK_KHR_portability_subset";
            }
            assert(vulkanDSL->enabled_extension_count < 64);
        }

        if (vulkanDSL->VK_KHR_incremental_present_enabled) {
            // Even though the user "enabled" the extension via the command
            // line, we must make sure that it's enumerated for use with the
            // device.  Therefore, disable it here, and re-enable it again if
            // enumerated.
            vulkanDSL->VK_KHR_incremental_present_enabled = false;
            for (uint32_t i = 0; i < device_extension_count; i++) {
                if (!strcmp(VK_KHR_INCREMENTAL_PRESENT_EXTENSION_NAME, device_extensions[i].extensionName)) {
                    vulkanDSL->extension_names[vulkanDSL->enabled_extension_count++] = VK_KHR_INCREMENTAL_PRESENT_EXTENSION_NAME;
                    vulkanDSL->VK_KHR_incremental_present_enabled = true;
                    DbgMsg("VK_KHR_incremental_present extension enabled\n");
                }
                assert(vulkanDSL->enabled_extension_count < 64);
            }
            if (!vulkanDSL->VK_KHR_incremental_present_enabled) {
                DbgMsg("VK_KHR_incremental_present extension NOT AVAILABLE\n");
            }
        }

        if (vulkanDSL->VK_GOOGLE_display_timing_enabled) {
            // Even though the user "enabled" the extension via the command
            // line, we must make sure that it's enumerated for use with the
            // device.  Therefore, disable it here, and re-enable it again if
            // enumerated.
            vulkanDSL->VK_GOOGLE_display_timing_enabled = false;
            for (uint32_t i = 0; i < device_extension_count; i++) {
                if (!strcmp(VK_GOOGLE_DISPLAY_TIMING_EXTENSION_NAME, device_extensions[i].extensionName)) {
                    vulkanDSL->extension_names[vulkanDSL->enabled_extension_count++] = VK_GOOGLE_DISPLAY_TIMING_EXTENSION_NAME;
                    vulkanDSL->VK_GOOGLE_display_timing_enabled = true;
                    DbgMsg("VK_GOOGLE_display_timing extension enabled\n");
                }
                assert(vulkanDSL->enabled_extension_count < 64);
            }
            if (!vulkanDSL->VK_GOOGLE_display_timing_enabled) {
                DbgMsg("VK_GOOGLE_display_timing extension NOT AVAILABLE\n");
            }
        }

        free(device_extensions);
    }

    if (!swapchainExtFound) {
        ERR_EXIT("vkEnumerateDeviceExtensionProperties failed to find the " VK_KHR_SWAPCHAIN_EXTENSION_NAME
                         " extension.\n\nDo you have a compatible Vulkan installable client driver (ICD) installed?\n"
                         "Please look at the Getting Started guide for additional information.\n",
                 "vkCreateInstance Failure");
    }

    if (vulkanDSL->validate) {
        // Setup VK_EXT_debug_utils function pointers always (we use them for
        // debug labels and names).
        vulkanDSL->CreateDebugUtilsMessengerEXT =
                (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vulkanDSL->inst, "vkCreateDebugUtilsMessengerEXT");
        vulkanDSL->DestroyDebugUtilsMessengerEXT =
                (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vulkanDSL->inst, "vkDestroyDebugUtilsMessengerEXT");
        vulkanDSL->SubmitDebugUtilsMessageEXT =
                (PFN_vkSubmitDebugUtilsMessageEXT)vkGetInstanceProcAddr(vulkanDSL->inst, "vkSubmitDebugUtilsMessageEXT");
        vulkanDSL->CmdBeginDebugUtilsLabelEXT =
                (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(vulkanDSL->inst, "vkCmdBeginDebugUtilsLabelEXT");
        vulkanDSL->CmdEndDebugUtilsLabelEXT =
                (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(vulkanDSL->inst, "vkCmdEndDebugUtilsLabelEXT");
        vulkanDSL->CmdInsertDebugUtilsLabelEXT =
                (PFN_vkCmdInsertDebugUtilsLabelEXT)vkGetInstanceProcAddr(vulkanDSL->inst, "vkCmdInsertDebugUtilsLabelEXT");
        vulkanDSL->SetDebugUtilsObjectNameEXT =
                (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(vulkanDSL->inst, "vkSetDebugUtilsObjectNameEXT");
        if (NULL == vulkanDSL->CreateDebugUtilsMessengerEXT || NULL == vulkanDSL->DestroyDebugUtilsMessengerEXT ||
            NULL == vulkanDSL->SubmitDebugUtilsMessageEXT || NULL == vulkanDSL->CmdBeginDebugUtilsLabelEXT ||
            NULL == vulkanDSL->CmdEndDebugUtilsLabelEXT || NULL == vulkanDSL->CmdInsertDebugUtilsLabelEXT ||
            NULL == vulkanDSL->SetDebugUtilsObjectNameEXT) {
            ERR_EXIT("GetProcAddr: Failed to init VK_EXT_debug_utils\n", "GetProcAddr: Failure");
        }

        err = vulkanDSL->CreateDebugUtilsMessengerEXT(vulkanDSL->inst, &dbg_messenger_create_info, NULL, &vulkanDSL->dbg_messenger);
        switch (err) {
            case VK_SUCCESS:
                break;
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                ERR_EXIT("CreateDebugUtilsMessengerEXT: out of host memory\n", "CreateDebugUtilsMessengerEXT Failure");
                break;
            default:
                ERR_EXIT("CreateDebugUtilsMessengerEXT: unknown failure\n", "CreateDebugUtilsMessengerEXT Failure");
                break;
        }
    }
    vkGetPhysicalDeviceProperties(vulkanDSL->gpu, &vulkanDSL->gpu_props);

    /* Call with NULL data to get count */
    vkGetPhysicalDeviceQueueFamilyProperties(vulkanDSL->gpu, &vulkanDSL->queue_family_count, NULL);
    assert(vulkanDSL->queue_family_count >= 1);

    vulkanDSL->queue_props = (VkQueueFamilyProperties *)malloc(vulkanDSL->queue_family_count * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(vulkanDSL->gpu, &vulkanDSL->queue_family_count, vulkanDSL->queue_props);

    // Query fine-grained feature support for this device.
    //  If app has specific feature requirements it should check supported
    //  features based on this query
    VkPhysicalDeviceFeatures physDevFeatures;
    vkGetPhysicalDeviceFeatures(vulkanDSL->gpu, &physDevFeatures);

    GET_INSTANCE_PROC_ADDR(vulkanDSL->inst, GetPhysicalDeviceSurfaceSupportKHR);
    GET_INSTANCE_PROC_ADDR(vulkanDSL->inst, GetPhysicalDeviceSurfaceCapabilitiesKHR);
    GET_INSTANCE_PROC_ADDR(vulkanDSL->inst, GetPhysicalDeviceSurfaceFormatsKHR);
    GET_INSTANCE_PROC_ADDR(vulkanDSL->inst, GetPhysicalDeviceSurfacePresentModesKHR);
    GET_INSTANCE_PROC_ADDR(vulkanDSL->inst, GetSwapchainImagesKHR);
}

static void demo_create_device(struct VulkanDSL *vulkanDSL) {
    VkResult U_ASSERT_ONLY err;
    float queue_priorities[1] = {0.0};
    VkDeviceQueueCreateInfo queues[1];
    // in the spec, we cannot have the same queue family on separate VkDeviceQueueCreateInfos
    queues[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queues[0].pNext = NULL;
    queues[0].queueFamilyIndex = vulkanDSL->graphics_queue_family_index;
    queues[0].queueCount = 1;
    queues[0].pQueuePriorities = queue_priorities;
    queues[0].flags = 0;

    // multisampling
    VkPhysicalDeviceFeatures deviceFeatures;
    memset(&deviceFeatures, 0, sizeof(VkPhysicalDeviceFeatures));
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.sampleRateShading = VK_TRUE;

    VkDeviceCreateInfo device = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = NULL,
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = queues,
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = NULL,
            .enabledExtensionCount = vulkanDSL->enabled_extension_count,
            .ppEnabledExtensionNames = (const char *const *)vulkanDSL->extension_names,
            .pEnabledFeatures = &deviceFeatures,  // If specific features are required, pass them in here
    };
    /*
    if (vulkanDSL->separate_present_queue) {
          queues[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
          queues[1].pNext = NULL;
          queues[1].queueFamilyIndex = vulkanDSL->present_queue_family_index;
          queues[1].queueCount = 1;
          queues[1].pQueuePriorities = queue_priorities;
          queues[1].flags = 0;
          device.queueCreateInfoCount = 2;
      }
    */

    device.queueCreateInfoCount = 1;

    err = vkCreateDevice(vulkanDSL->gpu, &device, NULL, &vulkanDSL->device);
    assert(!err);
}

static void demo_create_surface(struct VulkanDSL *vulkanDSL) {
    VkResult U_ASSERT_ONLY err;

// Create a WSI surface for the window:
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    VkWin32SurfaceCreateInfoKHR createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.hinstance = vulkanDSL->connection;
    createInfo.hwnd = vulkanDSL->window;

    err = vkCreateWin32SurfaceKHR(vulkanDSL->inst, &createInfo, NULL, &vulkanDSL->surface);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    VkWaylandSurfaceCreateInfoKHR createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.display = vulkanDSL->display;
    createInfo.surface = vulkanDSL->window;

    err = vkCreateWaylandSurfaceKHR(vulkanDSL->inst, &createInfo, NULL, &vulkanDSL->surface);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    VkAndroidSurfaceCreateInfoKHR createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.window = (struct ANativeWindow *)(vulkanDSL->window);

    err = vkCreateAndroidSurfaceKHR(vulkanDSL->inst, &createInfo, NULL, &vulkanDSL->surface);
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
    VkXlibSurfaceCreateInfoKHR createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.dpy = vulkanDSL->display;
    createInfo.window = vulkanDSL->xlib_window;

    err = vkCreateXlibSurfaceKHR(vulkanDSL->inst, &createInfo, NULL, &vulkanDSL->surface);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
    VkXcbSurfaceCreateInfoKHR createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.connection = vulkanDSL->connection;
    createInfo.window = vulkanDSL->xcb_window;

    err = vkCreateXcbSurfaceKHR(vulkanDSL->inst, &createInfo, NULL, &vulkanDSL->surface);
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
    VkDirectFBSurfaceCreateInfoEXT createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_DIRECTFB_SURFACE_CREATE_INFO_EXT;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.dfb = vulkanDSL->dfb;
    createInfo.surface = vulkanDSL->window;

    err = vkCreateDirectFBSurfaceEXT(vulkanDSL->inst, &createInfo, NULL, &vulkanDSL->surface);
#elif defined(VK_USE_PLATFORM_DISPLAY_KHR)
    err = demo_create_display_surface(vulkanDSL);
#elif defined(VK_USE_PLATFORM_METAL_EXT)
    VkMetalSurfaceCreateInfoEXT surface;
    surface.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
    surface.pNext = NULL;
    surface.flags = 0;
    surface.pLayer = vulkanDSL->caMetalLayer;

    err = vkCreateMetalSurfaceEXT(vulkanDSL->inst, &surface, NULL, &vulkanDSL->surface);
#endif
    assert(!err);
}

static VkSurfaceFormatKHR pick_surface_format(const VkSurfaceFormatKHR *surfaceFormats, uint32_t count) {
    // Prefer non-SRGB formats...
    for (uint32_t i = 0; i < count; i++) {
        const VkFormat format = surfaceFormats[i].format;

        if (format == VK_FORMAT_R8G8B8A8_UNORM || format == VK_FORMAT_B8G8R8A8_UNORM ||
            format == VK_FORMAT_A2B10G10R10_UNORM_PACK32 || format == VK_FORMAT_A2R10G10B10_UNORM_PACK32 ||
            format == VK_FORMAT_R16G16B16A16_SFLOAT) {
            return surfaceFormats[i];
        }
    }

    printf("Can't find our preferred formats... Falling back to first exposed format. Rendering may be incorrect.\n");

    assert(count >= 1);
    return surfaceFormats[0];
}

// use more queues if the work is independent
// https://stackoverflow.com/questions/37575012/should-i-try-to-use-as-many-queues-as-possible
static void demo_init_vk_swapchain(struct VulkanDSL *vulkanDSL) {
    VkResult U_ASSERT_ONLY err;

    demo_create_surface(vulkanDSL);

    // Iterate over each queue to learn whether it supports presenting:
    VkBool32 *supportsPresent = (VkBool32 *)malloc(vulkanDSL->queue_family_count * sizeof(VkBool32));
    for (uint32_t i = 0; i < vulkanDSL->queue_family_count; i++) {
        vulkanDSL->fpGetPhysicalDeviceSurfaceSupportKHR(vulkanDSL->gpu, i, vulkanDSL->surface, &supportsPresent[i]);
    }

    // Search for a graphics and a present queue in the array of queue
    // families, try to find one that supports both
    uint32_t graphicsQueueFamilyIndex = UINT32_MAX;
    uint32_t presentQueueFamilyIndex = UINT32_MAX;
    for (uint32_t i = 0; i < vulkanDSL->queue_family_count; i++) {
        //if ((vulkanDSL->queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
        if ((vulkanDSL->queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0
        && (vulkanDSL->queue_props[i].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0) {
            if (graphicsQueueFamilyIndex == UINT32_MAX) {
                graphicsQueueFamilyIndex = i;
            }

            if (supportsPresent[i] == VK_TRUE) {
                graphicsQueueFamilyIndex = i;
                presentQueueFamilyIndex = i;
            }
        }
    }

    if (presentQueueFamilyIndex == UINT32_MAX) {
        // If we didn't find a queue that supports both graphics and present, then
        // find a separate present queue.
        for (uint32_t i = 0; i < vulkanDSL->queue_family_count; ++i) {
            if (supportsPresent[i] == VK_TRUE) {
                presentQueueFamilyIndex = i;
                break;
            }
        }
    }

    // Generate error if could not find both a graphics and a present queue
    if (graphicsQueueFamilyIndex == UINT32_MAX || presentQueueFamilyIndex == UINT32_MAX) {
        ERR_EXIT("Could not find both graphics and present queues\n", "Swapchain Initialization Failure");
    }

    vulkanDSL->graphics_queue_family_index = graphicsQueueFamilyIndex;
    vulkanDSL->present_queue_family_index = presentQueueFamilyIndex;
    vulkanDSL->separate_present_queue = (vulkanDSL->graphics_queue_family_index != vulkanDSL->present_queue_family_index);
    free(supportsPresent);

    demo_create_device(vulkanDSL);

    vulkanDSL->fpCreateSwapchainKHR = vkCreateSwapchainKHR;
    vulkanDSL->fpDestroySwapchainKHR = vkDestroySwapchainKHR;
    vulkanDSL->fpGetSwapchainImagesKHR = vkGetSwapchainImagesKHR;
    vulkanDSL->fpAcquireNextImageKHR = vkAcquireNextImageKHR;
    vulkanDSL->fpQueuePresentKHR = vkQueuePresentKHR;
    //vulkanDSL->fpGetRefreshCycleDurationGOOGLE = vkGetRefreshCycleDurationGOOGLE;
    //vulkanDSL->fpGetPastPresentationTimingGOOGLE = vkGetPastPresentationTimingGOOGLE;

    /*
   GET_DEVICE_PROC_ADDR(vulkanDSL->device, CreateSwapchainKHR);
   GET_DEVICE_PROC_ADDR(vulkanDSL->device, DestroySwapchainKHR);
   GET_DEVICE_PROC_ADDR(vulkanDSL->device, GetSwapchainImagesKHR);
   GET_DEVICE_PROC_ADDR(vulkanDSL->device, AcquireNextImageKHR);
   GET_DEVICE_PROC_ADDR(vulkanDSL->device, QueuePresentKHR);
   if (vulkanDSL->VK_GOOGLE_display_timing_enabled) {
       GET_DEVICE_PROC_ADDR(vulkanDSL->device, GetRefreshCycleDurationGOOGLE);
       GET_DEVICE_PROC_ADDR(vulkanDSL->device, GetPastPresentationTimingGOOGLE);
   }
    */

    vkGetDeviceQueue(vulkanDSL->device, vulkanDSL->graphics_queue_family_index, 0, &vulkanDSL->graphics_queue);
    //vkGetDeviceQueue(vulkanDSL->device, vulkanDSL->graphics_queue_family_index, 1, &vulkanDSL->graphics_queue2);

    if (!vulkanDSL->separate_present_queue) {
        vulkanDSL->present_queue = vulkanDSL->graphics_queue;
    } else {
        vkGetDeviceQueue(vulkanDSL->device, vulkanDSL->present_queue_family_index, 0, &vulkanDSL->present_queue);
    }

    // Get the list of VkFormat's that are supported:
    uint32_t formatCount;
    err = vulkanDSL->fpGetPhysicalDeviceSurfaceFormatsKHR(vulkanDSL->gpu, vulkanDSL->surface, &formatCount, NULL);
    assert(!err);
    VkSurfaceFormatKHR *surfFormats = (VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
    err = vulkanDSL->fpGetPhysicalDeviceSurfaceFormatsKHR(vulkanDSL->gpu, vulkanDSL->surface, &formatCount, surfFormats);
    assert(!err);
    VkSurfaceFormatKHR surfaceFormat = pick_surface_format(surfFormats, formatCount);
    vulkanDSL->format = surfaceFormat.format;
    vulkanDSL->color_space = surfaceFormat.colorSpace;
    free(surfFormats);

    vulkanDSL->quit = false;
    vulkanDSL->curFrame = 0;

    // Create semaphores to synchronize acquiring presentable buffers before
    // rendering and waiting for drawing to be complete before presenting
    VkSemaphoreCreateInfo semaphoreCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
    };

    // Create fences that we can use to throttle if we get too far
    // ahead of the image presents
    VkFenceCreateInfo fence_ci = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .pNext = NULL, .flags = VK_FENCE_CREATE_SIGNALED_BIT};
    for (uint32_t i = 0; i < FRAME_LAG; i++) {
        err = vkCreateFence(vulkanDSL->device, &fence_ci, NULL, &vulkanDSL->fences[i]);
        assert(!err);

        err = vkCreateSemaphore(vulkanDSL->device, &semaphoreCreateInfo, NULL, &vulkanDSL->image_acquired_semaphores[i]);
        assert(!err);

        err = vkCreateSemaphore(vulkanDSL->device, &semaphoreCreateInfo, NULL, &vulkanDSL->draw_complete_semaphores[i]);
        assert(!err);

        if (vulkanDSL->separate_present_queue) {
            err = vkCreateSemaphore(vulkanDSL->device, &semaphoreCreateInfo, NULL, &vulkanDSL->image_ownership_semaphores[i]);
            assert(!err);
        }
    }
    vulkanDSL->frame_index = 0;

    // Get Memory information and properties
    vkGetPhysicalDeviceMemoryProperties(vulkanDSL->gpu, &vulkanDSL->memory_properties);
}

void demo_init_matrices(struct VulkanDSL *vulkanDSL, int width, int height) {
    //vec3 eye = {0.0f, 0, 12.0f}; // to be on the axis of rotation
    //vec3 eye = {0.0f, 4.5f, 6.5f};
    //vec3 origin = {0, 0, 0};
    //vec3 up = {0.0f, 1.0f, 1.0};
    vec3 eye = {0.0f, 3.0f, 5.0f};
    vec3 origin = {0, 0, 0};
    vec3 up = {0.0f, 1.0f, 0.0};

    // degrees per second
    vulkanDSL->spin_angle = 32.0f;
    vulkanDSL->spin_increment = 0.2f;
    vulkanDSL->pause = false;

    mat4x4_perspective(vulkanDSL->projection_matrix, (float)degreesToRadians(45.0f), (float)width/height, 0.1f, 100.0f);
    mat4x4_look_at(vulkanDSL->view_matrix, eye, origin, up);
    mat4x4_identity(vulkanDSL->model_matrix);

    vulkanDSL->projection_matrix[1][1] *= -1;  // Flip the projection matrix from GL to Vulkan orientation.
}

static void demo_init(struct VulkanDSL *vulkanDSL, int width, int height) {
    vulkanDSL->presentMode = VK_PRESENT_MODE_FIFO_KHR;
    vulkanDSL->frameCount = INT32_MAX;
    // Autodetect suitable / best GPU by default
    vulkanDSL->gpu_number = -1;
    vulkanDSL->width = width;
    vulkanDSL->height = height;
    demo_init_vk(vulkanDSL);
    demo_init_matrices(vulkanDSL, width, height);
}

// https://developer.android.com/ndk/reference/group/asset#group___asset_1ga90c459935e76acf809b9ec90d1872771
void setTextures(struct VulkanDSL *vulkanDSL ) {
    struct AssetsFetcher *assetsFetcher = &vulkanDSL->assetsFetcher;
    const char* assetsPath = assetsFetcher->assetsPath;
    tex_files = (char**) malloc((sizeof (char*)) * DEMO_TEXTURE_COUNT);
    char **tex_files_short = assetsFetcher->tex_files_short;
    for (int i = 0; i < DEMO_TEXTURE_COUNT; i++) {
#ifdef __ANDROID__
        const char *format = "%stextures/%s.png";
#else
        const char *format = "%s/%s.png";
#endif
        // +1 for the \0
        size_t allocatedSize = strlen(assetsPath) + strlen(tex_files_short[i]) + strlen(format) + 1;
        tex_files[i] = (char*) malloc((sizeof(char)) * allocatedSize);
        sprintf(tex_files[i], format, assetsPath, tex_files_short[i]);
    }
}

void vulkanDSL_main(struct VulkanDSL *vulkanDSL) {

    setTextures(vulkanDSL);

    demo_init(vulkanDSL, 10, 10);

    demo_init_vk_swapchain(vulkanDSL);

    demo_prepare(vulkanDSL);

    demo_draw(vulkanDSL, 0);
}

void VulkanDSL__setSize(struct VulkanDSL *vulkanDSL, int32_t width, int32_t height) {
    if (width == vulkanDSL->width && height == vulkanDSL->height) {
        return;
    }
    demo_init_matrices(vulkanDSL, width, height);
    demo_resize(vulkanDSL);
}

void VulkanDSL__freeResources(struct VulkanDSL *vulkanDSL) {
    AssetsFetcher__reset(&vulkanDSL->assetsFetcher);
    for (int i = 0; i < DEMO_TEXTURE_COUNT; i++) {
        free(tex_files[i]);
    }
    free(tex_files);
}
