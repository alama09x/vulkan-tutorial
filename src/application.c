#include "application.h"

#include "init/instance.h"
#include "init/device.h"
#include "init/validation.h"
#include "init/swapchain.h"
#include "init/render_pass.h"
#include "init/graphics_pipeline.h"
#include "init/framebuffers.h"
#include "init/commands.h"
#include "init/sync_objects.h"
#include "init/draw_frame.h"
#include "init/buffers.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#ifdef NDEBUG
const bool ENABLE_VALIDATION_LAYERS = false;
#else
const bool ENABLE_VALIDATION_LAYERS = true;
#endif

static const uint32_t WIDTH = 800;
static const uint32_t HEIGHT = 600;

static AppResult createSurface(Application *pApp)
{
    if (glfwCreateWindowSurface(pApp->instance, pApp->pWindow, NULL, &pApp->surface)
        != VK_SUCCESS)
    {
        fputs("Error: failed to create window surface!\n", stderr);
        return APP_ERROR;
    }

    return APP_SUCCESS;
}

static void framebufferResizeCallback(GLFWwindow *pWindow, int width, int height)
{
    Application *pApp = (Application *)glfwGetWindowUserPointer(pWindow);
    pApp->framebufferResized = true;
}

static AppResult initWindow(Application *pApp)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    if (!(pApp->pWindow = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", NULL, NULL))) {
        fputs("Error: failed to create window!\n", stderr);
        return APP_ERROR;
    }

    glfwSetWindowUserPointer(pApp->pWindow, pApp);
    glfwSetFramebufferSizeCallback(pApp->pWindow, framebufferResizeCallback);
    return APP_SUCCESS;
}

static AppResult initVulkan(struct Application *pApp)
{
    enum AppResult result = APP_SUCCESS;
    if ((result = createInstance(pApp)) != APP_SUCCESS) {
        fputs("Error: failed to create instance!\n", stderr);
        return result;
    }

    if ((result = setupDebugMessenger(pApp)) != APP_SUCCESS) {
        fputs("Error: failed to setup debug messenger!\n", stderr);
        return result;
    }

    if ((result = createSurface(pApp)) != APP_SUCCESS) {
        fputs("Error: failed to create window surface!\n", stderr);
        return result;
    }

    if ((result = pickPhysicalDevice(pApp)) != APP_SUCCESS) {
        fputs("Error: failed to pick physical device!\n", stderr);
        return result;
    }

    if ((result = createLogicalDevice(pApp)) != APP_SUCCESS) {
        fputs("Error: failed to create logical device!\n", stderr);
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

    if ((result = createRenderPass(pApp)) != APP_SUCCESS) {
        fputs("Error: failed to create render pass!\n", stderr);
        return result;
    }

    if ((result = createGraphicsPipeline(pApp)) != APP_SUCCESS) {
        fputs("Error: failed to create graphics pipeline!\n", stderr);
        return result;
    }

    if ((result = createFramebuffers(pApp)) != APP_SUCCESS) {
        fputs("Error: failed to create framebuffers!\n", stderr);
        return result;
    }

    if ((result = createCommandPool(pApp)) != APP_SUCCESS) {
        fputs("Error: failed to create command pool!\n", stderr);
        return result;
    }

    if ((result = createVertexBuffer(pApp)) != APP_SUCCESS) {
        fputs("Error: failed to create vertex buffer!\n", stderr);
        return result;
    }

    if ((result = createCommandBuffers(pApp)) != APP_SUCCESS) {
        fputs("Error: failed to create command buffer!\n", stderr);
        return result;
    }

    if ((result = createSyncObjects(pApp)) != APP_SUCCESS) {
        fputs("Error: failed to create sync objects!\n", stderr);
        return result;
    }

    return APP_SUCCESS;
}

static AppResult mainLoop(Application *pApp)
{
    uint32_t currentFrame = 0;
    pApp->framebufferResized = false;
    while (!glfwWindowShouldClose(pApp->pWindow)) {
        glfwPollEvents();
        drawFrame(pApp, currentFrame);
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    vkDeviceWaitIdle(pApp->device);

    return APP_SUCCESS;
}

static AppResult cleanup(struct Application *pApp)
{
    cleanupSwapchain(pApp);

    vkDestroyBuffer(pApp->device, pApp->vertexBuffer, NULL);
    vkFreeMemory(pApp->device, pApp->vertexBufferMemory, NULL);

    vkDestroyPipeline(pApp->device, pApp->graphicsPipeline, NULL);
    vkDestroyPipelineLayout(pApp->device, pApp->pipelineLayout, NULL);
    vkDestroyRenderPass(pApp->device, pApp->renderPass, NULL);

    for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(pApp->device, pApp->imageAvailableSemaphores[i], NULL);
        vkDestroySemaphore(pApp->device, pApp->renderFinishedSemaphores[i], NULL);
        vkDestroyFence(pApp->device, pApp->inFlightFences[i], NULL);
    }

    vkDestroyCommandPool(pApp->device, pApp->commandPool, NULL);

    vkDestroyDevice(pApp->device, NULL);

    if (ENABLE_VALIDATION_LAYERS) {
        destroyDebugUtilsMessengerEXT(pApp->instance, pApp->debugMessenger, NULL);
    }

    vkDestroySurfaceKHR(pApp->instance, pApp->surface, NULL);
    vkDestroyInstance(pApp->instance, NULL);

    glfwDestroyWindow(pApp->pWindow);

    glfwTerminate();

    return APP_SUCCESS;
}

AppResult appRun(Application *pApp)
{
    int result;
    if ((result = initWindow(pApp)) != APP_SUCCESS) {
        fputs("Error: failed to initialize window!\n", stderr);
        goto cleanup;
    }

    if ((result = initVulkan(pApp)) != APP_SUCCESS) {
        fputs("Error failed to initialize Vulkan!\n", stderr);
        goto cleanup;
    }

    if ((result = mainLoop(pApp)) != APP_SUCCESS) {
        fputs("Error: main loop failed!\n", stderr);
        goto cleanup;
    }

cleanup:
    cleanup(pApp);
    return result;
}
