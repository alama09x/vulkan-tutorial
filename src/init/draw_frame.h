#ifndef DRAW_FRAME_H
#define DRAW_FRAME_H

#include "application.h"
#include <time.h>

AppResult cleanupSwapchain(Application *pApp);
AppResult drawFrame(Application *pApp, uint32_t currentFrame, const time_t *pStartTime);

#endif
