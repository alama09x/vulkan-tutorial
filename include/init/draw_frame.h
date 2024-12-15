#ifndef DRAW_FRAME_H
#define DRAW_FRAME_H

#include "application.h"


enum app_result cleanup_swapchain(struct application *app);
enum app_result draw_frame(struct application *app, uint32_t current_frame);

#endif
