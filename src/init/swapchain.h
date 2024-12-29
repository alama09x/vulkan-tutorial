#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "application.h"

typedef struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    uint32_t format_count;
    VkSurfaceFormatKHR *pFormats;
    uint32_t presentModeCount;
    VkPresentModeKHR *pPresentModes;
} SwapchainSupportDetails;

SwapchainSupportDetails querySwapchainSupport(
    VkPhysicalDevice device,
    VkSurfaceKHR surface
);
void cleanupSwapchainSupport(SwapchainSupportDetails *pDetails);
AppResult createSwapchain(Application *pApp);
AppResult createImageViews(Application *pApp);

#endif
