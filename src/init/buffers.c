#include "init/buffers.h"

#include "init/vertex.h"
#include <stdio.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

static AppResult findMemoryType(
    VkPhysicalDevice physicalDevice,
    uint32_t typeFilter,
    VkMemoryPropertyFlags properties,
    uint32_t *pMemoryType)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags &
            properties) == properties)
        {
            *pMemoryType = i;
            return APP_SUCCESS;
        }
    }

    fputs("Error: failed to find suitable memory type!\n", stderr);
    return APP_ERROR;
}

AppResult createVertexBuffer(Application *pApp)
{
    const VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = VERTEX_COUNT * sizeof(Vertex),
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    if (vkCreateBuffer(pApp->device, &bufferInfo, NULL, &pApp->vertexBuffer) !=
        VK_SUCCESS)
    {
        fputs("Error: failed to create vertex buffer!\n", stderr);
        return APP_ERROR;
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(pApp->device, pApp->vertexBuffer, &memRequirements);

    uint32_t memoryType;
    if (findMemoryType(
        pApp->physicalDevice,
        memRequirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &memoryType) != VK_SUCCESS)
    {
        return APP_ERROR;
    };

    const VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = memoryType,
    };

    if (vkAllocateMemory(pApp->device, &allocInfo, NULL, &pApp->vertexBufferMemory)
        != VK_SUCCESS)
    {
        fputs("Error: failed to allocate vertex buffer memory!\n", stderr);
        return APP_ERROR;
    }

    vkBindBufferMemory(pApp->device, pApp->vertexBuffer, pApp->vertexBufferMemory, 0);

    void *data;
    vkMapMemory(pApp->device, pApp->vertexBufferMemory, 0, bufferInfo.size, 0, &data);
    memcpy(data, VERTICES, (size_t)bufferInfo.size);
    vkUnmapMemory(pApp->device, pApp->vertexBufferMemory);

    return APP_SUCCESS;
}
