#include "application.h"

#include "init/device.h"
#include "init/instance.h"
#include "init/validation.h"
#include "init/swapchain.h"
#include "init/graphics_pipeline.h"

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

    if ((result = create_graphics_pipeline(app)) != APP_SUCCESS) {
        fputs("Error: failed to create graphics pipeline!\n", stderr);
        return result;
    }

    return APP_SUCCESS;
}

static enum app_result main_loop(struct application *app)
{
    while (!glfwWindowShouldClose(app->window)) {
        glfwPollEvents();
    }

    return APP_SUCCESS;
}

static enum app_result cleanup(struct application *app)
{
    for (uint32_t i = 0; i < app->swapchain_image_count; i++) {
        vkDestroyImageView(app->device, app->swapchain_image_views[i], NULL);
    }

    if (app->swapchain_image_views) {
        free(app->swapchain_image_views);
    }
    if (app->swapchain_images) {
        free(app->swapchain_images);
    }

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
