#ifndef APPLICATION_H
#define APPLICATION_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <stdint.h>

extern const bool ENABLE_VALIDATION_LAYERS;
#define MAX_FRAMES_IN_FLIGHT 2

typedef struct Application {
    GLFWwindow *pWindow;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSwapchainKHR swapchain;
    uint32_t swapchainImageCount;
    VkImage *pSwapchainImages;
    VkFormat swapchainImageFormat;
    VkExtent2D swapchainExtent;
    VkImageView *pSwapchainImageViews;
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    VkFramebuffer *pSwapchainFramebuffers;
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT];
    VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT];
    bool framebufferResized;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
} Application;

typedef enum AppResult {
    APP_SUCCESS = 0,
    APP_ERROR = -1,
} AppResult;

AppResult appRun(Application *pApp);

#endif
