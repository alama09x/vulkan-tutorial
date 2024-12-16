#ifndef COMMANDS_H
#define COMMANDS_H

#include "application.h"

AppResult createCommandPool(Application *pApp);
AppResult createCommandBuffers(Application *pApp);
AppResult recordCommandBuffer(
    Application *pApp,
    VkCommandBuffer commandBuffer,
    uint32_t imageIndex);

#endif
