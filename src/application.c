#include "application.h"

#include "init/device.h"
#include "init/instance.h"
#include "init/validation.h"
#include "init/swapchain.h"
#include "init/render_pass.h"
#include "init/graphics_pipeline.h"
#include "init/framebuffers.h"
#include "init/commands.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#ifdef NDEBUG
const bool ENABLE_VALIDATION_LAYERS = false;
#else
const bool ENABLE_VALIDATION_LAYERS = true;
#endif

static const uint32_t WIDTH = 800;
static const uint32_t HEIGHT = 600;

enum app_result draw_frame(struct application *app)
{
    vkWaitForFences(app->device, 1, &app->in_flight_fence, VK_TRUE, UINT64_MAX);
    vkResetFences(app->device, 1, &app->in_flight_fence);

    uint32_t image_index;
    vkAcquireNextImageKHR(
        app->device,
        app->swapchain,
        UINT64_MAX,
        app->image_available_semaphore,
        VK_NULL_HANDLE,
        &image_index);

    vkResetCommandBuffer(app->command_buffer, 0);
    record_command_buffer(app, app->command_buffer, image_index);

    const VkSemaphore wait_semaphores[] = { app->image_available_semaphore };
    const VkPipelineStageFlags wait_stages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    };

    const VkSemaphore signal_semaphores[] = { app->render_finished_semaphore };

    const VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = wait_semaphores, // corresponds with below
        .pWaitDstStageMask = wait_stages,   // corresponds with above
        .commandBufferCount = 1,
        .pCommandBuffers = &app->command_buffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signal_semaphores,
    };

    if (vkQueueSubmit(app->graphics_queue, 1, &submit_info, app->in_flight_fence)
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

    vkQueuePresentKHR(app->present_queue, &present_info);

    return APP_SUCCESS;
}

enum app_result create_sync_objects(struct application *app)
{
    const VkSemaphoreCreateInfo semaphore_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    const VkFenceCreateInfo fence_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    if (vkCreateSemaphore(app->device, &semaphore_info, NULL, &app->image_available_semaphore)
        != VK_SUCCESS ||
        vkCreateSemaphore(app->device, &semaphore_info, NULL, &app->render_finished_semaphore)
        != VK_SUCCESS)
    {
        fputs("Error: failed to create semaphores!\n", stderr);
        return APP_ERROR;
    }

    if (vkCreateFence(app->device, &fence_info, NULL, &app->in_flight_fence)
        != VK_SUCCESS)
    {
        fputs("Error: failed to create fence!\n", stderr);
        return APP_ERROR;
    }

    return APP_SUCCESS;
}

static enum app_result create_surface(struct application *app)
{
    if (glfwCreateWindowSurface(app->instance, app->window, NULL, &app->surface)
        != VK_SUCCESS)
    {
        fputs("Error: failed to create window surface!\n", stderr);
        return APP_ERROR;
    }

    return APP_SUCCESS;
}

static enum app_result init_window(struct application *app)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    if (!(app->window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", NULL, NULL))) {
        fputs("Error: failed to create window!\n", stderr);
        return APP_ERROR;
    }
    return APP_SUCCESS;
}

static enum app_result init_vulkan(struct application *app)
{
    int result = APP_SUCCESS;
    if ((result = create_instance(app)) != APP_SUCCESS) {
        fputs("Error: failed to create instance!\n", stderr);
        return result;
    }

    if ((result = setup_debug_messenger(app)) != APP_SUCCESS) {
        fputs("Error: failed to setup debug messenger!\n", stderr);
        return result;
    }

    if ((result = create_surface(app)) != APP_SUCCESS) {
        fputs("Error: failed to create window surface!\n", stderr);
        return result;
    }

    if ((result = pick_physical_device(app)) != APP_SUCCESS) {
        fputs("Error: failed to pick physical device!\n", stderr);
        return result;
    }

    if ((result = create_logical_device(app)) != APP_SUCCESS) {
        fputs("Error: failed to create logical device!\n", stderr);
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

    if ((result = create_render_pass(app)) != APP_SUCCESS) {
        fputs("Error: failed to create render pass!\n", stderr);
        return result;
    }

    if ((result = create_graphics_pipeline(app)) != APP_SUCCESS) {
        fputs("Error: failed to create graphics pipeline!\n", stderr);
        return result;
    }

    if ((result = create_framebuffers(app)) != APP_SUCCESS) {
        fputs("Error: failed to create framebuffers!\n", stderr);
        return result;
    }

    if ((result = create_command_pool(app)) != APP_SUCCESS) {
        fputs("Error: failed to create command pool!\n", stderr);
        return result;
    }

    if ((result = create_command_buffer(app)) != APP_SUCCESS) {
        fputs("Error: failed to create command buffer!\n", stderr);
        return result;
    }

    if ((result = create_sync_objects(app)) != APP_SUCCESS) {
        fputs("Error: failed to create sync objects!\n", stderr);
        return result;
    }

    return APP_SUCCESS;
}

static enum app_result main_loop(struct application *app)
{
    while (!glfwWindowShouldClose(app->window)) {
        glfwPollEvents();
        draw_frame(app);
    }

    vkDeviceWaitIdle(app->device);

    return APP_SUCCESS;
}

static enum app_result cleanup(struct application *app)
{
    vkDestroySemaphore(app->device, app->image_available_semaphore, NULL);
    vkDestroySemaphore(app->device, app->render_finished_semaphore, NULL);
    vkDestroyFence(app->device, app->in_flight_fence, NULL);

    vkDestroyCommandPool(app->device, app->command_pool, NULL);

    if (app->swapchain_framebuffers) {
        for (uint32_t i = 0; i < app->swapchain_image_count; i++) {
            vkDestroyFramebuffer(app->device, app->swapchain_framebuffers[i], NULL);
        }
        free(app->swapchain_framebuffers);
    }

    vkDestroyPipeline(app->device, app->graphics_pipeline, NULL);
    vkDestroyPipelineLayout(app->device, app->pipeline_layout, NULL);
    vkDestroyRenderPass(app->device, app->render_pass, NULL);

    for (uint32_t i = 0; i < app->swapchain_image_count; i++) {
        vkDestroyImageView(app->device, app->swapchain_image_views[i], NULL);
    }

    free(app->swapchain_image_views);
    free(app->swapchain_images);

    vkDestroySwapchainKHR(app->device, app->swapchain, NULL);
    vkDestroyDevice(app->device, NULL);

    if (ENABLE_VALIDATION_LAYERS) {
        DestroyDebugUtilsMessengerEXT(app->instance, app->debug_messenger, NULL);
    }

    vkDestroySurfaceKHR(app->instance, app->surface, NULL);
    vkDestroyInstance(app->instance, NULL);
    glfwDestroyWindow(app->window);

    glfwTerminate();

    return APP_SUCCESS;
}

enum app_result app_run(struct application *app)
{
    int result;
    if ((result = init_window(app)) != APP_SUCCESS) {
        fputs("Error: failed to initialize window!\n", stderr);
        goto cleanup;
    }

    if ((result = init_vulkan(app)) != APP_SUCCESS) {
        fputs("Error failed to initialize Vulkan!\n", stderr);
        goto cleanup;
    }

    if ((result = main_loop(app)) != APP_SUCCESS) {
        fputs("Error: main loop failed!\n", stderr);
        goto cleanup;
    }

cleanup:
    cleanup(app);
    return result;
}
