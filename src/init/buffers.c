#include "buffers.h"

#include "data/vertex.h"
#include "data/index.h"
#include "data/uniform.h"

#include "device.h"
#include <stdio.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

/// Find ideal memory type
static AppResult findMemoryType(
    VkPhysicalDevice            physicalDevice,
    uint32_t                    typeFilter,
    VkMemoryPropertyFlags       properties,
    uint32_t*                   pMemoryType)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        // Check if all required memory properties are included by iteratively checking each bit
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

/// Creates a buffer and allocates buffer memory
/// Must clean up `pBuffer` and `pBufferMemory`
AppResult createBuffer(
    Application*                pApp,
    VkDeviceSize                size,
    VkBufferUsageFlags          usage,
    VkMemoryPropertyFlags       properties,
    VkBuffer*                   pBuffer,
    VkDeviceMemory*             pBufferMemory)
{
    QueueFamilyIndices indices;
    findQueueFamilies(pApp->physicalDevice, pApp->surface, &indices);

    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .queueFamilyIndexCount = 2,
    };

    if (*indices.pGraphicsFamily == *indices.pTransferFamily) {
        // Unified queue family
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.queueFamilyIndexCount = 1,
        bufferInfo.pQueueFamilyIndices = indices.pGraphicsFamily;
    } else {
        // Separate queue families
        bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
        bufferInfo.queueFamilyIndexCount = 2,
        bufferInfo.pQueueFamilyIndices = (uint32_t[]) {
            *indices.pGraphicsFamily,
            *indices.pTransferFamily,
        };
    }

    APP_EXPECT(
        vkCreateBuffer(pApp->device, &bufferInfo, NULL, pBuffer),
        "failed to allocate buffer memory"
    );

    cleanupQueueFamilies(&indices);
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(pApp->device, *pBuffer, &memRequirements);

    uint32_t memoryTypeIndex;
    findMemoryType(pApp->physicalDevice, memRequirements.memoryTypeBits, properties, &memoryTypeIndex);

    const VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = memoryTypeIndex,
    };

    APP_EXPECT(
        vkAllocateMemory(pApp->device, &allocInfo, NULL, pBufferMemory),
        "failed to allocate buffer memory"
    );

    vkBindBufferMemory(pApp->device, *pBuffer, *pBufferMemory, 0);
    return APP_SUCCESS;
}

AppResult copyBuffer(
    Application*    pApp,
    VkBuffer        srcBuffer,
    VkBuffer        dstBuffer,
    VkDeviceSize    size)
{
    const VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = pApp->transferCommandPool,
        .commandBufferCount = 1,
    };

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(pApp->device, &allocInfo, &commandBuffer);

    const VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    // Submit buffer copy command to command buffer
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    const VkBufferCopy copyRegion = { .srcOffset = 0, .dstOffset = 0, .size = size };
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    const VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
    };

    vkQueueSubmit(pApp->transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(pApp->transferQueue);

    vkFreeCommandBuffers(pApp->device, pApp->transferCommandPool, 1, &commandBuffer);

    return APP_SUCCESS;
}

/// Creates the vertex buffer and copies into it the contents of `VERTICES` through a staging buffer
/// Must clean up `pApp->vertexBuffer`
AppResult createVertexBuffer(Application* pApp)
{
    const VkDeviceSize bufferSize = sizeof(VERTICES[0]) * VERTEX_COUNT;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    APP_EXPECT(
        createBuffer(
            pApp,
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &stagingBuffer,
            &stagingBufferMemory
        ),
        "failed to create staging buffer"
    );
    void *data;
    vkMapMemory(pApp->device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, VERTICES, (size_t) bufferSize);
    vkUnmapMemory(pApp->device, stagingBufferMemory);

    APP_EXPECT(
        createBuffer(
            pApp,
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &pApp->vertexBuffer,
            &pApp->vertexBufferMemory
        ),
        "failed to create vertex buffer"
    );

    copyBuffer(pApp, stagingBuffer, pApp->vertexBuffer, bufferSize);

    vkDestroyBuffer(pApp->device, stagingBuffer, NULL);
    vkFreeMemory(pApp->device, stagingBufferMemory, NULL);

    return APP_SUCCESS;
}

/// Creates the index buffer and copies into it the contents of `INDICES` through a staging buffer
/// Must clean up `pApp->indexBuffer`
AppResult createIndexBuffer(Application* pApp)
{
    const VkDeviceSize bufferSize = sizeof(INDICES[0]) * INDEX_COUNT;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    APP_EXPECT(
        createBuffer(
            pApp,
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &stagingBuffer,
            &stagingBufferMemory
        ),
        "failed to create staging buffer"
    );

    void *data;
    vkMapMemory(pApp->device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, INDICES, (size_t) bufferSize);
    vkUnmapMemory(pApp->device, stagingBufferMemory);

    APP_EXPECT(
        createBuffer(
            pApp,
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &pApp->indexBuffer,
            &pApp->indexBufferMemory
        ),
        "failed to create index buffer"
    );

    copyBuffer(pApp, stagingBuffer, pApp->indexBuffer, bufferSize);

    vkDestroyBuffer(pApp->device, stagingBuffer, NULL);
    vkFreeMemory(pApp->device, stagingBufferMemory, NULL);

    return APP_SUCCESS;
}

/// Creates a uniform buffer for each frame in flight
/// Must clean up `pApp->uniformBuffersMemory` and `pApp->uniformBuffersMapped`
AppResult createUniformBuffers(Application* pApp)
{
    const VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        createBuffer(
            pApp,
            bufferSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &pApp->uniformBuffers[i],
            &pApp->uniformBuffersMemory[i]
        );

        vkMapMemory(
            pApp->device,
            pApp->uniformBuffersMemory[i],
            0,
            bufferSize, 0,
            &pApp->uniformBuffersMapped[i]
        );
    }

    return APP_SUCCESS;
}