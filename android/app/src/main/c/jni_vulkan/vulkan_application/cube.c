/*
 * Copyright (c) 2015-2019 The Khronos Group Inc.
 * Copyright (c) 2015-2019 Valve Corporation
 * Copyright (c) 2015-2019 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file exdemo_maincept in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: Chia-I Wu <olv@lunarg.com>
 * Author: Courtney Goeltzenleuchter <courtney@LunarG.com>
 * Author: Ian Elliott <ian@LunarG.com>
 * Author: Ian Elliott <ianelliott@google.com>
 * Author: Jon Ashburn <jon@lunarg.com>
 * Author: Gwan-gyeong Mun <elongbug@gmail.com>
 * Author: Tony Barbour <tony@LunarG.com>
 * Author: Bill Hollings <bill.hollings@brenwill.com>
 */

#include <stdio.h>
#include "cube.h"
// cube.c and cube2.h must be exact copies to sycn Android and iOS
#define STB_IMAGE_IMPLEMENTATION
#include "utils/stb_image.h"
#ifdef __ANDROID__
#include <android/native_activity.h>
AAssetManager *assetManager;
#endif

struct demo demo;

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
    struct demo *demo = (struct demo *)pUserData;

    if (demo->use_break) {
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
    if (!demo->suppress_popups) MessageBox(NULL, message, "Alert", MB_OK);
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
void demo_resize(struct demo *demo);
static void demo_create_surface(struct demo *demo);

static bool memory_type_from_properties(struct demo *demo, uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex) {
    // Search memtypes to find first index with those properties
    for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++) {
        if ((typeBits & 1) == 1) {
            // Type is available, does it match user properties?
            if ((demo->memory_properties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask) {
                *typeIndex = i;
                return true;
            }
        }
        typeBits >>= 1;
    }
    // No memory types matched, return failure
    return false;
}

static void demo_flush_init_cmd(struct demo *demo) {
    VkResult U_ASSERT_ONLY err;

    // This function could get called twice if the texture uses a staging buffer
    // In that case the second call should be ignored
    if (demo->cmd == VK_NULL_HANDLE) return;

    err = vkEndCommandBuffer(demo->cmd);
    assert(!err);

    VkFence fence;
    VkFenceCreateInfo fence_ci = {.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .pNext = NULL, .flags = 0};
    err = vkCreateFence(demo->device, &fence_ci, NULL, &fence);
    assert(!err);

    const VkCommandBuffer cmd_bufs[] = {demo->cmd};
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

    err = vkQueueSubmit(demo->graphics_queue, 1, &submit_info, fence);
    assert(!err);

    err = vkWaitForFences(demo->device, 1, &fence, VK_TRUE, UINT64_MAX);
    assert(!err);

    vkFreeCommandBuffers(demo->device, demo->cmd_pool, 1, cmd_bufs);
    vkDestroyFence(demo->device, fence, NULL);
    demo->cmd = VK_NULL_HANDLE;
}

static void demo_set_image_layout(struct demo *demo, VkImage image, VkImageAspectFlags aspectMask, VkImageLayout old_image_layout,
                                  VkImageLayout new_image_layout, VkAccessFlagBits srcAccessMask, VkPipelineStageFlags src_stages,
                                  VkPipelineStageFlags dest_stages) {
    assert(demo->cmd);

    VkImageMemoryBarrier image_memory_barrier = {.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
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

    vkCmdPipelineBarrier(demo->cmd, src_stages, dest_stages, 0, 0, NULL, 0, NULL, 1, pmemory_barrier);
}

static void demo_draw_build_cmd(struct demo *demo, VkCommandBuffer cmd_buf) {
    VkDebugUtilsLabelEXT label;
    memset(&label, 0, sizeof(label));
    const VkCommandBufferBeginInfo cmd_buf_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = NULL,
            .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
            .pInheritanceInfo = NULL,
    };
    const VkClearValue clear_values[2] = {
            [0] = {.color.float32 = {0.2f, 0.2f, 0.2f, 0.2f}},
            [1] = {.depthStencil = {1.0f, 0}},
    };
    const VkRenderPassBeginInfo rp_begin = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = NULL,
            .renderPass = demo->render_pass,
            .framebuffer = demo->swapchain_image_resources[demo->current_buffer].framebuffer,
            .renderArea.offset.x = 0,
            .renderArea.offset.y = 0,
            .renderArea.extent.width = demo->width,
            .renderArea.extent.height = demo->height,
            .clearValueCount = 2,
            .pClearValues = clear_values,
    };
    VkResult U_ASSERT_ONLY err;

    err = vkBeginCommandBuffer(cmd_buf, &cmd_buf_info);

    if (demo->validate) {
        // Set a name for the command buffer
        VkDebugUtilsObjectNameInfoEXT cmd_buf_name = {
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = NULL,
                .objectType = VK_OBJECT_TYPE_COMMAND_BUFFER,
                .objectHandle = (uint64_t)cmd_buf,
                .pObjectName = "CubeDrawCommandBuf",
        };
        demo->SetDebugUtilsObjectNameEXT(demo->device, &cmd_buf_name);

        label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        label.pNext = NULL;
        label.pLabelName = "DrawBegin";
        label.color[0] = 0.4f;
        label.color[1] = 0.3f;
        label.color[2] = 0.2f;
        label.color[3] = 0.1f;
        demo->CmdBeginDebugUtilsLabelEXT(cmd_buf, &label);
    }

    assert(!err);
    vkCmdBeginRenderPass(cmd_buf, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

    if (demo->validate) {
        label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        label.pNext = NULL;
        label.pLabelName = "InsideRenderPass";
        label.color[0] = 8.4f;
        label.color[1] = 7.3f;
        label.color[2] = 6.2f;
        label.color[3] = 7.1f;
        demo->CmdBeginDebugUtilsLabelEXT(cmd_buf, &label);
    }

    vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, demo->pipeline);
    vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, demo->pipeline_layout, 0, 1,
                            &demo->swapchain_image_resources[demo->current_buffer].descriptor_set, 0, NULL);
    VkViewport viewport;
    memset(&viewport, 0, sizeof(viewport));
    float viewport_dimension;
    if (demo->width < demo->height) {
        viewport_dimension = (float)demo->width;
        viewport.y = (demo->height - demo->width) / 2.0f;
    } else {
        viewport_dimension = (float)demo->height;
        viewport.x = (demo->width - demo->height) / 2.0f;
    }
    viewport.height = viewport_dimension;
    viewport.width = viewport_dimension;
    viewport.minDepth = (float)0.0f;
    viewport.maxDepth = (float)1.0f;
    vkCmdSetViewport(cmd_buf, 0, 1, &viewport);

    VkRect2D scissor;
    memset(&scissor, 0, sizeof(scissor));
    scissor.extent.width = demo->width;
    scissor.extent.height = demo->height;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    vkCmdSetScissor(cmd_buf, 0, 1, &scissor);

    if (demo->validate) {
        label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        label.pNext = NULL;
        label.pLabelName = "ActualDraw";
        label.color[0] = -0.4f;
        label.color[1] = -0.3f;
        label.color[2] = -0.2f;
        label.color[3] = -0.1f;
        demo->CmdBeginDebugUtilsLabelEXT(cmd_buf, &label);
    }

    vkCmdDraw(cmd_buf, 12 * 3, 1, 0, 0);
    if (demo->validate) {
        demo->CmdEndDebugUtilsLabelEXT(cmd_buf);
    }

    // Note that ending the renderpass changes the image's layout from
    // COLOR_ATTACHMENT_OPTIMAL to PRESENT_SRC_KHR
    vkCmdEndRenderPass(cmd_buf);
    if (demo->validate) {
        demo->CmdEndDebugUtilsLabelEXT(cmd_buf);
    }

    if (demo->separate_present_queue) {
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
                .srcQueueFamilyIndex = demo->graphics_queue_family_index,
                .dstQueueFamilyIndex = demo->present_queue_family_index,
                .image = demo->swapchain_image_resources[demo->current_buffer].image,
                .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};

        vkCmdPipelineBarrier(cmd_buf, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0,
                             NULL, 1, &image_ownership_barrier);
    }
    if (demo->validate) {
        demo->CmdEndDebugUtilsLabelEXT(cmd_buf);
    }
    err = vkEndCommandBuffer(cmd_buf);
    assert(!err);
}

void demo_build_image_ownership_cmd(struct demo *demo, int i) {
    VkResult U_ASSERT_ONLY err;

    const VkCommandBufferBeginInfo cmd_buf_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = NULL,
            .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
            .pInheritanceInfo = NULL,
    };
    err = vkBeginCommandBuffer(demo->swapchain_image_resources[i].graphics_to_present_cmd, &cmd_buf_info);
    assert(!err);

    VkImageMemoryBarrier image_ownership_barrier = {.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext = NULL,
            .srcAccessMask = 0,
            .dstAccessMask = 0,
            .oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .srcQueueFamilyIndex = demo->graphics_queue_family_index,
            .dstQueueFamilyIndex = demo->present_queue_family_index,
            .image = demo->swapchain_image_resources[i].image,
            .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};

    vkCmdPipelineBarrier(demo->swapchain_image_resources[i].graphics_to_present_cmd, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0, NULL, 1, &image_ownership_barrier);
    err = vkEndCommandBuffer(demo->swapchain_image_resources[i].graphics_to_present_cmd);
    assert(!err);
}

void demo_update_data_buffer(struct demo *demo, double elapsedTimeS) {
    mat4x4 MVP, Model, VP;
    int matrixSize = sizeof(MVP);

    mat4x4_mul(VP, demo->projection_matrix, demo->view_matrix);

    // Rotate around the Y axis
    mat4x4_dup(Model, demo->model_matrix);
    // degrees per second
    double movedAngle = demo->spin_angle * elapsedTimeS;
    //__android_log_print(ANDROID_LOG_INFO, "LOG", "%lf - &lf - %lf", demo->spin_angle, elapsedTimeS, movedAngle);
    //NSLog(@"%lf", movedAngle);
    mat4x4_rotate_Y(demo->model_matrix, Model, (float)degreesToRadians(movedAngle));
    mat4x4_orthonormalize(demo->model_matrix, demo->model_matrix);
    mat4x4_mul(MVP, VP, demo->model_matrix);

    memcpy(demo->swapchain_image_resources[demo->current_buffer].uniform_memory_ptr, (const void *)&MVP[0][0], matrixSize);
}

void DemoUpdateTargetIPD(struct demo *demo) {
    // Look at what happened to previous presents, and make appropriate
    // adjustments in timing:
    VkResult U_ASSERT_ONLY err;
    VkPastPresentationTimingGOOGLE *past = NULL;
    uint32_t count = 0;

    err = demo->fpGetPastPresentationTimingGOOGLE(demo->device, demo->swapchain, &count, NULL);
    assert(!err);
    if (count) {
        past = (VkPastPresentationTimingGOOGLE *)malloc(sizeof(VkPastPresentationTimingGOOGLE) * count);
        assert(past);
        err = demo->fpGetPastPresentationTimingGOOGLE(demo->device, demo->swapchain, &count, past);
        assert(!err);

        bool early = false;
        bool late = false;
        bool calibrate_next = false;
        for (uint32_t i = 0; i < count; i++) {
            if (!demo->syncd_with_actual_presents) {
                // This is the first time that we've received an
                // actualPresentTime for this swapchain.  In order to not
                // perceive these early frames as "late", we need to sync-up
                // our future desiredPresentTime's with the
                // actualPresentTime(s) that we're receiving now.
                calibrate_next = true;

                // So that we don't suspect any pending presents as late,
                // record them all as suspected-late presents:
                demo->last_late_id = demo->next_present_id - 1;
                demo->last_early_id = 0;
                demo->syncd_with_actual_presents = true;
                break;
            } else if (CanPresentEarlier(past[i].earliestPresentTime, past[i].actualPresentTime, past[i].presentMargin,
                                         demo->refresh_duration)) {
                // This image could have been presented earlier.  We don't want
                // to decrease the target_IPD until we've seen early presents
                // for at least two seconds.
                if (demo->last_early_id == past[i].presentID) {
                    // We've now seen two seconds worth of early presents.
                    // Flag it as such, and reset the counter:
                    early = true;
                    demo->last_early_id = 0;
                } else if (demo->last_early_id == 0) {
                    // This is the first early present we've seen.
                    // Calculate the presentID for two seconds from now.
                    uint64_t lastEarlyTime = past[i].actualPresentTime + (2 * BILLION);
                    uint32_t howManyPresents = (uint32_t)((lastEarlyTime - past[i].actualPresentTime) / demo->target_IPD);
                    demo->last_early_id = past[i].presentID + howManyPresents;
                } else {
                    // We are in the midst of a set of early images,
                    // and so we won't do anything.
                }
                late = false;
                demo->last_late_id = 0;
            } else if (ActualTimeLate(past[i].desiredPresentTime, past[i].actualPresentTime, demo->refresh_duration)) {
                // This image was presented after its desired time.  Since
                // there's a delay between calling vkQueuePresentKHR and when
                // we get the timing data, several presents may have been late.
                // Thus, we need to threat all of the outstanding presents as
                // being likely late, so that we only increase the target_IPD
                // once for all of those presents.
                if ((demo->last_late_id == 0) || (demo->last_late_id < past[i].presentID)) {
                    late = true;
                    // Record the last suspected-late present:
                    demo->last_late_id = demo->next_present_id - 1;
                } else {
                    // We are in the midst of a set of likely-late images,
                    // and so we won't do anything.
                }
                early = false;
                demo->last_early_id = 0;
            } else {
                // Since this image was not presented early or late, reset
                // any sets of early or late presentIDs:
                early = false;
                late = false;
                calibrate_next = true;
                demo->last_early_id = 0;
                demo->last_late_id = 0;
            }
        }

        if (early) {
            // Since we've seen at least two-seconds worth of presnts that
            // could have occured earlier than desired, let's decrease the
            // target_IPD (i.e. increase the frame rate):
            //
            // TODO(ianelliott): Try to calculate a better target_IPD based
            // on the most recently-seen present (this is overly-simplistic).
            demo->refresh_duration_multiplier--;
            if (demo->refresh_duration_multiplier == 0) {
                // This should never happen, but in case it does, don't
                // try to go faster.
                demo->refresh_duration_multiplier = 1;
            }
            demo->target_IPD = demo->refresh_duration * demo->refresh_duration_multiplier;
        }
        if (late) {
            // Since we found a new instance of a late present, we want to
            // increase the target_IPD (i.e. decrease the frame rate):
            //
            // TODO(ianelliott): Try to calculate a better target_IPD based
            // on the most recently-seen present (this is overly-simplistic).
            demo->refresh_duration_multiplier++;
            demo->target_IPD = demo->refresh_duration * demo->refresh_duration_multiplier;
        }

        if (calibrate_next) {
            int64_t multiple = demo->next_present_id - past[count - 1].presentID;
            demo->prev_desired_present_time = (past[count - 1].actualPresentTime + (multiple * demo->target_IPD));
        }
        free(past);
    }
}

void demo_draw(struct demo *demo, double elapsedTimeS) {
    VkResult U_ASSERT_ONLY err;

    // Ensure no more than FRAME_LAG renderings are outstanding
    vkWaitForFences(demo->device, 1, &demo->fences[demo->frame_index], VK_TRUE, UINT64_MAX);
    vkResetFences(demo->device, 1, &demo->fences[demo->frame_index]);

    do {
        // Get the index of the next available swapchain image:
        err =
                demo->fpAcquireNextImageKHR(demo->device, demo->swapchain, UINT64_MAX,
                                            demo->image_acquired_semaphores[demo->frame_index], VK_NULL_HANDLE, &demo->current_buffer);

        if (err == VK_ERROR_OUT_OF_DATE_KHR) {
            // demo->swapchain is out of date (e.g. the window was resized) and
            // must be recreated:
            demo_resize(demo);
        } else if (err == VK_SUBOPTIMAL_KHR) {
            // demo->swapchain is not as optimal as it could be, but the platform's
            // presentation engine will still present the image correctly.
            break;
        } else if (err == VK_ERROR_SURFACE_LOST_KHR) {
            vkDestroySurfaceKHR(demo->inst, demo->surface, NULL);
            demo_create_surface(demo);
            demo_resize(demo);
        } else {
            assert(!err);
        }
    } while (err != VK_SUCCESS);

    demo_update_data_buffer(demo, elapsedTimeS);

    if (demo->VK_GOOGLE_display_timing_enabled) {
        // Look at what happened to previous presents, and make appropriate
        // adjustments in timing:
        DemoUpdateTargetIPD(demo);

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
    submit_info.pWaitSemaphores = &demo->image_acquired_semaphores[demo->frame_index];
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &demo->swapchain_image_resources[demo->current_buffer].cmd;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &demo->draw_complete_semaphores[demo->frame_index];
    err = vkQueueSubmit(demo->graphics_queue, 1, &submit_info, demo->fences[demo->frame_index]);
    assert(!err);

    if (demo->separate_present_queue) {
        // If we are using separate queues, change image ownership to the
        // present queue before presenting, waiting for the draw complete
        // semaphore and signalling the ownership released semaphore when finished
        VkFence nullFence = VK_NULL_HANDLE;
        pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &demo->draw_complete_semaphores[demo->frame_index];
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &demo->swapchain_image_resources[demo->current_buffer].graphics_to_present_cmd;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &demo->image_ownership_semaphores[demo->frame_index];
        err = vkQueueSubmit(demo->present_queue, 1, &submit_info, nullFence);
        assert(!err);
    }

    // If we are using separate queues we have to wait for image ownership,
    // otherwise wait for draw complete
    VkPresentInfoKHR present = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = NULL,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = (demo->separate_present_queue) ? &demo->image_ownership_semaphores[demo->frame_index]
                                                              : &demo->draw_complete_semaphores[demo->frame_index],
            .swapchainCount = 1,
            .pSwapchains = &demo->swapchain,
            .pImageIndices = &demo->current_buffer,
    };

    VkRectLayerKHR rect;
    VkPresentRegionKHR region;
    VkPresentRegionsKHR regions;
    if (demo->VK_KHR_incremental_present_enabled) {
        // If using VK_KHR_incremental_present, we provide a hint of the region
        // that contains changed content relative to the previously-presented
        // image.  The implementation can use this hint in order to save
        // work/power (by only copying the region in the hint).  The
        // implementation is free to ignore the hint though, and so we must
        // ensure that the entire image has the correctly-drawn content.
        uint32_t eighthOfWidth = demo->width / 8;
        uint32_t eighthOfHeight = demo->height / 8;

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

    if (demo->VK_GOOGLE_display_timing_enabled) {
        VkPresentTimeGOOGLE ptime;

        if (demo->prev_desired_present_time == 0) {
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
                ptime.desiredPresentTime = curtime + (demo->target_IPD >> 1);
            }
        } else {
            ptime.desiredPresentTime = (demo->prev_desired_present_time + demo->target_IPD);
        }

        //ptime.desiredPresentTime = (uint64_t)(elapsedTimeS * 1000000000);
        ptime.presentID = demo->next_present_id++;
        demo->prev_desired_present_time = ptime.desiredPresentTime;

        VkPresentTimesInfoGOOGLE present_time = {
                .sType = VK_STRUCTURE_TYPE_PRESENT_TIMES_INFO_GOOGLE,
                .pNext = present.pNext,
                .swapchainCount = present.swapchainCount,
                .pTimes = &ptime,
        };
        if (demo->VK_GOOGLE_display_timing_enabled) {
            present.pNext = &present_time;
        }
    }

    err = demo->fpQueuePresentKHR(demo->present_queue, &present);
    demo->frame_index += 1;
    demo->frame_index %= FRAME_LAG;

    if (err == VK_ERROR_OUT_OF_DATE_KHR) {
        // demo->swapchain is out of date (e.g. the window was resized) and
        // must be recreated:
        demo_resize(demo);
    } else if (err == VK_SUBOPTIMAL_KHR) {
        // SUBOPTIMAL could be due to a resize
        VkSurfaceCapabilitiesKHR surfCapabilities;
        err = demo->fpGetPhysicalDeviceSurfaceCapabilitiesKHR(demo->gpu, demo->surface, &surfCapabilities);
        assert(!err);
        if (surfCapabilities.currentExtent.width != (uint32_t)demo->width ||
            surfCapabilities.currentExtent.height != (uint32_t)demo->height) {
            demo_resize(demo);
        }
    } else if (err == VK_ERROR_SURFACE_LOST_KHR) {
        vkDestroySurfaceKHR(demo->inst, demo->surface, NULL);
        demo_create_surface(demo);
        demo_resize(demo);
    } else {
        assert(!err);
    }
}

static void demo_prepare_buffers(struct demo *demo) {
    VkResult U_ASSERT_ONLY err;
    VkSwapchainKHR oldSwapchain = demo->swapchain;

    // Check the surface capabilities and formats
    VkSurfaceCapabilitiesKHR surfCapabilities;
    err = demo->fpGetPhysicalDeviceSurfaceCapabilitiesKHR(demo->gpu, demo->surface, &surfCapabilities);
    assert(!err);

    uint32_t presentModeCount;
    err = demo->fpGetPhysicalDeviceSurfacePresentModesKHR(demo->gpu, demo->surface, &presentModeCount, NULL);
    assert(!err);
    VkPresentModeKHR *presentModes = (VkPresentModeKHR *)malloc(presentModeCount * sizeof(VkPresentModeKHR));
    assert(presentModes);
    err = demo->fpGetPhysicalDeviceSurfacePresentModesKHR(demo->gpu, demo->surface, &presentModeCount, presentModes);
    assert(!err);

    VkExtent2D swapchainExtent;
    // width and height are either both 0xFFFFFFFF, or both not 0xFFFFFFFF.
    if (surfCapabilities.currentExtent.width == 0xFFFFFFFF) {
        // If the surface size is undefined, the size is set to the size
        // of the images requested, which must fit within the minimum and
        // maximum values.
        swapchainExtent.width = demo->width;
        swapchainExtent.height = demo->height;

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
        demo->width = surfCapabilities.currentExtent.width;
        demo->height = surfCapabilities.currentExtent.height;
    }

    if (surfCapabilities.maxImageExtent.width == 0 || surfCapabilities.maxImageExtent.height == 0) {
        demo->is_minimized = true;
        return;
    } else {
        demo->is_minimized = false;
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

    if (demo->presentMode != swapchainPresentMode) {
        for (size_t i = 0; i < presentModeCount; ++i) {
            if (presentModes[i] == demo->presentMode) {
                swapchainPresentMode = demo->presentMode;
                break;
            }
        }
    }
    if (swapchainPresentMode != demo->presentMode) {
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
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
    };
    for (uint32_t i = 0; i < ARRAY_SIZE(compositeAlphaFlags); i++) {
        if (surfCapabilities.supportedCompositeAlpha & compositeAlphaFlags[i]) {
            compositeAlpha = compositeAlphaFlags[i];
            break;
        }
    }

    // we do not want opaque
    compositeAlpha = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;

    VkSwapchainCreateInfoKHR swapchain_ci = {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext = NULL,
            .surface = demo->surface,
            .minImageCount = desiredNumOfSwapchainImages,
            .imageFormat = demo->format,
            .imageColorSpace = demo->color_space,
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
    err = demo->fpCreateSwapchainKHR(demo->device, &swapchain_ci, NULL, &demo->swapchain);
    assert(!err);

    // If we just re-created an existing swapchain, we should destroy the old
    // swapchain at this point.
    // Note: destroying the swapchain also cleans up all its associated
    // presentable images once the platform is done with them.
    if (oldSwapchain != VK_NULL_HANDLE) {
        demo->fpDestroySwapchainKHR(demo->device, oldSwapchain, NULL);
    }

    err = demo->fpGetSwapchainImagesKHR(demo->device, demo->swapchain, &demo->swapchainImageCount, NULL);
    assert(!err);

    VkImage *swapchainImages = (VkImage *)malloc(demo->swapchainImageCount * sizeof(VkImage));
    assert(swapchainImages);
    err = demo->fpGetSwapchainImagesKHR(demo->device, demo->swapchain, &demo->swapchainImageCount, swapchainImages);
    assert(!err);

    demo->swapchain_image_resources =
            (SwapchainImageResources *)malloc(sizeof(SwapchainImageResources) * demo->swapchainImageCount);
    assert(demo->swapchain_image_resources);

    for (i = 0; i < demo->swapchainImageCount; i++) {
        VkImageViewCreateInfo color_image_view = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .pNext = NULL,
                .format = demo->format,
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

        demo->swapchain_image_resources[i].image = swapchainImages[i];

        color_image_view.image = demo->swapchain_image_resources[i].image;

        err = vkCreateImageView(demo->device, &color_image_view, NULL, &demo->swapchain_image_resources[i].view);
        assert(!err);
    }

    if (demo->VK_GOOGLE_display_timing_enabled) {
        VkRefreshCycleDurationGOOGLE rc_dur;
        err = demo->fpGetRefreshCycleDurationGOOGLE(demo->device, demo->swapchain, &rc_dur);
        assert(!err);
        demo->refresh_duration = rc_dur.refreshDuration;

        demo->syncd_with_actual_presents = false;
        // Initially target 1X the refresh duration:
        demo->target_IPD = demo->refresh_duration;
        demo->refresh_duration_multiplier = 1;
        demo->prev_desired_present_time = 0;
        demo->next_present_id = 1;
    }

    if (NULL != swapchainImages) {
        free(swapchainImages);
    }

    if (NULL != presentModes) {
        free(presentModes);
    }
}

static void demo_prepare_depth(struct demo *demo) {
    const VkFormat depth_format = VK_FORMAT_D16_UNORM;
    const VkImageCreateInfo image = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = NULL,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = depth_format,
            .extent = {demo->width, demo->height, 1},
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
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

    demo->depth.format = depth_format;

    /* create image */
    err = vkCreateImage(demo->device, &image, NULL, &demo->depth.image);
    assert(!err);

    vkGetImageMemoryRequirements(demo->device, demo->depth.image, &mem_reqs);
    assert(!err);

    demo->depth.mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    demo->depth.mem_alloc.pNext = NULL;
    demo->depth.mem_alloc.allocationSize = mem_reqs.size;
    demo->depth.mem_alloc.memoryTypeIndex = 0;

    pass = memory_type_from_properties(demo, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                       &demo->depth.mem_alloc.memoryTypeIndex);
    assert(pass);

    /* allocate memory */
    err = vkAllocateMemory(demo->device, &demo->depth.mem_alloc, NULL, &demo->depth.mem);
    assert(!err);

    /* bind memory */
    err = vkBindImageMemory(demo->device, demo->depth.image, demo->depth.mem, 0);
    assert(!err);

    /* create image view */
    view.image = demo->depth.image;
    err = vkCreateImageView(demo->device, &view, NULL, &demo->depth.view);
    assert(!err);
}

bool loadTexture(const char *filename, uint8_t *rgba_data, VkSubresourceLayout *layout, int32_t *width, int32_t *height) {
    int texChannels;
#ifdef __ANDROID__
    AAsset* asset = AAssetManager_open(
      assetManager,
      filename,
      AASSET_MODE_BUFFER
    );
    off64_t length1 = AAsset_getLength(asset);
    void* rawData = AAsset_getBuffer(asset);
    FILE* file = fmemopen(rawData, length1, "rb");
    stbi_uc* pixels = stbi_load_from_file(file, width, height, &texChannels, STBI_rgb_alpha);
    fclose(file);
#else
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
    AAsset_close(asset);
    return true;
}


static void demo_prepare_texture_buffer(struct demo *demo, const char *filename, struct texture_object *tex_obj) {
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

    err = vkCreateBuffer(demo->device, &buffer_create_info, NULL, &tex_obj->buffer);
    assert(!err);

    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(demo->device, tex_obj->buffer, &mem_reqs);

    tex_obj->mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    tex_obj->mem_alloc.pNext = NULL;
    tex_obj->mem_alloc.allocationSize = mem_reqs.size;
    tex_obj->mem_alloc.memoryTypeIndex = 0;

    VkFlags requirements = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    pass = memory_type_from_properties(demo, mem_reqs.memoryTypeBits, requirements, &tex_obj->mem_alloc.memoryTypeIndex);
    assert(pass);

    err = vkAllocateMemory(demo->device, &tex_obj->mem_alloc, NULL, &(tex_obj->mem));
    assert(!err);

    /* bind memory */
    err = vkBindBufferMemory(demo->device, tex_obj->buffer, tex_obj->mem, 0);
    assert(!err);

    VkSubresourceLayout layout;
    memset(&layout, 0, sizeof(layout));
    layout.rowPitch = tex_width * 4;

    void *data;
    err = vkMapMemory(demo->device, tex_obj->mem, 0, tex_obj->mem_alloc.allocationSize, 0, &data);
    assert(!err);

    if (!loadTexture(filename, data, &layout, &tex_width, &tex_height)) {
        fprintf(stderr, "Error loading texture: %s\n", filename);
    }

    vkUnmapMemory(demo->device, tex_obj->mem);
}

static void demo_prepare_texture_image(struct demo *demo, const char *filename, struct texture_object *tex_obj,
                                       VkImageTiling tiling, VkImageUsageFlags usage, VkFlags required_props) {
    const VkFormat tex_format = VK_FORMAT_R8G8B8A8_UNORM;
    int32_t tex_width;
    int32_t tex_height;
    VkResult U_ASSERT_ONLY err;
    bool U_ASSERT_ONLY pass;
    int texChannels;
#ifdef __ANDROID__
    if (!loadTexture(filename, NULL, NULL, &tex_width, &tex_height)) {
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
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = tiling,
            .usage = usage,
            .flags = 0,
            .initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED,
    };

    VkMemoryRequirements mem_reqs;

    err = vkCreateImage(demo->device, &image_create_info, NULL, &tex_obj->image);
    assert(!err);

    vkGetImageMemoryRequirements(demo->device, tex_obj->image, &mem_reqs);

    tex_obj->mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    tex_obj->mem_alloc.pNext = NULL;
    tex_obj->mem_alloc.allocationSize = mem_reqs.size;
    tex_obj->mem_alloc.memoryTypeIndex = 0;

    pass = memory_type_from_properties(demo, mem_reqs.memoryTypeBits, required_props, &tex_obj->mem_alloc.memoryTypeIndex);
    assert(pass);

    /* allocate memory */
    err = vkAllocateMemory(demo->device, &tex_obj->mem_alloc, NULL, &(tex_obj->mem));
    assert(!err);

    /* bind memory */
    err = vkBindImageMemory(demo->device, tex_obj->image, tex_obj->mem, 0);
    assert(!err);

    if (required_props & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        const VkImageSubresource subres = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .arrayLayer = 0,
        };
        VkSubresourceLayout layout;
        void *data;

        vkGetImageSubresourceLayout(demo->device, tex_obj->image, &subres, &layout);

        err = vkMapMemory(demo->device, tex_obj->mem, 0, tex_obj->mem_alloc.allocationSize, 0, &data);
        assert(!err);

        int texChannels;
        if (!loadTexture(filename, data, &layout, &tex_width, &tex_height)) {
            fprintf(stderr, "Error loading texture: %s\n", filename);
        }

        vkUnmapMemory(demo->device, tex_obj->mem);
    }

    tex_obj->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

static void demo_destroy_texture(struct demo *demo, struct texture_object *tex_objs) {
    /* clean up staging resources */
    vkFreeMemory(demo->device, tex_objs->mem, NULL);
    if (tex_objs->image) vkDestroyImage(demo->device, tex_objs->image, NULL);
    if (tex_objs->buffer) vkDestroyBuffer(demo->device, tex_objs->buffer, NULL);
}

static void demo_prepare_textures(struct demo *demo) {
    const VkFormat tex_format = VK_FORMAT_R8G8B8A8_UNORM;
    VkFormatProperties props;
    uint32_t i;

    vkGetPhysicalDeviceFormatProperties(demo->gpu, tex_format, &props);
    for (i = 0; i < DEMO_TEXTURE_COUNT; i++) {
        VkResult U_ASSERT_ONLY err;
        if ((props.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) && !demo->use_staging_buffer) {
            //NSLog(@"--> Linear tiling is supported");
            /* Device can texture using linear textures */
            demo_prepare_texture_image(demo, tex_files[i], &demo->textures[i], VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_SAMPLED_BIT,
                                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            // Nothing in the pipeline needs to be complete to start, and don't allow fragment
            // shader to run until layout transition completes
            demo_set_image_layout(demo, demo->textures[i].image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PREINITIALIZED,
                                  demo->textures[i].imageLayout, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
            demo->staging_texture.image = 0;
        } else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) {
            /* Must use staging buffer to copy linear texture to optimized */

            memset(&demo->staging_texture, 0, sizeof(demo->staging_texture));
            demo_prepare_texture_buffer(demo, tex_files[i], &demo->staging_texture);

            demo_prepare_texture_image(demo, tex_files[i], &demo->textures[i], VK_IMAGE_TILING_OPTIMAL,
                                       (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            demo_set_image_layout(demo, demo->textures[i].image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PREINITIALIZED,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                  VK_PIPELINE_STAGE_TRANSFER_BIT);

            VkBufferImageCopy copy_region = {
                    .bufferOffset = 0,
                    .bufferRowLength = demo->staging_texture.tex_width,
                    .bufferImageHeight = demo->staging_texture.tex_height,
                    .imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
                    .imageOffset = {0, 0, 0},
                    .imageExtent = {demo->staging_texture.tex_width, demo->staging_texture.tex_height, 1},
            };

            vkCmdCopyBufferToImage(demo->cmd, demo->staging_texture.buffer, demo->textures[i].image,
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

            demo_set_image_layout(demo, demo->textures[i].image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                  demo->textures[i].imageLayout, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

        } else {
            /* Can't support VK_FORMAT_R8G8B8A8_UNORM !? */
            assert(!"No support for R8G8B8A8_UNORM as texture image format");
        }

        const VkSamplerCreateInfo sampler = {
                .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                .pNext = NULL,
                .magFilter = VK_FILTER_NEAREST,
                .minFilter = VK_FILTER_NEAREST,
                .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
                .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                .mipLodBias = 0.0f,
                .anisotropyEnable = VK_FALSE,
                .maxAnisotropy = 1,
                .compareOp = VK_COMPARE_OP_NEVER,
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
        err = vkCreateSampler(demo->device, &sampler, NULL, &demo->textures[i].sampler);
        assert(!err);

        /* create image view */
        view.image = demo->textures[i].image;
        err = vkCreateImageView(demo->device, &view, NULL, &demo->textures[i].view);
        assert(!err);
    }
}

void demo_prepare_cube_data_buffers(struct demo *demo) {
    VkBufferCreateInfo buf_info;
    VkMemoryRequirements mem_reqs;
    VkMemoryAllocateInfo mem_alloc;
    mat4x4 MVP, VP;
    VkResult U_ASSERT_ONLY err;
    bool U_ASSERT_ONLY pass;
    struct vktexcube_vs_uniform data;

    mat4x4_mul(VP, demo->projection_matrix, demo->view_matrix);
    mat4x4_mul(MVP, VP, demo->model_matrix);
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

    for (unsigned int i = 0; i < demo->swapchainImageCount; i++) {
        err = vkCreateBuffer(demo->device, &buf_info, NULL, &demo->swapchain_image_resources[i].uniform_buffer);
        assert(!err);

        vkGetBufferMemoryRequirements(demo->device, demo->swapchain_image_resources[i].uniform_buffer, &mem_reqs);

        mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        mem_alloc.pNext = NULL;
        mem_alloc.allocationSize = mem_reqs.size;
        mem_alloc.memoryTypeIndex = 0;

        pass = memory_type_from_properties(demo, mem_reqs.memoryTypeBits,
                                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                           &mem_alloc.memoryTypeIndex);
        assert(pass);

        err = vkAllocateMemory(demo->device, &mem_alloc, NULL, &demo->swapchain_image_resources[i].uniform_memory);
        assert(!err);

        err = vkMapMemory(demo->device, demo->swapchain_image_resources[i].uniform_memory, 0, VK_WHOLE_SIZE, 0,
                          &demo->swapchain_image_resources[i].uniform_memory_ptr);
        assert(!err);

        memcpy(demo->swapchain_image_resources[i].uniform_memory_ptr, &data, sizeof data);

        err = vkBindBufferMemory(demo->device, demo->swapchain_image_resources[i].uniform_buffer,
                                 demo->swapchain_image_resources[i].uniform_memory, 0);
        assert(!err);
    }
}

static void demo_prepare_descriptor_layout(struct demo *demo) {
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

    err = vkCreateDescriptorSetLayout(demo->device, &descriptor_layout, NULL, &demo->desc_layout);
    assert(!err);

    const VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = NULL,
            .setLayoutCount = 1,
            .pSetLayouts = &demo->desc_layout,
    };

    err = vkCreatePipelineLayout(demo->device, &pPipelineLayoutCreateInfo, NULL, &demo->pipeline_layout);
    assert(!err);
}

static void demo_prepare_render_pass(struct demo *demo) {
    // The initial layout for the color and depth attachments will be LAYOUT_UNDEFINED
    // because at the start of the renderpass, we don't care about their contents.
    // At the start of the subpass, the color attachment's layout will be transitioned
    // to LAYOUT_COLOR_ATTACHMENT_OPTIMAL and the depth stencil attachment's layout
    // will be transitioned to LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL.  At the end of
    // the renderpass, the color attachment's layout will be transitioned to
    // LAYOUT_PRESENT_SRC_KHR to be ready to present.  This is all done as part of
    // the renderpass, no barriers are necessary.
    const VkAttachmentDescription attachments[2] = {
            [0] =
                    {
                            .format = demo->format,
                            .flags = 0,
                            .samples = VK_SAMPLE_COUNT_1_BIT,
                            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                    },
            [1] =
                    {
                            .format = demo->depth.format,
                            .flags = 0,
                            .samples = VK_SAMPLE_COUNT_1_BIT,
                            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
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
    const VkSubpassDescription subpass = {
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .flags = 0,
            .inputAttachmentCount = 0,
            .pInputAttachments = NULL,
            .colorAttachmentCount = 1,
            .pColorAttachments = &color_reference,
            .pResolveAttachments = NULL,
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
            .attachmentCount = 2,
            .pAttachments = attachments,
            .subpassCount = 1,
            .pSubpasses = &subpass,
            .dependencyCount = 2,
            .pDependencies = attachmentDependencies,
    };
    VkResult U_ASSERT_ONLY err;

    err = vkCreateRenderPass(demo->device, &rp_info, NULL, &demo->render_pass);
    assert(!err);
}

static VkShaderModule demo_prepare_shader_module(struct demo *demo, const uint32_t *code, size_t size) {
    VkShaderModule module;
    VkShaderModuleCreateInfo moduleCreateInfo;
    VkResult U_ASSERT_ONLY err;

    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.pNext = NULL;
    moduleCreateInfo.flags = 0;
    moduleCreateInfo.codeSize = size;
    moduleCreateInfo.pCode = code;

    err = vkCreateShaderModule(demo->device, &moduleCreateInfo, NULL, &module);
    assert(!err);

    return module;
}

static void demo_prepare_vs(struct demo *demo) {
    const uint32_t vs_code[] = {
#include "cube.vert.inc"
    };
    demo->vert_shader_module = demo_prepare_shader_module(demo, vs_code, sizeof(vs_code));
}

static void demo_prepare_fs(struct demo *demo) {
    const uint32_t fs_code[] = {
#include "cube.frag.inc"
    };
    demo->frag_shader_module = demo_prepare_shader_module(demo, fs_code, sizeof(fs_code));
}

static void demo_prepare_pipeline(struct demo *demo) {
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
    pipeline.layout = demo->pipeline_layout;

    memset(&vi, 0, sizeof(vi));
    vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

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
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    demo_prepare_vs(demo);
    demo_prepare_fs(demo);

    // Two stages: vs and fs
    VkPipelineShaderStageCreateInfo shaderStages[2];
    memset(&shaderStages, 0, 2 * sizeof(VkPipelineShaderStageCreateInfo));

    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = demo->vert_shader_module;
    shaderStages[0].pName = "main";

    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = demo->frag_shader_module;
    shaderStages[1].pName = "main";

    memset(&pipelineCache, 0, sizeof(pipelineCache));
    pipelineCache.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

    err = vkCreatePipelineCache(demo->device, &pipelineCache, NULL, &demo->pipelineCache);
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
    pipeline.renderPass = demo->render_pass;
    pipeline.pDynamicState = &dynamicState;

    pipeline.renderPass = demo->render_pass;

    err = vkCreateGraphicsPipelines(demo->device, demo->pipelineCache, 1, &pipeline, NULL, &demo->pipeline);
    assert(!err);

    vkDestroyShaderModule(demo->device, demo->frag_shader_module, NULL);
    vkDestroyShaderModule(demo->device, demo->vert_shader_module, NULL);
}

static void demo_prepare_descriptor_pool(struct demo *demo) {
    const VkDescriptorPoolSize type_counts[2] = {
            [0] =
                    {
                            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                            .descriptorCount = demo->swapchainImageCount,
                    },
            [1] =
                    {
                            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                            .descriptorCount = demo->swapchainImageCount * DEMO_TEXTURE_COUNT,
                    },
    };
    const VkDescriptorPoolCreateInfo descriptor_pool = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = NULL,
            .maxSets = demo->swapchainImageCount,
            .poolSizeCount = 2,
            .pPoolSizes = type_counts,
    };
    VkResult U_ASSERT_ONLY err;

    err = vkCreateDescriptorPool(demo->device, &descriptor_pool, NULL, &demo->desc_pool);
    assert(!err);
}

static void demo_prepare_descriptor_set(struct demo *demo) {
    VkDescriptorImageInfo tex_descs[DEMO_TEXTURE_COUNT];
    VkWriteDescriptorSet writes[2];
    VkResult U_ASSERT_ONLY err;

    VkDescriptorSetAllocateInfo alloc_info = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = NULL,
            .descriptorPool = demo->desc_pool,
            .descriptorSetCount = 1,
            .pSetLayouts = &demo->desc_layout};

    VkDescriptorBufferInfo buffer_info;
    buffer_info.offset = 0;
    buffer_info.range = sizeof(struct vktexcube_vs_uniform);

    memset(&tex_descs, 0, sizeof(tex_descs));
    for (unsigned int i = 0; i < DEMO_TEXTURE_COUNT; i++) {
        tex_descs[i].sampler = demo->textures[i].sampler;
        tex_descs[i].imageView = demo->textures[i].view;
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

    for (unsigned int i = 0; i < demo->swapchainImageCount; i++) {
        err = vkAllocateDescriptorSets(demo->device, &alloc_info, &demo->swapchain_image_resources[i].descriptor_set);
        assert(!err);
        buffer_info.buffer = demo->swapchain_image_resources[i].uniform_buffer;
        writes[0].dstSet = demo->swapchain_image_resources[i].descriptor_set;
        writes[1].dstSet = demo->swapchain_image_resources[i].descriptor_set;
        vkUpdateDescriptorSets(demo->device, 2, writes, 0, NULL);
    }
}

static void demo_prepare_framebuffers(struct demo *demo) {
    VkImageView attachments[2];
    attachments[1] = demo->depth.view;

    const VkFramebufferCreateInfo fb_info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext = NULL,
            .renderPass = demo->render_pass,
            .attachmentCount = 2,
            .pAttachments = attachments,
            .width = demo->width,
            .height = demo->height,
            .layers = 1,
    };
    VkResult U_ASSERT_ONLY err;
    uint32_t i;

    for (i = 0; i < demo->swapchainImageCount; i++) {
        attachments[0] = demo->swapchain_image_resources[i].view;
        err = vkCreateFramebuffer(demo->device, &fb_info, NULL, &demo->swapchain_image_resources[i].framebuffer);
        assert(!err);
    }
}

static void demo_prepare(struct demo *demo) {
    demo_prepare_buffers(demo);

    if (demo->is_minimized) {
        demo->prepared = false;
        return;
    }

    VkResult U_ASSERT_ONLY err;
    if (demo->cmd_pool2 == VK_NULL_HANDLE) {
        const VkCommandPoolCreateInfo cmd_pool_info2 = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .pNext = NULL,
                .queueFamilyIndex = demo->graphics_queue_family_index2,
                .flags = 0,
        };
        err = vkCreateCommandPool(demo->device, &cmd_pool_info2, NULL, &demo->cmd_pool2);
        assert(!err);
    }

    const VkCommandBufferAllocateInfo cmd2 = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = NULL,
            .commandPool = demo->cmd_pool2,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
    };
    err = vkAllocateCommandBuffers(demo->device, &cmd2, &demo->cmd2);
    assert(!err);

    if (demo->cmd_pool == VK_NULL_HANDLE) {
        const VkCommandPoolCreateInfo cmd_pool_info = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .pNext = NULL,
                .queueFamilyIndex = demo->graphics_queue_family_index,
                .flags = 0,
        };
        err = vkCreateCommandPool(demo->device, &cmd_pool_info, NULL, &demo->cmd_pool);
        assert(!err);
    }

    const VkCommandBufferAllocateInfo cmd = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = NULL,
            .commandPool = demo->cmd_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
    };
    err = vkAllocateCommandBuffers(demo->device, &cmd, &demo->cmd);
    assert(!err);
    VkCommandBufferBeginInfo cmd_buf_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = NULL,
            .flags = 0,
            .pInheritanceInfo = NULL,
    };
    err = vkBeginCommandBuffer(demo->cmd, &cmd_buf_info);
    assert(!err);

    demo_prepare_depth(demo);
    demo_prepare_textures(demo);
    demo_prepare_cube_data_buffers(demo);

    demo_prepare_descriptor_layout(demo);
    demo_prepare_render_pass(demo);
    demo_prepare_pipeline(demo);

    for (uint32_t i = 0; i < demo->swapchainImageCount; i++) {
        err = vkAllocateCommandBuffers(demo->device, &cmd, &demo->swapchain_image_resources[i].cmd);
        assert(!err);
    }

    if (demo->separate_present_queue) {
        const VkCommandPoolCreateInfo present_cmd_pool_info = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .pNext = NULL,
                .queueFamilyIndex = demo->present_queue_family_index,
                .flags = 0,
        };
        err = vkCreateCommandPool(demo->device, &present_cmd_pool_info, NULL, &demo->present_cmd_pool);
        assert(!err);
        const VkCommandBufferAllocateInfo present_cmd_info = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .pNext = NULL,
                .commandPool = demo->present_cmd_pool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1,
        };
        for (uint32_t i = 0; i < demo->swapchainImageCount; i++) {
            err = vkAllocateCommandBuffers(demo->device, &present_cmd_info,
                                           &demo->swapchain_image_resources[i].graphics_to_present_cmd);
            assert(!err);
            demo_build_image_ownership_cmd(demo, i);
        }
    }

    demo_prepare_descriptor_pool(demo);
    demo_prepare_descriptor_set(demo);

    demo_prepare_framebuffers(demo);

    for (uint32_t i = 0; i < demo->swapchainImageCount; i++) {
        demo->current_buffer = i;
        demo_draw_build_cmd(demo, demo->swapchain_image_resources[i].cmd);
    }

    /*
     * Prepare functions above may generate pipeline commands
     * that need to be flushed before beginning the render loop.
     */
    demo_flush_init_cmd(demo);
    if (demo->staging_texture.buffer) {
        demo_destroy_texture(demo, &demo->staging_texture);
    }

    demo->current_buffer = 0;
    demo->prepared = true;
}

void demo_cleanup(struct demo *demo) {
    uint32_t i;

    demo->prepared = false;
    vkDeviceWaitIdle(demo->device);

    // Wait for fences from present operations
    for (i = 0; i < FRAME_LAG; i++) {
        vkWaitForFences(demo->device, 1, &demo->fences[i], VK_TRUE, UINT64_MAX);
        vkDestroyFence(demo->device, demo->fences[i], NULL);
        vkDestroySemaphore(demo->device, demo->image_acquired_semaphores[i], NULL);
        vkDestroySemaphore(demo->device, demo->draw_complete_semaphores[i], NULL);
        if (demo->separate_present_queue) {
            vkDestroySemaphore(demo->device, demo->image_ownership_semaphores[i], NULL);
        }
    }

    // If the window is currently minimized, demo_resize has already done some cleanup for us.
    if (!demo->is_minimized) {
        for (i = 0; i < demo->swapchainImageCount; i++) {
            vkDestroyFramebuffer(demo->device, demo->swapchain_image_resources[i].framebuffer, NULL);
        }
        vkDestroyDescriptorPool(demo->device, demo->desc_pool, NULL);

        vkDestroyPipeline(demo->device, demo->pipeline, NULL);
        vkDestroyPipelineCache(demo->device, demo->pipelineCache, NULL);
        vkDestroyRenderPass(demo->device, demo->render_pass, NULL);
        vkDestroyPipelineLayout(demo->device, demo->pipeline_layout, NULL);
        vkDestroyDescriptorSetLayout(demo->device, demo->desc_layout, NULL);

        for (i = 0; i < DEMO_TEXTURE_COUNT; i++) {
            vkDestroyImageView(demo->device, demo->textures[i].view, NULL);
            vkDestroyImage(demo->device, demo->textures[i].image, NULL);
            vkFreeMemory(demo->device, demo->textures[i].mem, NULL);
            vkDestroySampler(demo->device, demo->textures[i].sampler, NULL);
        }
        demo->fpDestroySwapchainKHR(demo->device, demo->swapchain, NULL);

        vkDestroyImageView(demo->device, demo->depth.view, NULL);
        vkDestroyImage(demo->device, demo->depth.image, NULL);
        vkFreeMemory(demo->device, demo->depth.mem, NULL);

        for (i = 0; i < demo->swapchainImageCount; i++) {
            vkDestroyImageView(demo->device, demo->swapchain_image_resources[i].view, NULL);
            vkFreeCommandBuffers(demo->device, demo->cmd_pool, 1, &demo->swapchain_image_resources[i].cmd);
            vkDestroyBuffer(demo->device, demo->swapchain_image_resources[i].uniform_buffer, NULL);
            vkUnmapMemory(demo->device, demo->swapchain_image_resources[i].uniform_memory);
            vkFreeMemory(demo->device, demo->swapchain_image_resources[i].uniform_memory, NULL);
        }
        free(demo->swapchain_image_resources);
        free(demo->queue_props);
        vkDestroyCommandPool(demo->device, demo->cmd_pool, NULL);

        if (demo->separate_present_queue) {
            vkDestroyCommandPool(demo->device, demo->present_cmd_pool, NULL);
        }
    }
    vkDeviceWaitIdle(demo->device);
    vkDestroyDevice(demo->device, NULL);
    if (demo->validate) {
        demo->DestroyDebugUtilsMessengerEXT(demo->inst, demo->dbg_messenger, NULL);
    }
    vkDestroySurfaceKHR(demo->inst, demo->surface, NULL);
    vkDestroyInstance(demo->inst, NULL);
}

void demo_resize(struct demo *demo) {
    uint32_t i;

    // Don't react to resize until after first initialization.
    if (!demo->prepared) {
        if (demo->is_minimized) {
            demo_prepare(demo);
        }
        return;
    }
    // In order to properly resize the window, we must re-create the swapchain
    // AND redo the command buffers, etc.
    //
    // First, perform part of the demo_cleanup() function:
    demo->prepared = false;
    vkDeviceWaitIdle(demo->device);

    for (i = 0; i < demo->swapchainImageCount; i++) {
        vkDestroyFramebuffer(demo->device, demo->swapchain_image_resources[i].framebuffer, NULL);
    }
    vkDestroyDescriptorPool(demo->device, demo->desc_pool, NULL);

    vkDestroyPipeline(demo->device, demo->pipeline, NULL);
    vkDestroyPipelineCache(demo->device, demo->pipelineCache, NULL);
    vkDestroyRenderPass(demo->device, demo->render_pass, NULL);
    vkDestroyPipelineLayout(demo->device, demo->pipeline_layout, NULL);
    vkDestroyDescriptorSetLayout(demo->device, demo->desc_layout, NULL);

    for (i = 0; i < DEMO_TEXTURE_COUNT; i++) {
        vkDestroyImageView(demo->device, demo->textures[i].view, NULL);
        vkDestroyImage(demo->device, demo->textures[i].image, NULL);
        vkFreeMemory(demo->device, demo->textures[i].mem, NULL);
        vkDestroySampler(demo->device, demo->textures[i].sampler, NULL);
    }

    vkDestroyImageView(demo->device, demo->depth.view, NULL);
    vkDestroyImage(demo->device, demo->depth.image, NULL);
    vkFreeMemory(demo->device, demo->depth.mem, NULL);

    for (i = 0; i < demo->swapchainImageCount; i++) {
        vkDestroyImageView(demo->device, demo->swapchain_image_resources[i].view, NULL);
        vkFreeCommandBuffers(demo->device, demo->cmd_pool, 1, &demo->swapchain_image_resources[i].cmd);
        vkDestroyBuffer(demo->device, demo->swapchain_image_resources[i].uniform_buffer, NULL);
        vkUnmapMemory(demo->device, demo->swapchain_image_resources[i].uniform_memory);
        vkFreeMemory(demo->device, demo->swapchain_image_resources[i].uniform_memory, NULL);
    }
    vkDestroyCommandPool(demo->device, demo->cmd_pool, NULL);
    demo->cmd_pool = VK_NULL_HANDLE;
    if (demo->separate_present_queue) {
        vkDestroyCommandPool(demo->device, demo->present_cmd_pool, NULL);
    }
    free(demo->swapchain_image_resources);

    // Second, re-perform the demo_prepare() function, which will re-create the
    // swapchain:
    demo_prepare(demo);
}

// On MS-Windows, make this a global, so it's available to WndProc()
struct demo demo;

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
static void demo_run(struct demo *demo) {
    if (!demo->prepared) return;

    demo_draw(demo, 0);
    demo->curFrame++;
}
#elif defined(VK_USE_PLATFORM_METAL_EXT)
static void demo_run(struct demo *demo) {
    //demo_draw(demo, 0);
    demo->curFrame++;
    if (demo->frameCount != INT32_MAX && demo->curFrame == demo->frameCount) {
        demo->quit = TRUE;
    }
}
#endif

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
static void demo_init_vk(struct demo *demo) {
    VkResult err;
    uint32_t instance_extension_count = 0;
    uint32_t instance_layer_count = 0;
    char *instance_validation_layers[] = {"VK_LAYER_KHRONOS_validation"};
    demo->enabled_extension_count = 0;
    demo->enabled_layer_count = 0;
    demo->is_minimized = false;
    demo->cmd_pool = VK_NULL_HANDLE;

    // Look for validation layers
    VkBool32 validation_found = 0;
    if (demo->validate) {
        err = vkEnumerateInstanceLayerProperties(&instance_layer_count, NULL);
        assert(!err);

        if (instance_layer_count > 0) {
            VkLayerProperties *instance_layers = malloc(sizeof(VkLayerProperties) * instance_layer_count);
            err = vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layers);
            assert(!err);

            validation_found = demo_check_layers(ARRAY_SIZE(instance_validation_layers), instance_validation_layers,
                                                 instance_layer_count, instance_layers);
            if (validation_found) {
                demo->enabled_layer_count = ARRAY_SIZE(instance_validation_layers);
                demo->enabled_layers[0] = "VK_LAYER_KHRONOS_validation";
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
    memset(demo->extension_names, 0, sizeof(demo->extension_names));

    err = vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, NULL);
    assert(!err);

    if (instance_extension_count > 0) {
        VkExtensionProperties *instance_extensions = malloc(sizeof(VkExtensionProperties) * instance_extension_count);
        err = vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, instance_extensions);
        assert(!err);
        for (uint32_t i = 0; i < instance_extension_count; i++) {
            if (!strcmp(VK_KHR_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName)) {
                surfaceExtFound = 1;
                demo->extension_names[demo->enabled_extension_count++] = VK_KHR_SURFACE_EXTENSION_NAME;
            }
#if defined(VK_USE_PLATFORM_WIN32_KHR)
            if (!strcmp(VK_KHR_WIN32_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName)) {
                platformSurfaceExtFound = 1;
                demo->extension_names[demo->enabled_extension_count++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
            }
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
            if (!strcmp(VK_KHR_XLIB_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName)) {
                platformSurfaceExtFound = 1;
                demo->extension_names[demo->enabled_extension_count++] = VK_KHR_XLIB_SURFACE_EXTENSION_NAME;
            }
#elif defined(VK_USE_PLATFORM_XCB_KHR)
            if (!strcmp(VK_KHR_XCB_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName)) {
                platformSurfaceExtFound = 1;
                demo->extension_names[demo->enabled_extension_count++] = VK_KHR_XCB_SURFACE_EXTENSION_NAME;
            }
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
            if (!strcmp(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName)) {
                platformSurfaceExtFound = 1;
                demo->extension_names[demo->enabled_extension_count++] = VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME;
            }
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
            if (!strcmp(VK_EXT_DIRECTFB_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName)) {
                platformSurfaceExtFound = 1;
                demo->extension_names[demo->enabled_extension_count++] = VK_EXT_DIRECTFB_SURFACE_EXTENSION_NAME;
            }
#elif defined(VK_USE_PLATFORM_DISPLAY_KHR)
            if (!strcmp(VK_KHR_DISPLAY_EXTENSION_NAME, instance_extensions[i].extensionName)) {
                platformSurfaceExtFound = 1;
                demo->extension_names[demo->enabled_extension_count++] = VK_KHR_DISPLAY_EXTENSION_NAME;
            }
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
            if (!strcmp(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName)) {
                platformSurfaceExtFound = 1;
                demo->extension_names[demo->enabled_extension_count++] = VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
            }
#elif defined(VK_USE_PLATFORM_METAL_EXT)
            if (!strcmp(VK_EXT_METAL_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName)) {
                platformSurfaceExtFound = 1;
                demo->extension_names[demo->enabled_extension_count++] = VK_EXT_METAL_SURFACE_EXTENSION_NAME;
            }
#endif
            if (!strcmp(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, instance_extensions[i].extensionName)) {
                demo->extension_names[demo->enabled_extension_count++] = VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME;
            }
            if (!strcmp(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, instance_extensions[i].extensionName)) {
                if (demo->validate) {
                    demo->extension_names[demo->enabled_extension_count++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
                }
            }
            assert(demo->enabled_extension_count < 64);
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
            .apiVersion = VK_API_VERSION_1_0,
    };
    VkInstanceCreateInfo inst_info = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = NULL,
            .pApplicationInfo = &app,
            .enabledLayerCount = demo->enabled_layer_count,
            .ppEnabledLayerNames = (const char *const *)instance_validation_layers,
            .enabledExtensionCount = demo->enabled_extension_count,
            .ppEnabledExtensionNames = (const char *const *)demo->extension_names,
    };

    /*
     * This is info for a temp callback to use during CreateInstance.
     * After the instance is created, we use the instance-based
     * function to register the final callback.
     */
    VkDebugUtilsMessengerCreateInfoEXT dbg_messenger_create_info;
    if (demo->validate) {
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
        dbg_messenger_create_info.pUserData = demo;
        inst_info.pNext = &dbg_messenger_create_info;
    }

    err = vkCreateInstance(&inst_info, NULL, &demo->inst);
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
    volkLoadInstance(demo->inst);
#endif

    /* Make initial call to query gpu_count, then second call for gpu info */
    uint32_t gpu_count = 0;
    err = vkEnumeratePhysicalDevices(demo->inst, &gpu_count, NULL);
    assert(!err);

    if (gpu_count <= 0) {
        ERR_EXIT(
                "vkEnumeratePhysicalDevices reported zero accessible devices.\n\n"
                "Do you have a compatible Vulkan installable client driver (ICD) installed?\n"
                "Please look at the Getting Started guide for additional information.\n",
                "vkEnumeratePhysicalDevices Failure");
    }

    VkPhysicalDevice *physical_devices = malloc(sizeof(VkPhysicalDevice) * gpu_count);
    err = vkEnumeratePhysicalDevices(demo->inst, &gpu_count, physical_devices);
    assert(!err);
    if (demo->gpu_number >= 0 && !((uint32_t)demo->gpu_number < gpu_count)) {
        fprintf(stderr, "GPU %d specified is not present, GPU count = %u\n", demo->gpu_number, gpu_count);
        ERR_EXIT("Specified GPU number is not present", "User Error");
    }

#if defined(VK_USE_PLATFORM_DISPLAY_KHR)
    demo->gpu_number = find_display_gpu(demo->gpu_number, gpu_count, physical_devices);
    if (demo->gpu_number < 0) {
        printf("Cannot find any display!\n");
        fflush(stdout);
        exit(1);
    }
#else
    /* Try to auto select most suitable device */
    if (demo->gpu_number == -1) {
        uint32_t count_device_type[VK_PHYSICAL_DEVICE_TYPE_CPU + 1];
        memset(count_device_type, 0, sizeof(count_device_type));

        VkPhysicalDeviceProperties physicalDeviceProperties;
        for (uint32_t i = 0; i < gpu_count; i++) {
            vkGetPhysicalDeviceProperties(physical_devices[i], &physicalDeviceProperties);
            assert(physicalDeviceProperties.deviceType <= VK_PHYSICAL_DEVICE_TYPE_CPU);
            count_device_type[physicalDeviceProperties.deviceType]++;
        }

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
                demo->gpu_number = i;
                break;
            }
        }
    }
#endif
    assert(demo->gpu_number >= 0);
    demo->gpu = physical_devices[demo->gpu_number];
    {
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(demo->gpu, &physicalDeviceProperties);
        fprintf(stderr, "Selected GPU %d: %s, type: %u\n", demo->gpu_number, physicalDeviceProperties.deviceName,
                physicalDeviceProperties.deviceType);
    }
    free(physical_devices);

    /* Look for device extensions */
    uint32_t device_extension_count = 0;
    VkBool32 swapchainExtFound = 0;
    demo->enabled_extension_count = 0;
    memset(demo->extension_names, 0, sizeof(demo->extension_names));

    err = vkEnumerateDeviceExtensionProperties(demo->gpu, NULL, &device_extension_count, NULL);
    assert(!err);

    if (device_extension_count > 0) {
        VkExtensionProperties *device_extensions = malloc(sizeof(VkExtensionProperties) * device_extension_count);
        err = vkEnumerateDeviceExtensionProperties(demo->gpu, NULL, &device_extension_count, device_extensions);
        assert(!err);

        for (uint32_t i = 0; i < device_extension_count; i++) {
            if (!strcmp(VK_KHR_SWAPCHAIN_EXTENSION_NAME, device_extensions[i].extensionName)) {
                swapchainExtFound = 1;
                demo->extension_names[demo->enabled_extension_count++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
            }
            if (!strcmp("VK_KHR_portability_subset", device_extensions[i].extensionName)) {
                demo->extension_names[demo->enabled_extension_count++] = "VK_KHR_portability_subset";
            }
            assert(demo->enabled_extension_count < 64);
        }

        if (demo->VK_KHR_incremental_present_enabled) {
            // Even though the user "enabled" the extension via the command
            // line, we must make sure that it's enumerated for use with the
            // device.  Therefore, disable it here, and re-enable it again if
            // enumerated.
            demo->VK_KHR_incremental_present_enabled = false;
            for (uint32_t i = 0; i < device_extension_count; i++) {
                if (!strcmp(VK_KHR_INCREMENTAL_PRESENT_EXTENSION_NAME, device_extensions[i].extensionName)) {
                    demo->extension_names[demo->enabled_extension_count++] = VK_KHR_INCREMENTAL_PRESENT_EXTENSION_NAME;
                    demo->VK_KHR_incremental_present_enabled = true;
                    DbgMsg("VK_KHR_incremental_present extension enabled\n");
                }
                assert(demo->enabled_extension_count < 64);
            }
            if (!demo->VK_KHR_incremental_present_enabled) {
                DbgMsg("VK_KHR_incremental_present extension NOT AVAILABLE\n");
            }
        }

        if (demo->VK_GOOGLE_display_timing_enabled) {
            // Even though the user "enabled" the extension via the command
            // line, we must make sure that it's enumerated for use with the
            // device.  Therefore, disable it here, and re-enable it again if
            // enumerated.
            demo->VK_GOOGLE_display_timing_enabled = false;
            for (uint32_t i = 0; i < device_extension_count; i++) {
                if (!strcmp(VK_GOOGLE_DISPLAY_TIMING_EXTENSION_NAME, device_extensions[i].extensionName)) {
                    demo->extension_names[demo->enabled_extension_count++] = VK_GOOGLE_DISPLAY_TIMING_EXTENSION_NAME;
                    demo->VK_GOOGLE_display_timing_enabled = true;
                    DbgMsg("VK_GOOGLE_display_timing extension enabled\n");
                }
                assert(demo->enabled_extension_count < 64);
            }
            if (!demo->VK_GOOGLE_display_timing_enabled) {
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

    if (demo->validate) {
        // Setup VK_EXT_debug_utils function pointers always (we use them for
        // debug labels and names).
        demo->CreateDebugUtilsMessengerEXT =
                (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(demo->inst, "vkCreateDebugUtilsMessengerEXT");
        demo->DestroyDebugUtilsMessengerEXT =
                (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(demo->inst, "vkDestroyDebugUtilsMessengerEXT");
        demo->SubmitDebugUtilsMessageEXT =
                (PFN_vkSubmitDebugUtilsMessageEXT)vkGetInstanceProcAddr(demo->inst, "vkSubmitDebugUtilsMessageEXT");
        demo->CmdBeginDebugUtilsLabelEXT =
                (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(demo->inst, "vkCmdBeginDebugUtilsLabelEXT");
        demo->CmdEndDebugUtilsLabelEXT =
                (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(demo->inst, "vkCmdEndDebugUtilsLabelEXT");
        demo->CmdInsertDebugUtilsLabelEXT =
                (PFN_vkCmdInsertDebugUtilsLabelEXT)vkGetInstanceProcAddr(demo->inst, "vkCmdInsertDebugUtilsLabelEXT");
        demo->SetDebugUtilsObjectNameEXT =
                (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(demo->inst, "vkSetDebugUtilsObjectNameEXT");
        if (NULL == demo->CreateDebugUtilsMessengerEXT || NULL == demo->DestroyDebugUtilsMessengerEXT ||
            NULL == demo->SubmitDebugUtilsMessageEXT || NULL == demo->CmdBeginDebugUtilsLabelEXT ||
            NULL == demo->CmdEndDebugUtilsLabelEXT || NULL == demo->CmdInsertDebugUtilsLabelEXT ||
            NULL == demo->SetDebugUtilsObjectNameEXT) {
            ERR_EXIT("GetProcAddr: Failed to init VK_EXT_debug_utils\n", "GetProcAddr: Failure");
        }

        err = demo->CreateDebugUtilsMessengerEXT(demo->inst, &dbg_messenger_create_info, NULL, &demo->dbg_messenger);
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
    vkGetPhysicalDeviceProperties(demo->gpu, &demo->gpu_props);

    /* Call with NULL data to get count */
    vkGetPhysicalDeviceQueueFamilyProperties(demo->gpu, &demo->queue_family_count, NULL);
    assert(demo->queue_family_count >= 1);

    demo->queue_props = (VkQueueFamilyProperties *)malloc(demo->queue_family_count * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(demo->gpu, &demo->queue_family_count, demo->queue_props);

    // Query fine-grained feature support for this device.
    //  If app has specific feature requirements it should check supported
    //  features based on this query
    VkPhysicalDeviceFeatures physDevFeatures;
    vkGetPhysicalDeviceFeatures(demo->gpu, &physDevFeatures);

    GET_INSTANCE_PROC_ADDR(demo->inst, GetPhysicalDeviceSurfaceSupportKHR);
    GET_INSTANCE_PROC_ADDR(demo->inst, GetPhysicalDeviceSurfaceCapabilitiesKHR);
    GET_INSTANCE_PROC_ADDR(demo->inst, GetPhysicalDeviceSurfaceFormatsKHR);
    GET_INSTANCE_PROC_ADDR(demo->inst, GetPhysicalDeviceSurfacePresentModesKHR);
    GET_INSTANCE_PROC_ADDR(demo->inst, GetSwapchainImagesKHR);
}

static void demo_create_device(struct demo *demo) {
    VkResult U_ASSERT_ONLY err;
    float queue_priorities[1] = {0.0};
    VkDeviceQueueCreateInfo queues[2];
    queues[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queues[0].pNext = NULL;
    queues[0].queueFamilyIndex = demo->graphics_queue_family_index;
    queues[0].queueCount = 1;
    queues[0].pQueuePriorities = queue_priorities;
    queues[0].flags = 0;

    VkDeviceCreateInfo device = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = NULL,
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = queues,
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = NULL,
            .enabledExtensionCount = demo->enabled_extension_count,
            .ppEnabledExtensionNames = (const char *const *)demo->extension_names,
            .pEnabledFeatures = NULL,  // If specific features are required, pass them in here
    };
  /*
  if (demo->separate_present_queue) {
        queues[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queues[1].pNext = NULL;
        queues[1].queueFamilyIndex = demo->present_queue_family_index;
        queues[1].queueCount = 1;
        queues[1].pQueuePriorities = queue_priorities;
        queues[1].flags = 0;
        device.queueCreateInfoCount = 2;
    }
  */

    queues[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queues[1].pNext = NULL;
    queues[1].queueFamilyIndex = demo->graphics_queue_family_index2;
    queues[1].queueCount = 1;
    queues[1].pQueuePriorities = queue_priorities;
    queues[1].flags = 0;
    device.queueCreateInfoCount = 2;
    err = vkCreateDevice(demo->gpu, &device, NULL, &demo->device);
    assert(!err);
}

static void demo_create_surface(struct demo *demo) {
    VkResult U_ASSERT_ONLY err;

// Create a WSI surface for the window:
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    VkWin32SurfaceCreateInfoKHR createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.hinstance = demo->connection;
    createInfo.hwnd = demo->window;

    err = vkCreateWin32SurfaceKHR(demo->inst, &createInfo, NULL, &demo->surface);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    VkWaylandSurfaceCreateInfoKHR createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.display = demo->display;
    createInfo.surface = demo->window;

    err = vkCreateWaylandSurfaceKHR(demo->inst, &createInfo, NULL, &demo->surface);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    VkAndroidSurfaceCreateInfoKHR createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.window = (struct ANativeWindow *)(demo->window);

    err = vkCreateAndroidSurfaceKHR(demo->inst, &createInfo, NULL, &demo->surface);
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
    VkXlibSurfaceCreateInfoKHR createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.dpy = demo->display;
    createInfo.window = demo->xlib_window;

    err = vkCreateXlibSurfaceKHR(demo->inst, &createInfo, NULL, &demo->surface);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
    VkXcbSurfaceCreateInfoKHR createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.connection = demo->connection;
    createInfo.window = demo->xcb_window;

    err = vkCreateXcbSurfaceKHR(demo->inst, &createInfo, NULL, &demo->surface);
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
    VkDirectFBSurfaceCreateInfoEXT createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_DIRECTFB_SURFACE_CREATE_INFO_EXT;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.dfb = demo->dfb;
    createInfo.surface = demo->window;

    err = vkCreateDirectFBSurfaceEXT(demo->inst, &createInfo, NULL, &demo->surface);
#elif defined(VK_USE_PLATFORM_DISPLAY_KHR)
    err = demo_create_display_surface(demo);
#elif defined(VK_USE_PLATFORM_METAL_EXT)
    VkMetalSurfaceCreateInfoEXT surface;
    surface.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
    surface.pNext = NULL;
    surface.flags = 0;
    surface.pLayer = demo->caMetalLayer;

    err = vkCreateMetalSurfaceEXT(demo->inst, &surface, NULL, &demo->surface);
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
static void demo_init_vk_swapchain(struct demo *demo) {
    VkResult U_ASSERT_ONLY err;

    demo_create_surface(demo);

    // Iterate over each queue to learn whether it supports presenting:
    VkBool32 *supportsPresent = (VkBool32 *)malloc(demo->queue_family_count * sizeof(VkBool32));
    for (uint32_t i = 0; i < demo->queue_family_count; i++) {
        demo->fpGetPhysicalDeviceSurfaceSupportKHR(demo->gpu, i, demo->surface, &supportsPresent[i]);
    }

    // Search for a graphics and a present queue in the array of queue
    // families, try to find one that supports both
    uint32_t graphicsQueueFamilyIndex = UINT32_MAX;
    uint32_t presentQueueFamilyIndex = UINT32_MAX;
    uint32_t graphicsQueueFamilyIndex2 = UINT32_MAX;
    uint32_t presentQueueFamilyIndex2 = UINT32_MAX;
    bool val1 = false;
    for (uint32_t i = 0; i < demo->queue_family_count; i++) {
        if ((demo->queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
            if (graphicsQueueFamilyIndex == UINT32_MAX) {
                graphicsQueueFamilyIndex = i;
            }

            if (val1 && supportsPresent[i] == VK_TRUE) {
                graphicsQueueFamilyIndex2 = i;
                presentQueueFamilyIndex2 = i;
                break;
            }
            if (!val1 && supportsPresent[i] == VK_TRUE) {
                graphicsQueueFamilyIndex = i;
                presentQueueFamilyIndex = i;
                val1 = true;
            }
        }
    }

    if (presentQueueFamilyIndex == UINT32_MAX) {
        // If we didn't find a queue that supports both graphics and present, then
        // find a separate present queue.
        for (uint32_t i = 0; i < demo->queue_family_count; ++i) {
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

    demo->graphics_queue_family_index = graphicsQueueFamilyIndex;
    demo->present_queue_family_index = presentQueueFamilyIndex;
    demo->separate_present_queue = (demo->graphics_queue_family_index != demo->present_queue_family_index);
    free(supportsPresent);

    demo_create_device(demo);

    demo->fpCreateSwapchainKHR = vkCreateSwapchainKHR;
    demo->fpDestroySwapchainKHR = vkDestroySwapchainKHR;
    demo->fpGetSwapchainImagesKHR = vkGetSwapchainImagesKHR;
    demo->fpAcquireNextImageKHR = vkAcquireNextImageKHR;
    demo->fpQueuePresentKHR = vkQueuePresentKHR;
    demo->fpGetRefreshCycleDurationGOOGLE = vkGetRefreshCycleDurationGOOGLE;
    demo->fpGetPastPresentationTimingGOOGLE = vkGetPastPresentationTimingGOOGLE;

     /*
    GET_DEVICE_PROC_ADDR(demo->device, CreateSwapchainKHR);
    GET_DEVICE_PROC_ADDR(demo->device, DestroySwapchainKHR);
    GET_DEVICE_PROC_ADDR(demo->device, GetSwapchainImagesKHR);
    GET_DEVICE_PROC_ADDR(demo->device, AcquireNextImageKHR);
    GET_DEVICE_PROC_ADDR(demo->device, QueuePresentKHR);
    if (demo->VK_GOOGLE_display_timing_enabled) {
        GET_DEVICE_PROC_ADDR(demo->device, GetRefreshCycleDurationGOOGLE);
        GET_DEVICE_PROC_ADDR(demo->device, GetPastPresentationTimingGOOGLE);
    }
     */

    vkGetDeviceQueue(demo->device, demo->graphics_queue_family_index, 0, &demo->graphics_queue);
    vkGetDeviceQueue(demo->device, demo->graphics_queue_family_index2, 0, &demo->graphics_queue2);

    if (!demo->separate_present_queue) {
        demo->present_queue = demo->graphics_queue;
    } else {
        vkGetDeviceQueue(demo->device, demo->present_queue_family_index, 0, &demo->present_queue);
    }

    // Get the list of VkFormat's that are supported:
    uint32_t formatCount;
    err = demo->fpGetPhysicalDeviceSurfaceFormatsKHR(demo->gpu, demo->surface, &formatCount, NULL);
    assert(!err);
    VkSurfaceFormatKHR *surfFormats = (VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
    err = demo->fpGetPhysicalDeviceSurfaceFormatsKHR(demo->gpu, demo->surface, &formatCount, surfFormats);
    assert(!err);
    VkSurfaceFormatKHR surfaceFormat = pick_surface_format(surfFormats, formatCount);
    demo->format = surfaceFormat.format;
    demo->color_space = surfaceFormat.colorSpace;
    free(surfFormats);

    demo->quit = false;
    demo->curFrame = 0;

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
        err = vkCreateFence(demo->device, &fence_ci, NULL, &demo->fences[i]);
        assert(!err);

        err = vkCreateSemaphore(demo->device, &semaphoreCreateInfo, NULL, &demo->image_acquired_semaphores[i]);
        assert(!err);

        err = vkCreateSemaphore(demo->device, &semaphoreCreateInfo, NULL, &demo->draw_complete_semaphores[i]);
        assert(!err);

        if (demo->separate_present_queue) {
            err = vkCreateSemaphore(demo->device, &semaphoreCreateInfo, NULL, &demo->image_ownership_semaphores[i]);
            assert(!err);
        }
    }
    demo->frame_index = 0;

    // Get Memory information and properties
    vkGetPhysicalDeviceMemoryProperties(demo->gpu, &demo->memory_properties);
}

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
static void pointer_handle_enter(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t sx,
                                 wl_fixed_t sy) {}

static void pointer_handle_leave(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface) {}

static void pointer_handle_motion(void *data, struct wl_pointer *pointer, uint32_t time, wl_fixed_t sx, wl_fixed_t sy) {}

static void pointer_handle_button(void *data, struct wl_pointer *wl_pointer, uint32_t serial, uint32_t time, uint32_t button,
                                  uint32_t state) {
    struct demo *demo = data;
    if (button == BTN_LEFT && state == WL_POINTER_BUTTON_STATE_PRESSED) {
        xdg_toplevel_move(demo->xdg_toplevel, demo->seat, serial);
    }
}

static void pointer_handle_axis(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis, wl_fixed_t value) {}

static const struct wl_pointer_listener pointer_listener = {
    pointer_handle_enter, pointer_handle_leave, pointer_handle_motion, pointer_handle_button, pointer_handle_axis,
};

static void keyboard_handle_keymap(void *data, struct wl_keyboard *keyboard, uint32_t format, int fd, uint32_t size) {}

static void keyboard_handle_enter(void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface,
                                  struct wl_array *keys) {}

static void keyboard_handle_leave(void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface) {}

static void keyboard_handle_key(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key,
                                uint32_t state) {
    if (state != WL_KEYBOARD_KEY_STATE_RELEASED) return;
    struct demo *demo = data;
    switch (key) {
        case KEY_ESC:  // Escape
            demo->quit = true;
            break;
        case KEY_LEFT:  // left arrow key
            demo->spin_angle -= demo->spin_increment;
            break;
        case KEY_RIGHT:  // right arrow key
            demo->spin_angle += demo->spin_increment;
            break;
        case KEY_SPACE:  // space bar
            demo->pause = !demo->pause;
            break;
    }
}

static void keyboard_handle_modifiers(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t mods_depressed,
                                      uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {}

static const struct wl_keyboard_listener keyboard_listener = {
    keyboard_handle_keymap, keyboard_handle_enter, keyboard_handle_leave, keyboard_handle_key, keyboard_handle_modifiers,
};

static void seat_handle_capabilities(void *data, struct wl_seat *seat, enum wl_seat_capability caps) {
    // Subscribe to pointer events
    struct demo *demo = data;
    if ((caps & WL_SEAT_CAPABILITY_POINTER) && !demo->pointer) {
        demo->pointer = wl_seat_get_pointer(seat);
        wl_pointer_add_listener(demo->pointer, &pointer_listener, demo);
    } else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && demo->pointer) {
        wl_pointer_destroy(demo->pointer);
        demo->pointer = NULL;
    }
    // Subscribe to keyboard events
    if (caps & WL_SEAT_CAPABILITY_KEYBOARD) {
        demo->keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(demo->keyboard, &keyboard_listener, demo);
    } else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD)) {
        wl_keyboard_destroy(demo->keyboard);
        demo->keyboard = NULL;
    }
}

static const struct wl_seat_listener seat_listener = {
    seat_handle_capabilities,
};

static void wm_base_ping(void *data UNUSED, struct xdg_wm_base *xdg_wm_base, uint32_t serial) {
    xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener wm_base_listener = {wm_base_ping};

static void registry_handle_global(void *data, struct wl_registry *registry, uint32_t id, const char *interface,
                                   uint32_t version UNUSED) {
    struct demo *demo = data;
    // pickup wayland objects when they appear
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        uint32_t minVersion = version < 4 ? version : 4;
        demo->compositor = wl_registry_bind(registry, id, &wl_compositor_interface, minVersion);
        if (demo->VK_KHR_incremental_present_enabled && minVersion < 4) {
            fprintf(stderr, "Wayland compositor doesn't support VK_KHR_incremental_present, disabling.\n");
            demo->VK_KHR_incremental_present_enabled = false;
        }
    } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        demo->xdg_wm_base = wl_registry_bind(registry, id, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(demo->xdg_wm_base, &wm_base_listener, NULL);
    } else if (strcmp(interface, wl_seat_interface.name) == 0) {
        demo->seat = wl_registry_bind(registry, id, &wl_seat_interface, 1);
        wl_seat_add_listener(demo->seat, &seat_listener, demo);
    } else if (strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0) {
        demo->xdg_decoration_mgr = wl_registry_bind(registry, id, &zxdg_decoration_manager_v1_interface, 1);
    }
}

static void registry_handle_global_remove(void *data UNUSED, struct wl_registry *registry UNUSED, uint32_t name UNUSED) {}

static const struct wl_registry_listener registry_listener = {registry_handle_global, registry_handle_global_remove};
#endif

static void demo_init(struct demo *demo, int argc, char **argv) {
    vec3 eye = {0.0f, 3.0f, 5.0f};
    vec3 origin = {0, 0, 0};
    vec3 up = {0.0f, 1.0f, 0.0};

    //memset(demo, 0, sizeof(*demo));
    demo->presentMode = VK_PRESENT_MODE_FIFO_KHR;
    demo->frameCount = INT32_MAX;
    /* Autodetect suitable / best GPU by default */
    demo->gpu_number = -1;
    demo->width = 500;
    demo->height = 500;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--use_staging") == 0) {
            demo->use_staging_buffer = true;
            continue;
        }
        if ((strcmp(argv[i], "--present_mode") == 0) && (i < argc - 1)) {
            demo->presentMode = atoi(argv[i + 1]);
            i++;
            continue;
        }
        if (strcmp(argv[i], "--break") == 0) {
            demo->use_break = true;
            continue;
        }
        if (strcmp(argv[i], "--validate") == 0) {
            demo->validate = true;
            continue;
        }
        if (strcmp(argv[i], "--validate-checks-disabled") == 0) {
            demo->validate = true;
            demo->validate_checks_disabled = true;
            continue;
        }
        if (strcmp(argv[i], "--xlib") == 0) {
            fprintf(stderr, "--xlib is deprecated and no longer does anything");
            continue;
        }
        if (strcmp(argv[i], "--c") == 0 && demo->frameCount == INT32_MAX && i < argc - 1 &&
            sscanf(argv[i + 1], "%d", &demo->frameCount) == 1 && demo->frameCount >= 0) {
            i++;
            continue;
        }
        if (strcmp(argv[i], "--width") == 0 && i < argc - 1 && sscanf(argv[i + 1], "%d", &demo->width) == 1 && demo->width > 0) {
            i++;
            continue;
        }
        if (strcmp(argv[i], "--height") == 0 && i < argc - 1 && sscanf(argv[i + 1], "%d", &demo->height) == 1 && demo->height > 0) {
            i++;
            continue;
        }
        if (strcmp(argv[i], "--suppress_popups") == 0) {
            demo->suppress_popups = true;
            continue;
        }
        if (strcmp(argv[i], "--display_timing") == 0) {
            demo->VK_GOOGLE_display_timing_enabled = true;
            continue;
        }
        if (strcmp(argv[i], "--incremental_present") == 0) {
            demo->VK_KHR_incremental_present_enabled = true;
            continue;
        }
        if ((strcmp(argv[i], "--gpu_number") == 0) && (i < argc - 1)) {
            demo->gpu_number = atoi(argv[i + 1]);
            assert(demo->gpu_number >= 0);
            i++;
            continue;
        }

#if defined(ANDROID)
        ERR_EXIT("Usage: vkcube [--validate]\n", "Usage");
#else
        char *message =
                "Usage:\n  %s\t[--use_staging] [--validate] [--validate-checks-disabled]\n"
                "\t[--break] [--c <framecount>] [--suppress_popups]\n"
                "\t[--incremental_present] [--display_timing]\n"
                "\t[--gpu_number <index of physical device>]\n"
                "\t[--present_mode <present mode enum>]\n"
                "\t[--width <width>] [--height <height>]\n"
                "\t<present_mode_enum>\n"
                "\t\tVK_PRESENT_MODE_IMMEDIATE_KHR = %d\n"
                "\t\tVK_PRESENT_MODE_MAILBOX_KHR = %d\n"
                "\t\tVK_PRESENT_MODE_FIFO_KHR = %d\n"
                "\t\tVK_PRESENT_MODE_FIFO_RELAXED_KHR = %d\n";
        int length = snprintf(NULL, 0, message, APP_SHORT_NAME, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_MAILBOX_KHR,
                              VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_FIFO_RELAXED_KHR);
        char *usage = (char *)malloc(length + 1);
        if (!usage) {
            exit(1);
        }
        snprintf(usage, length + 1, message, APP_SHORT_NAME, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_MAILBOX_KHR,
                 VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_FIFO_RELAXED_KHR);
#if defined(_WIN32)
        if (!demo->suppress_popups) MessageBox(NULL, usage, "Usage Error", MB_OK);
#else
        fprintf(stderr, "%s", usage);
        fflush(stderr);
#endif
        free(usage);
        exit(1);
#endif
    }

    demo_init_vk(demo);

    // degrees per second
    demo->spin_angle = 8.0f;
    demo->spin_increment = 0.2f;
    demo->pause = false;

    mat4x4_perspective(demo->projection_matrix, (float)degreesToRadians(45.0f), 1.0f, 0.1f, 100.0f);
    mat4x4_look_at(demo->view_matrix, eye, origin, up);
    mat4x4_identity(demo->model_matrix);

    demo->projection_matrix[1][1] *= -1;  // Flip projection matrix from GL to Vulkan orientation.
}

// https://developer.android.com/ndk/reference/group/asset#group___asset_1ga90c459935e76acf809b9ec90d1872771

void set_textures_android(AAssetManager *pAssetManager) {
    assetManager = pAssetManager;
    tex_files = (char**) malloc((sizeof (char*)) * DEMO_TEXTURE_COUNT);
    for (int i = 0; i < DEMO_TEXTURE_COUNT; i++) {
        // 5+1=6 for \0
        size_t allocatedSize = strlen("textures") + strlen(tex_files_short[i]) + 6;
        tex_files[i] = (char*) malloc((sizeof(char)) * allocatedSize);
        snprintf(tex_files[i], allocatedSize, "%s/%s.png", "textures", tex_files_short[i]);
    }
}

#if defined(VK_USE_PLATFORM_METAL_EXT)
void demo_main(struct demo *demo, void *caMetalLayer, int argc, const char *argv[]) {
    //demo->VK_GOOGLE_display_timing_enabled = true;
    demo_init(demo, argc, (char **)argv);
    demo->caMetalLayer = caMetalLayer;
    demo_init_vk_swapchain(demo);
    // degrees per second
    demo->spin_angle = 8;
}
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
#include <android/log.h>
#include <android_native_app_glue.h>
#endif

struct demo demo;

#ifdef __ANDROID__
int demo_main_android(struct demo *demo, struct ANativeWindow* window, int argc, char **argv) {
    memset(demo, 0, sizeof(*demo));
    demo->window = window;

    demo_init(demo, argc, argv);

    demo_init_vk_swapchain(demo);

    demo_prepare(demo);

    demo_run(demo);

    return validation_error;
}
#endif

void setSizeFull(struct demo *demo, int32_t width, int32_t height) {
    demo->width = width;
    demo->height = height;
    demo_resize(demo);
}

void freeResources() {
  for (int i = 0; i < DEMO_TEXTURE_COUNT; i++) {
    free(tex_files[i]);
  }
  free(tex_files);
}


