#include "init/graphics_pipeline.h"
#include "init/vertex.h"
#include <stdio.h>
#include <stdlib.h>

static AppResult readFile(const char *pFilename, char **ppCode, uint32_t *pSize)
{
    FILE *pf = fopen(pFilename, "rb+");
    if (!pf) {
        fprintf(stderr, "Error: file \"%s\" could not be opened!\n", pFilename);
        return APP_ERROR;
    }

    fseek(pf, 0, SEEK_END);
    *pSize = ftell(pf);

    *ppCode = NULL;
    *ppCode = malloc(*pSize * sizeof(char));
    if (!*ppCode) {
        fputs("Error: could not allocate memory!\n", stderr);
        return APP_ERROR;
    }

    rewind(pf);

    fread(*ppCode, sizeof(char), *pSize, pf);

    fclose(pf);
    return APP_SUCCESS;
}

static AppResult createShaderModule(
    const VkDevice device,
    const char *pCode,
    const uint32_t size,
    VkShaderModule *pShaderModule)
{
    const VkShaderModuleCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = size,
        .pCode = (const uint32_t *)pCode,
    };

    if (vkCreateShaderModule(device, &createInfo, NULL, pShaderModule) != VK_SUCCESS) {
        fputs("Error: failed to create shader module!\n", stderr);
        return APP_ERROR;
    }

    return APP_SUCCESS;
}

AppResult createGraphicsPipeline(Application *pApp)
{
    uint32_t vertShaderSize, fragShaderSize;
    char *pVertShaderCode, *pFragShaderCode;

    readFile("./bin/vert.spv", &pVertShaderCode, &vertShaderSize);
    readFile("./bin/frag.spv", &pFragShaderCode, &fragShaderSize);

    VkShaderModule vertShaderModule, fragShaderModule;
    createShaderModule(pApp->device, pVertShaderCode, vertShaderSize, &vertShaderModule);
    createShaderModule(pApp->device, pFragShaderCode, fragShaderSize, &fragShaderModule);

    const VkPipelineShaderStageCreateInfo vertShaderStageInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertShaderModule,
        .pName = "main",
    };

    const VkPipelineShaderStageCreateInfo fragShaderStageInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragShaderModule,
        .pName = "main",
    };

    const VkPipelineShaderStageCreateInfo shaderStages[] = {
        vertShaderStageInfo,
        fragShaderStageInfo,
    };

    VkVertexInputBindingDescription bindingDescription = vertexGetBindingDescription();
    VkVertexInputAttributeDescription attributeDescriptions[ATTRIBUTE_DESCRIPTION_COUNT];
    vertexGetAttributeDescriptions(attributeDescriptions);

    const VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .vertexAttributeDescriptionCount = ATTRIBUTE_DESCRIPTION_COUNT,
        .pVertexBindingDescriptions = &bindingDescription,
        .pVertexAttributeDescriptions = attributeDescriptions,
    };

    const VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float)pApp->swapchainExtent.width,
        .height = (float)pApp->swapchainExtent.height,
        .minDepth= 0.0f,
        .maxDepth = 1.0f,
    };

    VkRect2D scissor = {
        .offset = {
            .x = 0,
            .y = 0,
        },
        .extent = pApp->swapchainExtent,
    };

    const uint32_t dynamicStateCount = 2;
    const VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

    const VkPipelineDynamicStateCreateInfo dynamicState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = dynamicStateCount,
        .pDynamicStates = dynamicStates,
    };

    const VkPipelineViewportStateCreateInfo viewportState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        // multiple requires enabling designated GPU feature
        .viewportCount = 1,
        .scissorCount = 1,
        .pViewports = &viewport,
        .pScissors = &scissor,
    };

    const VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        // requires enabling designated GPU feature
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        // anything else requires enabling designated GPU feature
        .polygonMode = VK_POLYGON_MODE_FILL,
        // wider requires enabling designated GPU feature
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
    };

    const VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    };

    const VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT
            | VK_COLOR_COMPONENT_G_BIT
            | VK_COLOR_COMPONENT_B_BIT
            | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,  // optional
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO, // optional
        .colorBlendOp = VK_BLEND_OP_ADD,             // optional
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,  // optional
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO, // optional
        .alphaBlendOp = VK_BLEND_OP_ADD,             // optional
    };

    const VkPipelineColorBlendStateCreateInfo colorBlending = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY, // optional
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
        .blendConstants = { 0.0f, 0.0f, 0.0f, 0.0f },
    };

    const VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0,
        .pushConstantRangeCount = 0,
    };

    if (vkCreatePipelineLayout(pApp->device, &pipelineLayoutInfo, NULL, &pApp->pipelineLayout)
        != VK_SUCCESS)
    {
        fputs("Error: failed to create pipeline layout!\n", stderr);
        return APP_ERROR;
    }

    const VkGraphicsPipelineCreateInfo pipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = NULL, // optional
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicState,
        .layout = pApp->pipelineLayout,
        .renderPass = pApp->renderPass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE, // optional
        .basePipelineIndex = -1,              // optional
    };

    if (vkCreateGraphicsPipelines(
        pApp->device, NULL, 1, &pipelineInfo, NULL, &pApp->graphicsPipeline)
        != VK_SUCCESS)
    {
        fputs("Error: failed to create graphics pipeline!\n", stderr);
        return APP_ERROR;
    }

    vkDestroyShaderModule(pApp->device, vertShaderModule, NULL);
    vkDestroyShaderModule(pApp->device, fragShaderModule, NULL);
    free(pVertShaderCode);
    free(pFragShaderCode);
    return APP_SUCCESS;
}
