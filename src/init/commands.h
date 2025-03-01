#ifndef COMMANDS_H
#define COMMANDS_H

#include "application.h"

AppResult createCommandPool(Application* pApp, VkCommandPool* pCommandPool, uint32_t queueFamilyIndex);
AppResult createCommandBuffers(Application* pApp);
AppResult recordCommandBuffer(
    Application*        pApp,
    VkCommandBuffer     commandBuffer,
    uint32_t            imageIndex,
    uint32_t            currentFrame);

#endif
