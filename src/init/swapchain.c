#include "init/swapchain.h"
#include "init/device.h"
#include "utility.h"
#include <stdlib.h>
#include <stdio.h>

static VkSurfaceFormatKHR choose_swapchain_surface_format(
    const uint32_t available_format_count,
    const VkSurfaceFormatKHR *available_formats)
{
    for (uint32_t i = 0; i < available_format_count; i++) {
        if (available_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            available_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return available_formats[i];
        }
    }

    return available_formats[0];
}

static VkPresentModeKHR choose_swapchain_present_mode(
    const uint32_t available_present_mode_count,
    const VkPresentModeKHR *available_present_modes)
{
    for (uint32_t i = 0; i < available_present_mode_count; i++) {
        if (available_present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return available_present_modes[i];
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D choose_swapchain_extent(
    GLFWwindow *window, const VkSurfaceCapabilitiesKHR *capabilities)
{
    if (capabilities->currentExtent.width != UINT32_MAX) {
        return capabilities->currentExtent;
    } else {
        int32_t width, height;
        glfwGetFramebufferSize(window, &width, &height);

        clamp(&width, capabilities->minImageExtent.width, capabilities->maxImageExtent.width);
        clamp(&height, capabilities->minImageExtent.height, capabilities->maxImageExtent.height);

        const VkExtent2D actual_extent = {
            .width = (uint32_t)width,
            .height = (uint32_t)height,
        };
        return actual_extent;
    }
}

struct swapchain_support_details query_swapchain_support(
    VkPhysicalDevice device,
    VkSurfaceKHR surface)
{
    struct swapchain_support_details details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, NULL);
    details.format_count = format_count;

    if (format_count != 0) {
        details.formats = malloc(format_count * sizeof(*details.formats));
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, details.formats);
    }

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, NULL);
    details.present_mode_count = present_mode_count;

    if (present_mode_count != 0) {
        details.present_modes = malloc(present_mode_count * sizeof(*details.present_modes));
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            device,
            surface,
            &present_mode_count,
            details.present_modes);
    }

    return details;
}

void cleanup_swapchain_support(struct swapchain_support_details *details)
{
        if (details->formats) {
            free(details->formats);
        }
        if (details->present_modes) {
            free(details->present_modes);
        }
}

enum app_result create_swapchain(struct application *app)
{
    struct swapchain_support_details swapchain_support =
        query_swapchain_support(app->physical_device, app->surface);

    const VkSurfaceFormatKHR surface_format = choose_swapchain_surface_format(
        swapchain_support.format_count,
        swapchain_support.formats);

    const VkPresentModeKHR present_mode = choose_swapchain_present_mode(
        swapchain_support.present_mode_count,
        swapchain_support.present_modes);

    const VkExtent2D extent = choose_swapchain_extent(
        app->window, &swapchain_support.capabilities);

    uint32_t image_count = swapchain_support.capabilities.minImageCount + 1;
    if (swapchain_support.capabilities.maxImageCount > 0 &&
        image_count > swapchain_support.capabilities.maxImageCount)
        image_count = swapchain_support.capabilities.maxImageCount;

    struct queue_family_indices indices;
    find_queue_families(app->physical_device, app->surface, &indices);
    uint32_t queue_family_indices[] = { *indices.graphics_family, *indices.present_family };

    VkSwapchainCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = app->surface,
        .minImageCount = image_count,
        .imageFormat = surface_format.format,
        .imageColorSpace = surface_format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = swapchain_support.capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = present_mode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };
    if (*indices.graphics_family != *indices.present_family) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    if (vkCreateSwapchainKHR(app->device, &create_info, NULL, &app->swapchain) != VK_SUCCESS) {
        fputs("Error: failed to create swapchain!\n", stderr);
        return APP_ERROR;
    }

    vkGetSwapchainImagesKHR(app->device, app->swapchain, &app->swapchain_image_count, NULL);
    app->swapchain_images = malloc(app->swapchain_image_count * sizeof(VkImage));

    vkGetSwapchainImagesKHR(
        app->device, app->swapchain, &app->swapchain_image_count, app->swapchain_images);

    app->swapchain_image_format = surface_format.format;
    app->swapchain_extent = extent;

    cleanup_swapchain_support(&swapchain_support);
    cleanup_queue_families(&indices);

    return APP_SUCCESS;
}

enum app_result create_image_views(struct application *app)
{
    app->swapchain_image_views = malloc(app->swapchain_image_count * sizeof(VkImageView));
    for (uint32_t i = 0; i < app->swapchain_image_count; i++) {
        const VkImageViewCreateInfo create_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = app->swapchain_images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = app->swapchain_image_format,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };
        if (vkCreateImageView(app->device, &create_info, NULL, &app->swapchain_image_views[i]) !=
            VK_SUCCESS)
        {
            fputs("Error: failed to create image views!\n", stderr);
            return APP_ERROR;
        }
    }
    return APP_SUCCESS;
}
