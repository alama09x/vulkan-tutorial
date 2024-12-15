#include "application.h"

#include "init/device.h"
#include "init/instance.h"
#include "init/validation.h"
#include "init/swapchain.h"
#include "init/render_pass.h"
#include "init/graphics_pipeline.h"
#include "init/framebuffers.h"
#include "init/commands.h"
#include "init/sync_objects.h"
#include "init/draw_frame.h"

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

static void framebuffer_resize_callback(GLFWwindow *window, int width, int height)
{
    struct application *app = (struct application *)glfwGetWindowUserPointer(window);
    app->framebuffer_resized = true;
}

static enum app_result init_window(struct application *app)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    if (!(app->window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", NULL, NULL))) {
        fputs("Error: failed to create window!\n", stderr);
        return APP_ERROR;
    }

    glfwSetWindowUserPointer(app->window, app);
    glfwSetFramebufferSizeCallback(app->window, framebuffer_resize_callback);
    return APP_SUCCESS;
}

static enum app_result init_vulkan(struct application *app)
{
    enum app_result result = APP_SUCCESS;
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

    if ((result = create_command_buffers(app)) != APP_SUCCESS) {
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
    uint32_t current_frame = 0;
    app->framebuffer_resized = false;
    while (!glfwWindowShouldClose(app->window)) {
        glfwPollEvents();
        draw_frame(app, current_frame);
        current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    vkDeviceWaitIdle(app->device);

    return APP_SUCCESS;
}

static enum app_result cleanup(struct application *app)
{
    cleanup_swapchain(app);

    vkDestroyPipeline(app->device, app->graphics_pipeline, NULL);
    vkDestroyPipelineLayout(app->device, app->pipeline_layout, NULL);
    vkDestroyRenderPass(app->device, app->render_pass, NULL);

    for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(app->device, app->image_available_semaphores[i], NULL);
        vkDestroySemaphore(app->device, app->render_finished_semaphores[i], NULL);
        vkDestroyFence(app->device, app->in_flight_fences[i], NULL);
    }

    vkDestroyCommandPool(app->device, app->command_pool, NULL);

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
