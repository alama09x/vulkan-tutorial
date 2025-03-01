#ifndef DEVICE_H
#define DEVICE_H

#include "application.h"

#define QUEUE_FAMILY_INDICES_COUNT 3

typedef struct QueueFamilyIndices {
    uint32_t*       pGraphicsFamily;
    uint32_t*       pPresentFamily;
    uint32_t*       pTransferFamily;
} QueueFamilyIndices;

void findQueueFamilies(
    VkPhysicalDevice        device,
    VkSurfaceKHR            surface,
    QueueFamilyIndices*     pIndices);
void cleanupQueueFamilies(QueueFamilyIndices* pIndices);

AppResult pickPhysicalDevice(Application* pApp);
AppResult createLogicalDevice(Application* pApp);

#endif
