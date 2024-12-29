#ifndef VERTEX_H
#define VERTEX_H

#include <cglm/cglm.h>

typedef struct Vertex {
    vec2 pos;
    vec3 color;
} Vertex;

extern const uint32_t VERTEX_COUNT;
extern const Vertex VERTICES[];

#endif
