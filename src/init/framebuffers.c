#include "framebuffers.h"
#include <stdio.h>
#include <stdlib.h>

/// Must clean up `pApp->pSwapchainBuffers`
AppResult createFramebuffers(Application* pApp)
{
    pApp->pSwapchainFramebuffers =
        malloc(pApp->swapchainImageCount * sizeof(VkFramebuffer));

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

        APP_EXPECT(
            vkCreateFramebuffer(
                pApp->device,
                &framebufferInfo,
                NULL,
                &pApp->pSwapchainFramebuffers[i]),
            "failed to create framebuffer"
        );
    }
    return APP_SUCCESS;
}
