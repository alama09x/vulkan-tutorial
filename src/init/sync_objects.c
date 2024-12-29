#include "sync_objects.h"

#include <stdio.h>

AppResult createSyncObjects(Application *app)
{
    const VkSemaphoreCreateInfo semaphoreInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    const VkFenceCreateInfo fenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (
            vkCreateSemaphore(
                app->device,
                &semaphoreInfo,
                NULL,
                &app->imageAvailableSemaphores[i]
            ) != VK_SUCCESS ||
            vkCreateSemaphore(
                app->device,
                &semaphoreInfo,
                NULL,
                &app->renderFinishedSemaphores[i]
            ) != VK_SUCCESS ||
            vkCreateFence(
                app->device,
                &fenceInfo,
                NULL, &app->inFlightFences[i]
            ) != VK_SUCCESS
        ) {
            APP_ERROR("failed to create synchronization objects for a frame");
        }
    }

    return APP_SUCCESS;
}
