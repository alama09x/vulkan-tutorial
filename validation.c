#include "validation.h"
#include <vulkan/vulkan_core.h>

const bool ENABLE_VALIDATION_LAYERS =
#ifdef NDEBUG
    false
#else
    true
#endif
;

const char *VALIDATION_LAYERS[] = {
    "VK_LAYER_KHRONOS_validation",
};
const size_t VALIDATION_LAYER_COUNT = sizeof(VALIDATION_LAYERS) / sizeof(*VALIDATION_LAYERS);

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{
    fprintf(stderr, "Validation error: %s\n", pCallbackData->pMessage);
    return VK_FALSE;
}

void destroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator)
{
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

    if (func)
        func(instance, debugMessenger, pAllocator);
}

VkResult createDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    if (func)
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    else
        return VK_ERROR_EXTENSION_NOT_PRESENT;
}

bool checkValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);

    VkLayerProperties availableLayers[layerCount];
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    for (size_t i = 0; i < VALIDATION_LAYER_COUNT; i++) {
        bool layerFound = false;

        for (size_t j = 0; j < layerCount; j++) {
            if (strcmp(VALIDATION_LAYERS[i], availableLayers[j].layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
            return false;
    }


    return true;
}

const char **getRequiredExtensions(uint32_t *pExtensionCount)
{
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    *pExtensionCount = ENABLE_VALIDATION_LAYERS
        ? glfwExtensionCount + 1
        : glfwExtensionCount;

    const char **extensions = malloc(sizeof(*extensions) * *pExtensionCount);
    extensions = glfwExtensions;

    if (ENABLE_VALIDATION_LAYERS)
        extensions[glfwExtensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

    return extensions;
}

const VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo()
{
    return (VkDebugUtilsMessengerCreateInfoEXT) {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pUserData = NULL, // optional
        .pfnUserCallback = debugCallback,
    };
}

int setupDebugMessenger(App *app)
{
    if (!ENABLE_VALIDATION_LAYERS)
        return EXIT_SUCCESS;

    const VkDebugUtilsMessengerCreateInfoEXT createInfo = debugMessengerCreateInfo();
    if (createDebugUtilsMessengerEXT(app->instance, &createInfo, NULL, &app->debugMessenger)
        != VK_SUCCESS
    ) {
        fputs("Error: failed to set up debug messenger!", stderr);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int cleanupValidation(App *app)
{
    if (ENABLE_VALIDATION_LAYERS)
        destroyDebugUtilsMessengerEXT(app->instance, app->debugMessenger, NULL);

    return EXIT_SUCCESS;
}
