#ifndef APPLICATION_H
#define APPLICATION_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <stdint.h>

extern const bool ENABLE_VALIDATION_LAYERS;
#define MAX_FRAMES_IN_FLIGHT 2

/// The general state of the program
typedef struct Application {
    GLFWwindow*                 pWindow;
    VkInstance                  instance;
    VkDebugUtilsMessengerEXT    debugMessenger;
    VkSurfaceKHR                surface;

    VkPhysicalDevice            physicalDevice;
    VkDevice                    device;

    VkQueue                     graphicsQueue;
    VkQueue                     transferQueue;
    VkQueue                     presentQueue;

    VkSwapchainKHR              swapchain;
    uint32_t                    swapchainImageCount;
    VkImage*                    pSwapchainImages;
    VkFormat                    swapchainImageFormat;
    VkExtent2D                  swapchainExtent;
    VkImageView*                pSwapchainImageViews;

    VkRenderPass                renderPass;
    VkDescriptorSetLayout       descriptorSetLayout;
    VkPipelineLayout            pipelineLayout;
    VkPipeline                  graphicsPipeline;
    VkFramebuffer*              pSwapchainFramebuffers;

    VkCommandPool               graphicsCommandPool;
    VkCommandPool               transferCommandPool;

    VkCommandBuffer             commandBuffers[MAX_FRAMES_IN_FLIGHT];

    VkSemaphore                 imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore                 renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT];
    VkFence                     inFlightFences[MAX_FRAMES_IN_FLIGHT];

    bool                        framebufferResized;

    VkBuffer                    vertexBuffer;
    VkDeviceMemory              vertexBufferMemory;

    VkBuffer                    indexBuffer;
    VkDeviceMemory              indexBufferMemory;

    VkBuffer                    uniformBuffers[MAX_FRAMES_IN_FLIGHT];
    VkDeviceMemory              uniformBuffersMemory[MAX_FRAMES_IN_FLIGHT];
    void*                       uniformBuffersMapped[MAX_FRAMES_IN_FLIGHT];

    VkDescriptorPool            descriptorPool;
    VkDescriptorSet             descriptorSets[MAX_FRAMES_IN_FLIGHT];
} Application;

/// Represents the success or failure of a process
typedef enum AppResult {
    APP_SUCCESS = VK_SUCCESS,
    APP_ERROR,
} AppResult;

// Macros to simplify handling `AppResult` objects
#define APP_ERROR_MSG(msg)                  \
    fprintf(stderr, "Error: %s!\n", (msg));

#define APP_ERROR(msg)                      \
    APP_ERROR_MSG(msg);                     \
    return APP_ERROR;

#define APP_EXPECT(expr, msg)               \
    if ((AppResult)(expr) != APP_SUCCESS) {            \
        APP_ERROR(msg)                      \
    }

AppResult appRun(Application *pApp);

#endif
