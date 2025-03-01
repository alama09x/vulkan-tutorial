#ifndef VERTEX_H
#define VERTEX_H

#include <cglm/cglm.h>
#include <stdint.h>

// Represents a single vertex in the vertex buffer
typedef struct Vertex {
    vec2 pos;
    vec3 color;
} Vertex;

extern const uint32_t VERTEX_COUNT;
extern const Vertex VERTICES[];

#endif
