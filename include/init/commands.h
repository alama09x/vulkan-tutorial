#ifndef COMMANDS_H
#define COMMANDS_H

#include "application.h"


enum app_result create_command_pool(struct application *app);
enum app_result create_command_buffer(struct application *app);
enum app_result record_command_buffer(
    struct application *app,
    VkCommandBuffer command_buffer,
    uint32_t image_index);

#endif
