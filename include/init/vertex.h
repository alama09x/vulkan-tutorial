#ifndef VERTEX_H
#define VERTEX_H

#include <cglm/cglm.h>
#include <vulkan/vulkan_core.h>

extern const uint32_t ATTRIBUTE_DESCRIPTION_COUNT;

typedef struct Vertex {
    vec2 pos;
    vec3 color;
} Vertex;

#define VERTEX_COUNT 3
extern const Vertex VERTICES[VERTEX_COUNT];

VkVertexInputBindingDescription vertexGetBindingDescription();
void vertexGetAttributeDescriptions(
    VkVertexInputAttributeDescription
    attributeDescriptions[ATTRIBUTE_DESCRIPTION_COUNT]);

#endif
