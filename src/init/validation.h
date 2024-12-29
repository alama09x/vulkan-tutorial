#ifndef VALIDATION_H
#define VALIDATION_H

#include "application.h"

extern const char *const VALIDATION_LAYERS[];
extern const uint8_t VALIDATION_LAYER_COUNT;

AppResult setupDebugMessenger(Application *pApp);

bool checkValidationLayerSupport();
VkDebugUtilsMessengerCreateInfoEXT configureDebugMessengerCreateInfo();

void destroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks *pAllocator
);

#endif
