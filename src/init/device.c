#include "init/device.h"
#include "init/validation.h"
#include "init/swapchain.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const uint8_t DEVICE_EXTENSION_COUNT = 1;
const char *const DEVICE_EXTENSIONS[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

static bool queue_family_indices_complete(const struct queue_family_indices *indices)
{
    return indices->graphics_family != NULL && indices->present_family != NULL;
}

void find_queue_families(
    VkPhysicalDevice device,
    VkSurfaceKHR surface,
    struct queue_family_indices *indices)
{
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, NULL);

    VkQueueFamilyProperties queue_families[queue_family_count];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);

    for (uint32_t i = 0; i < queue_family_count; i++) {
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices->graphics_family = malloc(sizeof(uint32_t));
            *indices->graphics_family = i;
        }

        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
        if (present_support) {
            indices->present_family = malloc(sizeof(uint32_t));
            *indices->present_family = i;
        }

        if (queue_family_indices_complete(indices)) {
            break;
        }
    }
}

void cleanup_queue_families(struct queue_family_indices *indices)
{
    free(indices->graphics_family);
    free(indices->present_family);
}

static bool check_device_extension_support(VkPhysicalDevice device)
{
    uint32_t ext_count;
    vkEnumerateDeviceExtensionProperties(device, NULL, &ext_count, NULL);

    VkExtensionProperties available_extensions[ext_count];
    vkEnumerateDeviceExtensionProperties(device, NULL, &ext_count, available_extensions);

    puts("Available device extensions:");
    for (int i = 0; i < ext_count; i++) {
        printf("\t%s\n", available_extensions[i].extensionName);
    }

    for (uint8_t i = 0; i < DEVICE_EXTENSION_COUNT; i++) {
        bool extension_found = false;

        for (uint8_t j = 0; j < ext_count; j++) {
            if (strcmp(DEVICE_EXTENSIONS[i], available_extensions[j].extensionName) == 0) {
                extension_found = true;
                break;
            }
        }

        if (!extension_found) {
            return false;
        }
    }

    return true;
}

static bool device_suitable(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    struct queue_family_indices indices;
    find_queue_families(device, surface, &indices);

    bool indices_complete = queue_family_indices_complete(&indices);
    bool extensions_supported = check_device_extension_support(device);

    bool swapchain_adequate = false;
    if (extensions_supported) {
        struct swapchain_support_details swapchain_support =
            query_swapchain_support(device, surface);

        swapchain_adequate = swapchain_support.formats != NULL &&
            swapchain_support.present_modes != NULL;

        cleanup_swapchain_support(&swapchain_support);
    }

    cleanup_queue_families(&indices);

    return indices_complete & extensions_supported && swapchain_adequate;
}

enum app_result create_logical_device(struct application *app)
{
    struct queue_family_indices indices;
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
        .enabledExtensionCount = DEVICE_EXTENSION_COUNT,
        .ppEnabledExtensionNames = DEVICE_EXTENSIONS,
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
    cleanup_queue_families(&indices);
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
