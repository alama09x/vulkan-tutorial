#ifndef VALIDATION_H
#define VALIDATION_H

#include "application.h"

extern const char *const VALIDATION_LAYERS[];
extern const uint8_t VALIDATION_LAYER_COUNT;

enum app_result setup_debug_messenger(struct application *app);

bool check_validation_layer_support();
VkDebugUtilsMessengerCreateInfoEXT configure_debug_messenger_create_info();

void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks *pAllocator);

#endif
