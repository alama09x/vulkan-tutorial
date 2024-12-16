#include "init/swapchain.h"
#include "init/device.h"
#include "utility.h"
#include <stdlib.h>
#include <stdio.h>

static VkSurfaceFormatKHR chooseSwapchainSurfaceFormat(
    const uint32_t availableFormatCount,
    const VkSurfaceFormatKHR *pAvailableFormats)
{
    for (uint32_t i = 0; i < availableFormatCount; i++) {
        if (pAvailableFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            pAvailableFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return pAvailableFormats[i];
        }
    }

    return pAvailableFormats[0];
}

static VkPresentModeKHR chooseSwapchainPresentMode(
    const uint32_t availablePresentModeCount,
    const VkPresentModeKHR *pAvailablePresentModes)
{
    for (uint32_t i = 0; i < availablePresentModeCount; i++) {
        if (pAvailablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return pAvailablePresentModes[i];
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D chooseSwapchainExtent(
    GLFWwindow *window, const VkSurfaceCapabilitiesKHR *capabilities)
{
    if (capabilities->currentExtent.width != UINT32_MAX) {
        return capabilities->currentExtent;
    } else {
        int32_t width, height;
        glfwGetFramebufferSize(window, &width, &height);

        clamp(&width, capabilities->minImageExtent.width, capabilities->maxImageExtent.width);
        clamp(&height, capabilities->minImageExtent.height, capabilities->maxImageExtent.height);

        const VkExtent2D actualExtent = {
            .width = (uint32_t)width,
            .height = (uint32_t)height,
        };
        return actualExtent;
    }
}

SwapchainSupportDetails querySwapchainSupport(
    VkPhysicalDevice device,
    VkSurfaceKHR surface)
{
    SwapchainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, NULL);
    details.format_count = formatCount;

    if (formatCount != 0) {
        details.pFormats = malloc(formatCount * sizeof(*details.pFormats));
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.pFormats);
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, NULL);
    details.presentModeCount = presentModeCount;

    if (presentModeCount != 0) {
        details.pPresentModes = malloc(presentModeCount * sizeof(*details.pPresentModes));
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            device,
            surface,
            &presentModeCount,
            details.pPresentModes);
    }

    return details;
}

void cleanupSwapchainSupport(SwapchainSupportDetails *pDetails)
{
        if (pDetails->pFormats) {
            free(pDetails->pFormats);
        }
        if (pDetails->pPresentModes) {
            free(pDetails->pPresentModes);
        }
}

AppResult createSwapchain(Application *pApp)
{
    SwapchainSupportDetails swapchainSupport =
        querySwapchainSupport(pApp->physicalDevice, pApp->surface);

    const VkSurfaceFormatKHR surfaceFormat = chooseSwapchainSurfaceFormat(
        swapchainSupport.format_count,
        swapchainSupport.pFormats);

    const VkPresentModeKHR presentMode = chooseSwapchainPresentMode(
        swapchainSupport.presentModeCount,
        swapchainSupport.pPresentModes);

    const VkExtent2D extent = chooseSwapchainExtent(
        pApp->pWindow, &swapchainSupport.capabilities);

    uint32_t image_count = swapchainSupport.capabilities.minImageCount + 1;
    if (swapchainSupport.capabilities.maxImageCount > 0 &&
        image_count > swapchainSupport.capabilities.maxImageCount)
        image_count = swapchainSupport.capabilities.maxImageCount;

    QueueFamilyIndices indices;
    findQueueFamilies(pApp->physicalDevice, pApp->surface, &indices);
    uint32_t queueFamilyIndices[] = { *indices.pGraphicsFamily, *indices.pPresentFamily };

    VkSwapchainCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = pApp->surface,
        .minImageCount = image_count,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = swapchainSupport.capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };
    if (*indices.pGraphicsFamily != *indices.pPresentFamily) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    if (vkCreateSwapchainKHR(pApp->device, &create_info, NULL, &pApp->swapchain) != VK_SUCCESS) {
        fputs("Error: failed to create swapchain!\n", stderr);
        return APP_ERROR;
    }

    vkGetSwapchainImagesKHR(pApp->device, pApp->swapchain, &pApp->swapchainImageCount, NULL);
    pApp->pSwapchainImages = malloc(pApp->swapchainImageCount * sizeof(VkImage));

    vkGetSwapchainImagesKHR(
        pApp->device, pApp->swapchain, &pApp->swapchainImageCount, pApp->pSwapchainImages);

    pApp->swapchainImageFormat = surfaceFormat.format;
    pApp->swapchainExtent = extent;

    cleanupSwapchainSupport(&swapchainSupport);
    cleanupQueueFamilies(&indices);

    return APP_SUCCESS;
}

AppResult createImageViews(Application *app)
{
    app->pSwapchainImageViews = malloc(app->swapchainImageCount * sizeof(VkImageView));
    for (uint32_t i = 0; i < app->swapchainImageCount; i++) {
        const VkImageViewCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = app->pSwapchainImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = app->swapchainImageFormat,
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
        if (vkCreateImageView(app->device, &createInfo, NULL, &app->pSwapchainImageViews[i]) !=
            VK_SUCCESS)
        {
            fputs("Error: failed to create image views!\n", stderr);
            return APP_ERROR;
        }
    }
    return APP_SUCCESS;
}
