#include "init/commands.h"
#include "init/device.h"
#include <stdio.h>

enum app_result create_command_pool(struct application *app)
{
    struct queue_family_indices queue_family_indices;
    find_queue_families(app->physical_device, app->surface, &queue_family_indices);

    const VkCommandPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = *queue_family_indices.graphics_family,
    };

    if (vkCreateCommandPool(app->device, &pool_info, NULL, &app->command_pool)
        != VK_SUCCESS)
    {
        fputs("Error: failed to create command pool!\n", stderr);
        return APP_ERROR;
    }
    return APP_SUCCESS;
}

enum app_result create_command_buffer(struct application *app)
{
    const VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = app->command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    if (vkAllocateCommandBuffers(app->device, &alloc_info, &app->command_buffer)
        != VK_SUCCESS)
    {
        fputs("Error: failure to allocate command buffers!\n", stderr);
    }


    return APP_SUCCESS;
}

enum app_result record_command_buffer(
    struct application *app,
    VkCommandBuffer command_buffer,
    uint32_t image_index)
{
    const VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = 0,
        .pInheritanceInfo = NULL, // optional
    };

    if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) {
        fputs("Error: failed to begin recording command buffer!\n", stderr);
        return APP_ERROR;
    }

    const VkClearValue clear_color = {0.0f, 0.0f, 0.0f, 0.0f};

    const VkRenderPassBeginInfo render_pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = app->render_pass,
        .framebuffer = app->swapchain_framebuffers[image_index],
        .renderArea = {
            .offset = { .x = 0, .y = 0 },
            .extent = app->swapchain_extent,
        },
        .clearValueCount = 1,
        .pClearValues = &clear_color,
    };

    vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(
        command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, app->graphics_pipeline);

    const VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float)app->swapchain_extent.width,
        .height = (float)app->swapchain_extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    const VkRect2D scissor = {
        .offset = { .x = 0, .y = 0 },
        .extent = app->swapchain_extent,
    };
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    vkCmdDraw(command_buffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(command_buffer);

    if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
        fputs("Error: failed to record command buffer!\n", stderr);
        return APP_ERROR;
    }

    return APP_SUCCESS;
}
