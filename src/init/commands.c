#include "init/commands.h"
#include "application.h"
#include "init/device.h"
#include "init/vertex.h"
#include <stdio.h>

AppResult createCommandPool(Application *pApp, VkCommandPool *pCommandPool, uint32_t queueFamilyIndex)
{
    QueueFamilyIndices queueFamilyIndices;
    findQueueFamilies(pApp->physicalDevice, pApp->surface, &queueFamilyIndices);

    const VkCommandPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queueFamilyIndex,
    };

    if (vkCreateCommandPool(pApp->device, &poolInfo, NULL, pCommandPool)
        != VK_SUCCESS)
    {
        fputs("Error: failed to create command pool!\n", stderr);
        return APP_ERROR;
    }
    return APP_SUCCESS;
}

AppResult createCommandBuffers(Application *pApp)
{
    const VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = pApp->graphicsCommandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = MAX_FRAMES_IN_FLIGHT,
    };

    if (vkAllocateCommandBuffers(pApp->device, &allocInfo, pApp->commandBuffers)
        != VK_SUCCESS)
    {
        fputs("Error: failure to allocate command buffers!\n", stderr);
    }


    return APP_SUCCESS;
}

AppResult recordCommandBuffer(
    Application *pApp,
    VkCommandBuffer commandBuffer,
    uint32_t imageIndex)
{
    const VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = 0,
        .pInheritanceInfo = NULL, // optional
    };

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        fputs("Error: failed to begin recording command buffer!\n", stderr);
        return APP_ERROR;
    }

    const VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 0.0f};

    const VkRenderPassBeginInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = pApp->renderPass,
        .framebuffer = pApp->pSwapchainFramebuffers[imageIndex],
        .renderArea = {
            .offset = { .x = 0, .y = 0 },
            .extent = pApp->swapchainExtent,
        },
        .clearValueCount = 1,
        .pClearValues = &clearColor,
    };

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(
        commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pApp->graphicsPipeline);

    const VkBuffer vertexBuffers[] = { pApp->vertexBuffer };
    const VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, pApp->indexBuffer, 0, VK_INDEX_TYPE_UINT16);

    const VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float)pApp->swapchainExtent.width,
        .height = (float)pApp->swapchainExtent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    const VkRect2D scissor = {
        .offset = { .x = 0, .y = 0 },
        .extent = pApp->swapchainExtent,
    };
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdDrawIndexed(commandBuffer, INDEX_COUNT, 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        fputs("Error: failed to record command buffer!\n", stderr);
        return APP_ERROR;
    }

    return APP_SUCCESS;
}
