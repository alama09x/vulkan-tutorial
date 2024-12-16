#ifndef VERTEX_H
#define VERTEX_H

#include <cglm/cglm.h>
#include <vulkan/vulkan_core.h>

const uint32_t ATTRIBUTE_DESCRIPTION_COUNT = 2;

typedef struct Vertex {
    vec2 pos;
    vec3 color;
} Vertex;

const Vertex VERTICES[] = {
    { .pos = {0.0f, -0.5f}, .color = {1.0f, 0.0f, 0.0f} },
    { .pos = {0.5f, 0.5f}, .color = {0.0f, 1.0f, 0.0f} },
    { .pos = {-0.5f, 0.0f}, .color = {1.0f, 0.0f, 1.0f} },
};

VkVertexInputBindingDescription vertexGetBindingDescription() {
    VkVertexInputBindingDescription bindingDescription = {
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };

    return bindingDescription;
}

void vertexGetAttributeDescriptions(
    VkVertexInputAttributeDescription attributeDescriptions[ATTRIBUTE_DESCRIPTION_COUNT])
{
    attributeDescriptions[0] = (VkVertexInputAttributeDescription) {
        .binding = 0,
        .location = 0,
        .format = VK_FORMAT_R32G32_SFLOAT,
        .offset = offsetof(Vertex, pos),
    };

    attributeDescriptions[1] = (VkVertexInputAttributeDescription) {
        .binding = 0,
        .location = 1,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(Vertex, color),
    };
}

#endif
