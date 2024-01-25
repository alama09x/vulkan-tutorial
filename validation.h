#ifndef VALIDATION_H
#define VALIDATION_H

#include "App.h"

bool checkValidationLayerSupport();
const char **getRequiredExtensions(uint32_t *pExtensionCount);
const VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo();
int setupDebugMessenger(App *app);
int cleanupValidation(App *app);

#endif
