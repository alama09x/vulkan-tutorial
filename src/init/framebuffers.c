#include "init/framebuffers.h"
#include <stdio.h>
#include <stdlib.h>

enum app_result create_framebuffers(struct application *app)
{
    app->swapchain_framebuffers = malloc(app->swapchain_image_count * sizeof(VkFramebuffer));

    for (uint32_t i = 0; i < app->swapchain_image_count; i++) {
        const VkImageView attachments[] = {
            app->swapchain_image_views[i],
        };

        const VkFramebufferCreateInfo framebuffer_info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = app->render_pass,
            .attachmentCount = 1,
            .pAttachments = attachments,
            .width = app->swapchain_extent.width,
            .height = app->swapchain_extent.height,
            .layers = 1,
        };

        if (vkCreateFramebuffer(
            app->device, &framebuffer_info, NULL, &app->swapchain_framebuffers[i])
            != VK_SUCCESS)
        {
            fputs("Error: failed to create framebuffer!\n", stderr);
            return APP_ERROR;
        }
    }
    return APP_SUCCESS;
}
