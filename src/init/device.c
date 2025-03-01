#include "device.h"

#include "validation.h"
#include "swapchain.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __APPLE__
const uint8_t DEVICE_EXTENSION_COUNT = 2;
#else
const uint8_t DEVICE_EXTENSION_COUNT = 1;
#endif

const char *const DEVICE_EXTENSIONS[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#ifdef __APPLE__
    "VK_KHR_portability_subset",
#endif
};

static bool queueFamilyIndicesComplete(const QueueFamilyIndices* pIndices)
{
    return pIndices->pGraphicsFamily != NULL &&
           pIndices->pTransferFamily != NULL &&
           pIndices->pPresentFamily != NULL;
}

/// Populates `pIndices` with found queue family indices
/// Must clean up `pIndices` with `cleanupQueueFamilies(pIndices)`
void findQueueFamilies(
    VkPhysicalDevice        device,
    VkSurfaceKHR            surface,
    QueueFamilyIndices*     pIndices)
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

    VkQueueFamilyProperties pQueueFamilies[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(
        device,
        &queueFamilyCount,
        pQueueFamilies
    );

    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        if (pQueueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            pIndices->pGraphicsFamily = malloc(sizeof(uint32_t));
            *pIndices->pGraphicsFamily = i;
        }
        if (pQueueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
            pIndices->pTransferFamily = malloc(sizeof(uint32_t));
            *pIndices->pTransferFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (presentSupport) {
            pIndices->pPresentFamily = malloc(sizeof(uint32_t));
            *pIndices->pPresentFamily = i;
        }

        if (queueFamilyIndicesComplete(pIndices)) {
            break;
        }
    }

    if (!pIndices->pTransferFamily && pIndices->pGraphicsFamily) {
        pIndices->pTransferFamily = pIndices->pGraphicsFamily;
    }
}

void cleanupQueueFamilies(QueueFamilyIndices *pIndices)
{
    free(pIndices->pGraphicsFamily);
    free(pIndices->pPresentFamily);
}

/// Return true unless at least one of the required `DEVICE_EXTENSIONS` is not suppported
static bool checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);

    VkExtensionProperties pAvailableExtensions[extensionCount];
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, pAvailableExtensions);

    puts("Available device extensions:");
    for (uint32_t i = 0; i < extensionCount; i++) {
        printf("\t%s\n", pAvailableExtensions[i].extensionName);
    }

    for (uint8_t i = 0; i < DEVICE_EXTENSION_COUNT; i++) {
        bool extensionFound = false;

        for (uint8_t j = 0; j < extensionCount; j++) {
            if (strcmp(
                DEVICE_EXTENSIONS[i],
                pAvailableExtensions[j].extensionName
            ) == 0) {
                extensionFound = true;
                break;
            }
        }

        if (!extensionFound) {
            return false;
        }
    }

    return true;
}

/// A device is suitable if it has complete `QueueFamilyIndices`, all extensions are supported,
/// and the swapchain supports formats and present modes
static bool deviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    QueueFamilyIndices indices;
    findQueueFamilies(device, surface, &indices);

    bool indicesComplete = queueFamilyIndicesComplete(&indices);
    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapchainAdequate = false;
    if (extensionsSupported) {
        SwapchainSupportDetails swapchainSupport =
            querySwapchainSupport(device, surface);

        swapchainAdequate = swapchainSupport.pFormats != NULL &&
            swapchainSupport.pPresentModes != NULL;

        cleanupSwapchainSupport(&swapchainSupport);
    }

    cleanupQueueFamilies(&indices);

    return indicesComplete && extensionsSupported && swapchainAdequate;
}

/// Must clean up `pApp->device`
AppResult createLogicalDevice(Application *pApp)
{
    QueueFamilyIndices indices;
    findQueueFamilies(pApp->physicalDevice, pApp->surface, &indices);

    // TODO: check queue family indices against NULL
    const uint32_t queueFamilies[] = {
        *indices.pGraphicsFamily,
        *indices.pPresentFamily,
        *indices.pTransferFamily
    };

    // Only use unique queue families (for example, the graphics family and transfer family
    // could be of the same index)
    // Akin to creating a std::set in C++ for only unique indices
    uint32_t *pUniqueQueueFamilies =
        malloc(QUEUE_FAMILY_INDICES_COUNT * sizeof(uint32_t));

    uint32_t uniqueCount = 0;

    // TODO: implement lookup table for O(n) as opposed to this O(n^2)
    for (uint8_t i = 0; i < QUEUE_FAMILY_INDICES_COUNT; i++) {
        bool unique = true;
        for (uint8_t j = 0; j < uniqueCount; j++) {
            if (queueFamilies[i] == pUniqueQueueFamilies[j]) {
                unique = false;
                break;
            }
        }

        if (unique) {
            pUniqueQueueFamilies[uniqueCount++] = queueFamilies[i];
        }
    }

    // Used for device queue creation for each queue family
    VkDeviceQueueCreateInfo pQueueCreateInfos[uniqueCount];
    const float queuePriority = 1.0;

    for (uint8_t i = 0; i < uniqueCount; i++) {
        const VkDeviceQueueCreateInfo queueCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = pUniqueQueueFamilies[i],
            .queueCount = 1,
            .pQueuePriorities = &queuePriority,
        };
        pQueueCreateInfos[i] = queueCreateInfo;
    }

    const VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos = pQueueCreateInfos,
        .queueCreateInfoCount = uniqueCount,
        .pEnabledFeatures = &deviceFeatures,
        .enabledExtensionCount = DEVICE_EXTENSION_COUNT,
        .ppEnabledExtensionNames = DEVICE_EXTENSIONS,
        .enabledLayerCount = 0,
    };
    if (ENABLE_VALIDATION_LAYERS) {
        createInfo.enabledLayerCount = VALIDATION_LAYER_COUNT;
        createInfo.ppEnabledLayerNames = VALIDATION_LAYERS;
    }

    APP_EXPECT(
        vkCreateDevice(pApp->physicalDevice, &createInfo, NULL, &pApp->device),
        "failed to create logical device"
    );

    // Get device queue from each respective queue family
    vkGetDeviceQueue(pApp->device, *indices.pGraphicsFamily, 0, &pApp->graphicsQueue);
    vkGetDeviceQueue(pApp->device, *indices.pTransferFamily, 0, &pApp->transferQueue);
    vkGetDeviceQueue(pApp->device, *indices.pPresentFamily, 0, &pApp->presentQueue);

    free(pUniqueQueueFamilies);
    cleanupQueueFamilies(&indices);
    return APP_SUCCESS;
}

// Pick GPU that is suitable according to `deviceSuitable`. See `deviceSuitable` for conditions.
AppResult pickPhysicalDevice(Application* pApp)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(pApp->instance, &deviceCount, NULL);

    if (deviceCount == 0) {
        APP_ERROR("failed to find GPUs with Vulkan support");
    }

    VkPhysicalDevice pDevices[deviceCount];
    vkEnumeratePhysicalDevices(pApp->instance, &deviceCount, pDevices);

    for (uint32_t i = 0; i < deviceCount; i++) {
        if (deviceSuitable(pDevices[i], pApp->surface)) {
            pApp->physicalDevice = pDevices[i];
            break;
        }
    }

    if (!pApp->physicalDevice) {
        APP_ERROR("failed to find a suitable GPU");
    }

    return APP_SUCCESS;
}
