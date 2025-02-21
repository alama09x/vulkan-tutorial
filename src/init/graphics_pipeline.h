#ifndef GRAPHICS_PIPELINE_H
#define GRAPHICS_PIPELINE_H

#include "application.h"

AppResult createDescriptorPool(Application *pApp);
AppResult createDescriptorSetLayout(Application *pApp);
AppResult createDescriptorSets(Application *pApp);
AppResult createGraphicsPipeline(Application *pApp);

#endif
