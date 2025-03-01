#include "instance.h"
#include "validation.h"
#include <stdio.h>
#include <string.h>

static void getRequiredExtensions(
    uint32_t* pRequiredExtensionCount,
    const char** ppRequiredExtensions)
{
    uint32_t glfwExtensionCount = 0;
    const char **ppGlfwExtensions =
        glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    *pRequiredExtensionCount = glfwExtensionCount + ENABLE_VALIDATION_LAYERS;

    #ifdef __APPLE__
        (*pRequiredExtensionCount) += 2;
    #endif

    if (ppRequiredExtensions) {
        for (uint32_t i = 0; i < glfwExtensionCount; i++) {
            ppRequiredExtensions[i] = ppGlfwExtensions[i];
        }

        if (ENABLE_VALIDATION_LAYERS) {
            ppRequiredExtensions[glfwExtensionCount] =
                VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
        }

        #ifdef __APPLE__
            ppRequiredExtensions[glfwExtensionCount + 1] =
                VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
            ppRequiredExtensions[glfwExtensionCount + 2] =
                VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME;
        #endif
    }
}

static bool requiredExtensionsAvailable(
    const char **ppRequiredExtensionNames,
    VkExtensionProperties *pAvailableExtensions,
    uint32_t requiredCount,
    uint32_t availableCount
) {
    for (uint32_t i = 0; i < requiredCount; i++) {
        bool included = false;

        for (uint32_t j = 0; j < availableCount; j++) {
            const char *pAvailableName = pAvailableExtensions[j].extensionName;
            if (strncmp(
                ppRequiredExtensionNames[i],
                pAvailableName,
                strlen(pAvailableName)
            ) == 0) {
                included = true;
            }
        }

        if (!included) {
            return false;
        }
    }

    return true;
}

AppResult createInstance(Application *pApp)
{
    if (ENABLE_VALIDATION_LAYERS && !checkValidationLayerSupport()) {
        fputs("Error: validation layers requested, but not available!\n", stderr);
        return APP_ERROR;
    }

    const VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Hello Triangle",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0,
    };

    uint32_t requiredExtensionCount;
    getRequiredExtensions(&requiredExtensionCount, NULL);

    const char *pRequiredExtensions[requiredExtensionCount];
    getRequiredExtensions(&requiredExtensionCount, pRequiredExtensions);

    uint32_t availableExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &availableExtensionCount, NULL);

    VkExtensionProperties pAvailableExtensions[availableExtensionCount];
    vkEnumerateInstanceExtensionProperties(
        NULL,
        &availableExtensionCount,
        pAvailableExtensions
    );

    puts("Available instance extensions:");
    for (uint32_t i = 0; i < availableExtensionCount; i++) {
        printf("\t%s\n", pAvailableExtensions[i].extensionName);
    }

    if (!requiredExtensionsAvailable(
        pRequiredExtensions,
        pAvailableExtensions,
        requiredExtensionCount,
        availableExtensionCount))
    {
        fputs("Error: required extensions not available!\n", stderr);
        return APP_ERROR;
    }

    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = requiredExtensionCount,
        .ppEnabledExtensionNames = pRequiredExtensions,
        #ifdef __APPLE__
            .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
        #endif
    };
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (ENABLE_VALIDATION_LAYERS) {
        createInfo.enabledLayerCount = VALIDATION_LAYER_COUNT;
        createInfo.ppEnabledLayerNames = VALIDATION_LAYERS;

        debugCreateInfo = configureDebugMessengerCreateInfo();

        createInfo.pNext = &debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
    }

    APP_EXPECT(
        vkCreateInstance(&createInfo, NULL, &pApp->instance),
        "failed to create instance"
    );

    return APP_SUCCESS;
}
