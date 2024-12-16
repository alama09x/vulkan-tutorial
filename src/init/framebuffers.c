#include "init/framebuffers.h"
#include <stdio.h>
#include <stdlib.h>

AppResult createFramebuffers(Application *pApp)
{
    pApp->pSwapchainFramebuffers = malloc(pApp->swapchainImageCount * sizeof(VkFramebuffer));

    for (uint32_t i = 0; i < pApp->swapchainImageCount; i++) {
        const VkImageView attachments[] = {
            pApp->pSwapchainImageViews[i],
        };

        const VkFramebufferCreateInfo framebufferInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = pApp->renderPass,
            .attachmentCount = 1,
            .pAttachments = attachments,
            .width = pApp->swapchainExtent.width,
            .height = pApp->swapchainExtent.height,
            .layers = 1,
        };

        if (vkCreateFramebuffer(
            pApp->device, &framebufferInfo, NULL, &pApp->pSwapchainFramebuffers[i])
            != VK_SUCCESS)
        {
            fputs("Error: failed to create framebuffer!\n", stderr);
            return APP_ERROR;
        }
    }
    return APP_SUCCESS;
}
