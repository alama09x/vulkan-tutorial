#include "vertex.h"
#include <stdint.h>

const uint32_t VERTEX_COUNT = 4;
const Vertex VERTICES[] = {
    { .pos = {-0.5f, -0.5f}, .color = {1.0f, 0.0f, 0.0f} },
    { .pos = {0.5f, -0.5f}, .color = {0.0f, 1.0f, 0.0f} },
    { .pos = {0.5f, 0.5f}, .color = {0.0f, 0.0f, 1.0f} },
    { .pos = {-0.5f, 0.5f}, .color = {1.0f, 1.0f, 1.0f} },
};
