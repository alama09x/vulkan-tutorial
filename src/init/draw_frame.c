#include "init/draw_frame.h"
#include "init/swapchain.h"
#include "init/framebuffers.h"
#include "init/commands.h"

#include <stdio.h>
#include <stdlib.h>

AppResult cleanupSwapchain(Application *pApp)
{
    if (pApp->pSwapchainFramebuffers) {
        for (uint32_t i = 0; i < pApp->swapchainImageCount; i++) {
            vkDestroyFramebuffer(pApp->device, pApp->pSwapchainFramebuffers[i], NULL);
        }
        free(pApp->pSwapchainFramebuffers);
    }

    for (uint32_t i = 0; i < pApp->swapchainImageCount; i++) {
        vkDestroyImageView(pApp->device, pApp->pSwapchainImageViews[i], NULL);
    }

    vkDestroySwapchainKHR(pApp->device, pApp->swapchain, NULL);

    free(pApp->pSwapchainImageViews);
    free(pApp->pSwapchainImages);

    return APP_SUCCESS;
}

static AppResult recreateSwapchain(Application *pApp)
{
    int32_t width = 0, height = 0;
    glfwGetFramebufferSize(pApp->pWindow, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(pApp->pWindow, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(pApp->device);
    AppResult result = APP_SUCCESS;
    if ((result = cleanupSwapchain(pApp)) != APP_SUCCESS) {
        fputs("Error: failed to cleanup swapchain!\n", stderr);
        return result;
    }

    if ((result = createSwapchain(pApp)) != APP_SUCCESS) {
        fputs("Error: failed to create swapchain!\n", stderr);
        return result;
    }

    if ((result = createImageViews(pApp)) != APP_SUCCESS) {
        fputs("Error: failed to create image views!\n", stderr);
        return result;
    }

    if ((result = createFramebuffers(pApp)) != APP_SUCCESS) {
        fputs("Error: failed to create framebuffers!\n", stderr);
        return result;
    }

    return APP_SUCCESS;
}

AppResult drawFrame(Application *pApp, uint32_t currentFrame)
{
    vkWaitForFences(pApp->device, 1, &pApp->inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        pApp->device,
        pApp->swapchain,
        UINT64_MAX,
        pApp->imageAvailableSemaphores[currentFrame],
        VK_NULL_HANDLE,
        &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapchain(pApp);
        return APP_SUCCESS;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        fputs("Error: failed to acquire swap chain image!", stderr);
    }

    vkResetFences(pApp->device, 1, &pApp->inFlightFences[currentFrame]);

    vkResetCommandBuffer(pApp->commandBuffers[currentFrame], 0);
    recordCommandBuffer(pApp, pApp->commandBuffers[currentFrame], imageIndex);

    const VkSemaphore waitSemaphores[] = {
        pApp->imageAvailableSemaphores[currentFrame],
    };
    const VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    };
    const VkSemaphore signalSemaphores[] = {
        pApp->renderFinishedSemaphores[currentFrame],
    };

    const VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores, // corresponds with below
        .pWaitDstStageMask = waitStages,   // corresponds with above
        .commandBufferCount = 1,
        .pCommandBuffers = &pApp->commandBuffers[currentFrame],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signalSemaphores,
    };

    if (vkQueueSubmit(pApp->graphicsQueue, 1, &submitInfo, pApp->inFlightFences[currentFrame])
        != VK_SUCCESS)
    {
        fputs("Error: failed to submit draw command buffer!\n", stderr);
        return APP_ERROR;
    }

    const VkSwapchainKHR swapchains[] = { pApp->swapchain };

    const VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores,
        .swapchainCount = 1,
        .pSwapchains = swapchains,
        .pImageIndices = &imageIndex,
        .pResults = NULL,
    };

    result = vkQueuePresentKHR(pApp->presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        pApp->framebufferResized) {
        pApp->framebufferResized = false;
        recreateSwapchain(pApp);
    } else if (result != VK_SUCCESS) {
        fputs("Error: failed to present swapchain image!\n", stderr);
    }

    return APP_SUCCESS;
}

