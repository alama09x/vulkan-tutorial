#ifndef DRAW_FRAME_H
#define DRAW_FRAME_H

#include "application.h"

AppResult cleanupSwapchain(Application *pApp);
AppResult drawFrame(Application *pApp, uint32_t currentFrame);

#endif
