#ifndef APPLICATION_H
#define APPLICATION_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <stdint.h>

extern const bool ENABLE_VALIDATION_LAYERS;

struct application {
    GLFWwindow *window;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    VkSurfaceKHR surface;
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkQueue graphics_queue;
    VkQueue present_queue;
    VkSwapchainKHR swapchain;
    uint32_t swapchain_image_count;
    VkImage *swapchain_images;
    VkFormat swapchain_image_format;
    VkExtent2D swapchain_extent;
    VkImageView *swapchain_image_views;
    VkRenderPass render_pass;
    VkPipelineLayout pipeline_layout;
    VkPipeline graphics_pipeline;
    VkFramebuffer *swapchain_framebuffers;
    VkCommandPool command_pool;
    VkCommandBuffer command_buffer;
    VkSemaphore image_available_semaphore;
    VkSemaphore render_finished_semaphore;
    VkFence in_flight_fence;
};

enum app_result {
    APP_SUCCESS = 0,
    APP_ERROR = -1,
};

enum app_result app_run(struct application *app);

#endif
