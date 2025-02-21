#include "draw_frame.h"
#include "data/uniform.h"
#include "swapchain.h"
#include "framebuffers.h"
#include "commands.h"
#include <string.h>

#define CGLM_FORCE_RADIANS
#include <stdio.h>
#include <stdlib.h>
#include <cglm/cglm.h>

AppResult cleanupSwapchain(Application *pApp)
{
    if (pApp->pSwapchainFramebuffers) {
        for (uint32_t i = 0; i < pApp->swapchainImageCount; i++) {
            vkDestroyFramebuffer(
                pApp->device,
                pApp->pSwapchainFramebuffers[i], NULL
            );
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

    APP_EXPECT(cleanupSwapchain(pApp), "failed to cleanup swapchain");
    APP_EXPECT(createSwapchain(pApp), "failed to create swapchain");
    APP_EXPECT(createImageViews(pApp), "failed to create image views");
    APP_EXPECT(createFramebuffers(pApp), "failed to create framebuffers");

    return APP_SUCCESS;
}

static void updateUniformBuffer(
    Application *pApp,
    uint32_t currentImage,
    const time_t *pStartTime
) {
    const time_t currentTime = time(NULL);
    const double time = difftime(currentTime, *pStartTime);

    UniformBufferObject ubo;
    glm_mat4_copy(GLM_MAT4_IDENTITY, ubo.model);

    glm_rotate(
        ubo.model,
        time * glm_rad(90.0f),
        (vec3){ 0.0f, 0.0f, 1.0f, }
    );

    glm_lookat(
        (vec3){ 2.0f, 2.0f, 2.0f },
        (vec3){ 0.0f, 0.0f, 0.0f },
        (vec3){ 0.0f, 0.0f, 1.0f },
        ubo.view
    );

    glm_perspective(
        glm_rad(45.0f),
        pApp->swapchainExtent.width / (float)pApp->swapchainExtent.height,
        0.1f,
        10.0f,
        ubo.proj
    );
    ubo.proj[1][1] *= -1;

    memcpy(pApp->uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

AppResult drawFrame(Application *pApp, uint32_t currentFrame, const time_t *pStartTime)
{
    vkWaitForFences(
        pApp->device,
        1,
        &pApp->inFlightFences[currentFrame],
        VK_TRUE,
        UINT64_MAX
    );

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        pApp->device,
        pApp->swapchain,
        UINT64_MAX,
        pApp->imageAvailableSemaphores[currentFrame],
        VK_NULL_HANDLE,
        &imageIndex
    );

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapchain(pApp);
        return APP_SUCCESS;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        APP_ERROR_MSG("failed to acquire swapchain image!");
    }

    vkResetFences(pApp->device, 1, &pApp->inFlightFences[currentFrame]);

    vkResetCommandBuffer(pApp->commandBuffers[currentFrame], 0);
    recordCommandBuffer(
        pApp,
        pApp->commandBuffers[currentFrame],
        imageIndex,
        currentFrame
    );

    const VkSemaphore waitSemaphores[] = {
        pApp->imageAvailableSemaphores[currentFrame],
    };
    const VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    };
    const VkSemaphore signalSemaphores[] = {
        pApp->renderFinishedSemaphores[currentFrame],
    };

    updateUniformBuffer(pApp, currentFrame, pStartTime);

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

    APP_EXPECT(
        vkQueueSubmit(
            pApp->graphicsQueue,
            1,
            &submitInfo,
            pApp->inFlightFences[currentFrame]
        ),
        "failed to submit draw command buffer"
    );

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
        APP_ERROR_MSG("failed to present swapchain image");
    }

    return APP_SUCCESS;
}

