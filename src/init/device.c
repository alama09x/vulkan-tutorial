#include "init/device.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

static bool queue_family_indices_complete(const struct queue_family_indices *indices)
{
    return indices->graphics_family != NULL;
}

static void find_queue_families(
    VkPhysicalDevice device,
    const struct queue_family_indices *indices)
{
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, NULL);

    VkQueueFamilyProperties queue_families[queue_family_count];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);

    for (uint32_t i = 0; i < queue_family_count; i++) {
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            *indices->graphics_family = i;
        }

        if (queue_family_indices_complete(indices)) {
            break;
        }
    }
}

static bool device_suitable(VkPhysicalDevice device)
{
    const struct queue_family_indices indices = {
        .graphics_family = malloc(sizeof(uint32_t)),
    };
    find_queue_families(device, &indices);

    bool result = queue_family_indices_complete(&indices);

    free(indices.graphics_family);

    return result;
}

enum app_result create_logical_device(struct application *app)
{
    const struct queue_family_indices indices = {
        .graphics_family = malloc(sizeof(uint32_t)),        
    };
    find_queue_families(app->physical_device, &indices);

    const float queue_priority = 1.0;
    const VkDeviceQueueCreateInfo queue_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = *indices.graphics_family,
        .queueCount = 1,
        .pQueuePriorities = &queue_priority,
    };

    const VkPhysicalDeviceFeatures device_features = {};

    VkDeviceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos = &queue_create_info,
        .queueCreateInfoCount = 1,
        .pEnabledFeatures = &device_features,
        .enabledExtensionCount = 0,
        .enabledLayerCount = 0,
    };
    if (ENABLE_VALIDATION_LAYERS) {
        create_info.enabledLayerCount = VALIDATION_LAYER_COUNT;
        create_info.ppEnabledLayerNames = VALIDATION_LAYERS;
    }

    if (vkCreateDevice(app->physical_device, &create_info, NULL, &app->device) != VK_SUCCESS) {
        fputs("Error: failed to create logical device!\n", stderr);
        return APP_ERROR;
    }

    vkGetDeviceQueue(app->device, *indices.graphics_family, 0, &app->graphics_queue);

    free(indices.graphics_family);
    return APP_SUCCESS;
}

enum app_result pick_physical_device(struct application *app)
{

    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(app->instance, &device_count, NULL);

    if (device_count == 0) {
        fputs("Error: failed to find GPUs with Vulkan support!\n", stderr);
        return APP_ERROR;
    }

    VkPhysicalDevice devices[device_count];
    vkEnumeratePhysicalDevices(app->instance, &device_count, devices);

    for (uint32_t i = 0; i < device_count; i++) {
        if (device_suitable(devices[i])) {
            app->physical_device = devices[i];
            break;
        }
    }

    if (!app->physical_device) {
        fputs("Error: failed to find a suitable GPU!\n", stderr);
        return APP_ERROR;
    }

    return APP_SUCCESS;
}
