#ifndef APPLICATION_H
#define APPLICATION_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <stdint.h>

extern const bool ENABLE_VALIDATION_LAYERS;
extern const char *const VALIDATION_LAYERS[];
extern const uint8_t VALIDATION_LAYER_COUNT;
extern const uint32_t APPLICATION_STRUCT_SIZE;

struct application {
    GLFWwindow *window;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    VkSurfaceKHR surface;
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkQueue graphics_queue;
    VkQueue present_queue;
};

enum app_result {
    APP_SUCCESS = 0,
    APP_ERROR = -1,
};

enum app_result app_run(struct application *app);

#endif
