#include "init/instance.h"
#include "init/validation.h"
#include <stdio.h>
#include <string.h>

static void get_required_extensions(uint32_t *req_ext_count, const char **req_extensions)
{
    uint32_t glfw_ext_count = 0;
    const char **glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_ext_count);

    *req_ext_count = glfw_ext_count + ENABLE_VALIDATION_LAYERS;

    if (req_extensions) {
        for (uint32_t i = 0; i < glfw_ext_count; i++) {
            req_extensions[i] = glfw_extensions[i];
        }

        if (ENABLE_VALIDATION_LAYERS) {
            req_extensions[glfw_ext_count] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
        }
    }
}

static bool required_extensions_available(
    const char **req,
    VkExtensionProperties *avb,
    uint32_t reqc,
    uint32_t avbc)
{
    for (uint32_t i = 0; i < reqc; i++) {
        bool included = false;

        for (uint32_t j = 0; j < avbc; j++) {
            const char *avb_str = avb[j].extensionName;
            if (strncmp(req[i], avb_str, strlen(avb_str)) == 0) {
                included = true;
            }
        }

        if (!included) {
            return false;
        }
    }

    return true;
}

enum app_result create_instance(struct application *app)
{
    if (ENABLE_VALIDATION_LAYERS && !check_validation_layer_support()) {
        fputs("Error: validation layers requested, but not available!\n", stderr);
        return APP_ERROR;
    }

    const VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Hello Triangle",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0,
    };

    uint32_t req_ext_count;
    get_required_extensions(&req_ext_count, NULL);

    const char *req_extensions[req_ext_count];
    get_required_extensions(&req_ext_count, req_extensions);

    uint32_t avb_ext_count = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &avb_ext_count, NULL);

    VkExtensionProperties avb_extensions[avb_ext_count];
    vkEnumerateInstanceExtensionProperties(NULL, &avb_ext_count, avb_extensions);

    puts("Available instance extensions:");
    for (uint32_t i = 0; i < avb_ext_count; i++) {
        printf("\t%s\n", avb_extensions[i].extensionName);
    }

    if (!required_extensions_available(
        req_extensions,
        avb_extensions,
        req_ext_count,
        avb_ext_count))
    {
        fputs("Error: required extensions not available!\n", stderr);
        return APP_ERROR;
    }

    VkInstanceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info,
        .enabledExtensionCount = req_ext_count,
        .ppEnabledExtensionNames = req_extensions,
    };
    VkDebugUtilsMessengerCreateInfoEXT debug_create_info;
    if (ENABLE_VALIDATION_LAYERS) {
        create_info.enabledLayerCount = VALIDATION_LAYER_COUNT;
        create_info.ppEnabledLayerNames = VALIDATION_LAYERS;

        debug_create_info = configure_debug_messenger_create_info();

        create_info.pNext = &debug_create_info;
    } else {
        create_info.enabledLayerCount = 0;
    }


    if (vkCreateInstance(&create_info, NULL, &app->instance) != VK_SUCCESS) {
        fputs("Error: failed to create instance!\n", stderr);
        return APP_ERROR;
    }

    return APP_SUCCESS;
}
