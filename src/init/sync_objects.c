#include "init/sync_objects.h"
#include <stdio.h>

enum app_result create_sync_objects(struct application *app)
{
    const VkSemaphoreCreateInfo semaphore_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    const VkFenceCreateInfo fence_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(app->device, &semaphore_info, NULL, &app->image_available_semaphores[i])
            != VK_SUCCESS ||
            vkCreateSemaphore(app->device, &semaphore_info, NULL, &app->render_finished_semaphores[i])
            != VK_SUCCESS ||
            vkCreateFence(app->device, &fence_info, NULL, &app->in_flight_fences[i])
            != VK_SUCCESS)
        {
            fputs("Error: failed to create synchronization objects for a frame!\n", stderr);
            return APP_ERROR;
        }
    }

    return APP_SUCCESS;
}
