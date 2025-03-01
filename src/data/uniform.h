#ifndef UNIFORM_H
#define UNIFORM_H

#include <cglm/cglm.h>

// Accessed as global objects by shaders
typedef struct UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} UniformBufferObject;

#endif
