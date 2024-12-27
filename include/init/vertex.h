#ifndef VERTEX_H
#define VERTEX_H

#include <cglm/cglm.h>
#include <vulkan/vulkan_core.h>

extern const uint32_t ATTRIBUTE_DESCRIPTION_COUNT;

typedef struct Vertex {
    vec2 pos;
    vec3 color;
} Vertex;

extern const uint32_t VERTEX_COUNT;
extern const Vertex VERTICES[];

extern const uint32_t INDEX_COUNT;
extern const uint16_t INDICES[];

VkVertexInputBindingDescription vertexGetBindingDescription();
void vertexGetAttributeDescriptions(
    VkVertexInputAttributeDescription
    attributeDescriptions[ATTRIBUTE_DESCRIPTION_COUNT]);

#endif
