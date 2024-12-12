#include "application.h"

#include "init/device.h"
#include "init/instance.h"
#include "init/validation.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#ifdef NDEBUG
const bool ENABLE_VALIDATION_LAYERS = false;
#else
const bool ENABLE_VALIDATION_LAYERS = true;
#endif

const uint32_t APPLICATION_STRUCT_SIZE = sizeof(struct application);

const uint8_t VALIDATION_LAYER_COUNT = 1;
const char *const VALIDATION_LAYERS[] = {
    "VK_LAYER_KHRONOS_validation",
};

static const uint32_t WIDTH = 800;
static const uint32_t HEIGHT = 600;

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

    if ((result = pick_physical_device(app)) != APP_SUCCESS) {
        fputs("Error: failed to pick physical device!\n", stderr);
        return result;
    }

    if ((result = create_logical_device(app)) != APP_SUCCESS) {
        fputs("Error: failed to create logical device!\n", stderr);
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
    if (app->device) {
        vkDestroyDevice(app->device, NULL);
    }

    if (ENABLE_VALIDATION_LAYERS) {
        DestroyDebugUtilsMessengerEXT(app->instance, app->debug_messenger, NULL);
    }
    if (app->instance) {
        vkDestroyInstance(app->instance, NULL);
    }

    if (app->window) {
        glfwDestroyWindow(app->window);
    }

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
