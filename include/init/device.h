#ifndef DEVICE_H
#define DEVICE_H

#include "application.h"

#define QUEUE_FAMILY_INDICES_COUNT 2

struct queue_family_indices {
    uint32_t *graphics_family;
    uint32_t *present_family;
};

enum app_result pick_physical_device(struct application *app);
enum app_result create_logical_device(struct application *app);

#endif
