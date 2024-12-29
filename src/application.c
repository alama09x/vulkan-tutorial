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
    APP_EXPECT(
        glfwCreateWindowSurface(
            pApp->instance,
            pApp->pWindow,
            NULL,
            &pApp->surface
        ),
        "failed to create window surface"
    );

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
        APP_ERROR("failed to create window");
    }

    glfwSetWindowUserPointer(pApp->pWindow, pApp);
    glfwSetFramebufferSizeCallback(pApp->pWindow, framebufferResizeCallback);

    return APP_SUCCESS;
}

static AppResult initVulkan(Application *pApp)
{
    APP_EXPECT(createInstance(pApp), "failed to create instance");

    APP_EXPECT(setupDebugMessenger(pApp), "failed to setup debug messenger");

    APP_EXPECT(createSurface(pApp), "failed to create window surface");

    APP_EXPECT(pickPhysicalDevice(pApp), "failed to pick physical device");
    APP_EXPECT(createLogicalDevice(pApp), "failed to create logical device");

    APP_EXPECT(createSwapchain(pApp), "failed to create swapchain");
    APP_EXPECT(createImageViews(pApp), "failed to create image views");

    APP_EXPECT(createRenderPass(pApp), "failed to create render pass");

    APP_EXPECT(
        createDescriptorSetLayout(pApp),
        "failed to create descriptor set layout"
    );
    APP_EXPECT(createGraphicsPipeline(pApp), "failed to create graphics pipeline");

    APP_EXPECT(createFramebuffers(pApp), "failed to create framebuffers");

    QueueFamilyIndices indices;
    findQueueFamilies(pApp->physicalDevice, pApp->surface, &indices);

    APP_EXPECT(
        createCommandPool(
            pApp,
            &pApp->graphicsCommandPool,
            *indices.pGraphicsFamily),
        "failed to create graphics command pool"
    );
    APP_EXPECT(
        createCommandPool(
            pApp,
            &pApp->transferCommandPool,
            *indices.pTransferFamily
        ),
        "failed to create transfer command pool"
    );

    cleanupQueueFamilies(&indices);

    APP_EXPECT(createVertexBuffer(pApp), "failed to create vertex buffer");
    APP_EXPECT(createIndexBuffer(pApp), "failed to create index buffer");
    APP_EXPECT(createUniformBuffers(pApp), "failed to create uniform buffers");

    APP_EXPECT(createCommandBuffers(pApp), "failed to create command buffer!");
    APP_EXPECT(createSyncObjects(pApp), "failed to create sync objects!");

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

    vkDestroyBuffer(pApp->device, pApp->indexBuffer, NULL);
    vkFreeMemory(pApp->device, pApp->indexBufferMemory, NULL);

    vkDestroyBuffer(pApp->device, pApp->vertexBuffer, NULL);
    vkFreeMemory(pApp->device, pApp->vertexBufferMemory, NULL);

    vkDestroyPipeline(pApp->device, pApp->graphicsPipeline, NULL);
    vkDestroyPipelineLayout(pApp->device, pApp->pipelineLayout, NULL);

    vkDestroyDescriptorSetLayout(pApp->device, pApp->descriptorSetLayout, NULL);

    vkDestroyRenderPass(pApp->device, pApp->renderPass, NULL);

    for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(pApp->device, pApp->imageAvailableSemaphores[i], NULL);
        vkDestroySemaphore(pApp->device, pApp->renderFinishedSemaphores[i], NULL);
        vkDestroyFence(pApp->device, pApp->inFlightFences[i], NULL);
    }

    vkDestroyCommandPool(pApp->device, pApp->transferCommandPool, NULL);
    vkDestroyCommandPool(pApp->device, pApp->graphicsCommandPool, NULL);

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
    APP_EXPECT(initWindow(pApp), "failed to initialize window");
    APP_EXPECT(initVulkan(pApp), "failed to initialize Vulkan");
    APP_EXPECT(mainLoop(pApp), "main loop failed");
    APP_EXPECT(cleanup(pApp), "failed to cleanup");

    return APP_SUCCESS;
}
