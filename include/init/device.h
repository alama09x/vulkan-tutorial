#ifndef DEVICE_H
#define DEVICE_H

#include "application.h"

struct queue_family_indices {
    uint32_t *graphics_family;
};


enum app_result pick_physical_device(struct application *app);
enum app_result create_logical_device(struct application *app);

#endif
