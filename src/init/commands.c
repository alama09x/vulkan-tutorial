#include "commands.h"

#include "data/index.h"
#include "device.h"

#include <stdio.h>

/// Must clean up `pCommandPool`
AppResult createCommandPool(
    Application *       pApp,
    VkCommandPool*      pCommandPool,
    uint32_t            queueFamilyIndex)
{
    QueueFamilyIndices queueFamilyIndices;
    findQueueFamilies(pApp->physicalDevice, pApp->surface, &queueFamilyIndices);

    const VkCommandPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queueFamilyIndex,
    };

    APP_EXPECT(
        vkCreateCommandPool(pApp->device, &poolInfo, NULL, pCommandPool),
        "failed to create command pool"
    );

    return APP_SUCCESS;
}

/// Must clean up `pApp->commandBuffers`
AppResult createCommandBuffers(Application *pApp)
{
    const VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = pApp->graphicsCommandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = MAX_FRAMES_IN_FLIGHT,
    };

    APP_EXPECT(
        vkAllocateCommandBuffers(pApp->device, &allocInfo, pApp->commandBuffers),
        "failure to allocate command buffers"
    );

    return APP_SUCCESS;
}

/// Perform operations on `commandBuffer` on `currentFrame` to the framebuffer of `imageIndex`
AppResult recordCommandBuffer(
    Application*        pApp,
    VkCommandBuffer     commandBuffer,
    uint32_t            imageIndex,
    uint32_t            currentFrame)
{
    const VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = 0,
        .pInheritanceInfo = NULL, // optional
    };

    APP_EXPECT(
        vkBeginCommandBuffer(commandBuffer, &beginInfo),
        "failed to begin recording command buffer"
    );

    // To clear background
    const VkClearValue clearColor = {
        .color = {
            .float32 = { 0.0f, 0.0f, 0.0f, 0.0f },
        },
    };

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

    // Use graphics pipeline
    vkCmdBindPipeline(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pApp->graphicsPipeline
    );

    // One vertex buffer
    const VkBuffer vertexBuffers[] = { pApp->vertexBuffer };

    // No buffer offset
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

    vkCmdBindDescriptorSets(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pApp->pipelineLayout,
        0,
        1,
        &pApp->descriptorSets[currentFrame],
        0,
        NULL
    );

    vkCmdDrawIndexed(commandBuffer, INDEX_COUNT, 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    APP_EXPECT(vkEndCommandBuffer(commandBuffer), "failed to record command buffer");

    return APP_SUCCESS;
}
