#include "instance.h"
#include "validation.h"

int createInstance(App *app)
{
    if (ENABLE_VALIDATION_LAYERS && !checkValidationLayerSupport()) {
        fputs("Error: validation layers requested but not available!", stderr);        
        return EXIT_FAILURE;
    }
    
    const VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Vulkan Triangle",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0,
    };

    uint32_t extensionCount;
    const char **extensions = getRequiredExtensions(&extensionCount);

    uint32_t availableExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &availableExtensionCount, NULL);

    VkExtensionProperties availableExtensions[availableExtensionCount];
    vkEnumerateInstanceExtensionProperties(NULL, &availableExtensionCount, availableExtensions);

    puts("Available extensions:");
    for (size_t i = 0; i < availableExtensionCount; i++)
        printf("\t%s\n", availableExtensions[i].extensionName);

    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = extensionCount,
        .ppEnabledExtensionNames = extensions,
        .enabledLayerCount = 0,
    };

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (ENABLE_VALIDATION_LAYERS) {
        debugCreateInfo = debugMessengerCreateInfo();
        createInfo.enabledLayerCount = VALIDATION_LAYER_COUNT;
        createInfo.ppEnabledLayerNames = VALIDATION_LAYERS;
        createInfo.pNext = &debugCreateInfo;
    }

    if (vkCreateInstance(&createInfo, NULL, &app->instance) != VK_SUCCESS) {
        fputs("Error: failed to create instance!", stderr);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int cleanupInstance(App *app)
{
    vkDestroyInstance(app->instance, NULL);
    return EXIT_SUCCESS;
}
