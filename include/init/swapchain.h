#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "application.h"

struct swapchain_support_details {
    VkSurfaceCapabilitiesKHR capabilities;
    uint32_t format_count;
    VkSurfaceFormatKHR *formats;
    uint32_t present_mode_count;
    VkPresentModeKHR *present_modes;
};

struct swapchain_support_details query_swapchain_support(
    VkPhysicalDevice device,
    VkSurfaceKHR surface);
void cleanup_swapchain_support(struct swapchain_support_details *details);
enum app_result create_swapchain(struct application *app);
enum app_result create_image_views(struct application *app);

#endif
