#include "init/device.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

static bool queue_family_indices_complete(const struct queue_family_indices *indices)
{
    return indices->graphics_family != NULL && indices->present_family != NULL;
}

static void find_queue_families(
    VkPhysicalDevice device,
    VkSurfaceKHR surface,
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

        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
        if (present_support) {
            *indices->present_family = i;
        }

        if (queue_family_indices_complete(indices)) {
            break;
        }
    }
}

static bool device_suitable(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    const struct queue_family_indices indices = {
        .graphics_family = malloc(sizeof(uint32_t)),
        .present_family = malloc(sizeof(uint32_t)),
    };
    find_queue_families(device, surface, &indices);

    bool result = queue_family_indices_complete(&indices);

    free(indices.graphics_family);
    free(indices.present_family);

    return result;
}

enum app_result create_logical_device(struct application *app)
{
    const struct queue_family_indices indices = {
        .graphics_family = malloc(sizeof(uint32_t)),        
        .present_family = malloc(sizeof(uint32_t)),
    };
    find_queue_families(app->physical_device, app->surface, &indices);

    uint32_t queue_families[] = { *indices.graphics_family, *indices.present_family };
    uint32_t *unique_queue_families = malloc(QUEUE_FAMILY_INDICES_COUNT * sizeof(uint32_t));
    uint32_t unique_count = 0;

    for (uint8_t i = 0; i < QUEUE_FAMILY_INDICES_COUNT; i++) {
        bool unique = true;
        for (uint8_t j = 0; j < unique_count; j++) {
            if (queue_families[i] == unique_queue_families[j]) {
                unique = false;
                break;
            }
        }

        if (unique) {
            unique_queue_families[unique_count++] = queue_families[i];
        }
    }

    VkDeviceQueueCreateInfo queue_create_infos[unique_count];
    const float queue_priority = 1.0;

    for (uint8_t i = 0; i < unique_count; i++) {
        const VkDeviceQueueCreateInfo queue_create_info = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = unique_queue_families[i],
            .queueCount = 1,
            .pQueuePriorities = &queue_priority,
        };
        queue_create_infos[i] = queue_create_info;
    }

    const VkPhysicalDeviceFeatures device_features = {};

    VkDeviceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos = queue_create_infos,
        .queueCreateInfoCount = unique_count,
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
    vkGetDeviceQueue(app->device, *indices.present_family, 0, &app->present_queue);

    free(unique_queue_families);
    free(indices.graphics_family);
    free(indices.present_family);
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
        if (device_suitable(devices[i], app->surface)) {
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
