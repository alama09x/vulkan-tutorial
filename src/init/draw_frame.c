#include "init/draw_frame.h"
#include "init/swapchain.h"
#include "init/framebuffers.h"
#include "init/commands.h"

#include <stdio.h>
#include <stdlib.h>

enum app_result cleanup_swapchain(struct application *app)
{
    if (app->swapchain_framebuffers) {
        for (uint32_t i = 0; i < app->swapchain_image_count; i++) {
            vkDestroyFramebuffer(app->device, app->swapchain_framebuffers[i], NULL);
        }
        free(app->swapchain_framebuffers);
    }

    for (uint32_t i = 0; i < app->swapchain_image_count; i++) {
        vkDestroyImageView(app->device, app->swapchain_image_views[i], NULL);
    }

    vkDestroySwapchainKHR(app->device, app->swapchain, NULL);

    free(app->swapchain_image_views);
    free(app->swapchain_images);

    return APP_SUCCESS;
}

static enum app_result recreate_swapchain(struct application *app)
{
    int32_t width = 0, height = 0;
    glfwGetFramebufferSize(app->window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(app->window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(app->device);
    enum app_result result = APP_SUCCESS;
    if ((result = cleanup_swapchain(app)) != APP_SUCCESS) {
        fputs("Error: failed to cleanup swapchain!\n", stderr);
        return result;
    }

    if ((result = create_swapchain(app)) != APP_SUCCESS) {
        fputs("Error: failed to create swapchain!\n", stderr);
        return result;
    }

    if ((result = create_image_views(app)) != APP_SUCCESS) {
        fputs("Error: failed to create image views!\n", stderr);
        return result;
    }

    if ((result = create_framebuffers(app)) != APP_SUCCESS) {
        fputs("Error: failed to create framebuffers!\n", stderr);
        return result;
    }

    return APP_SUCCESS;
}

enum app_result draw_frame(struct application *app, uint32_t current_frame)
{
    vkWaitForFences(app->device, 1, &app->in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);

    uint32_t image_index;
    VkResult result = vkAcquireNextImageKHR(
        app->device,
        app->swapchain,
        UINT64_MAX,
        app->image_available_semaphores[current_frame],
        VK_NULL_HANDLE,
        &image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreate_swapchain(app);
        return APP_SUCCESS;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        fputs("Error: failed to acquire swap chain image!", stderr);
    }

    vkResetFences(app->device, 1, &app->in_flight_fences[current_frame]);

    vkResetCommandBuffer(app->command_buffers[current_frame], 0);
    record_command_buffer(app, app->command_buffers[current_frame], image_index);

    const VkSemaphore wait_semaphores[] = {
        app->image_available_semaphores[current_frame],
    };
    const VkPipelineStageFlags wait_stages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    };
    const VkSemaphore signal_semaphores[] = {
        app->render_finished_semaphores[current_frame],
    };

    const VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = wait_semaphores, // corresponds with below
        .pWaitDstStageMask = wait_stages,   // corresponds with above
        .commandBufferCount = 1,
        .pCommandBuffers = &app->command_buffers[current_frame],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signal_semaphores,
    };

    if (vkQueueSubmit(app->graphics_queue, 1, &submit_info, app->in_flight_fences[current_frame])
        != VK_SUCCESS)
    {
        fputs("Error: failed to submit draw command buffer!\n", stderr);
        return APP_ERROR;
    }

    const VkSwapchainKHR swapchains[] = { app->swapchain };

    const VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signal_semaphores,
        .swapchainCount = 1,
        .pSwapchains = swapchains,
        .pImageIndices = &image_index,
        .pResults = NULL,
    };

    result = vkQueuePresentKHR(app->present_queue, &present_info);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        app->framebuffer_resized) {
        app->framebuffer_resized = false;
        recreate_swapchain(app);
    } else if (result != VK_SUCCESS) {
        fputs("Error: failed to present swapchain image!\n", stderr);
    }

    return APP_SUCCESS;
}

