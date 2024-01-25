#ifndef APP_H
#define APP_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

extern const uint32_t WIDTH;
extern const uint32_t HEIGHT;

extern const bool ENABLE_VALIDATION_LAYERS;
extern const char *VALIDATION_LAYERS[];
extern const size_t VALIDATION_LAYER_COUNT;

typedef struct App {
    GLFWwindow *window;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
} App;

int run(App *app);

int initWindow(App *app);
int initVulkan(App *app);
int mainLoop(App *app);
int cleanup(App *app);

#endif
