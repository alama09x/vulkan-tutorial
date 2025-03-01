#ifndef BUFFERS_H
#define BUFFERS_H

#include "application.h"

AppResult createVertexBuffer(Application* pApp);
AppResult createIndexBuffer(Application* pApp);
AppResult createUniformBuffers(Application* pApp);

#endif
