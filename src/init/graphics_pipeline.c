#include "graphics_pipeline.h"

#include "application.h"
#include "data/uniform.h"
#include "data/vertex.h"
#include <stdio.h>
#include <stdlib.h>

const uint32_t ATTRIBUTE_DESCRIPTION_COUNT = 2;

/// Read contents of file `pFilename` and store the bytes in `ppCode`,
/// allocating `size` bytes
static AppResult readFile(
    const char*    pFilename,
    char**         ppCode,
    uint32_t       size)
{
    FILE *pFile;
    // Use safer fopen_s
    if (fopen_s(&pFile, pFilename, "rb+") != 0) {
        fprintf(stderr, "Error: file \"%s\" could not be opened!\n", pFilename);
        return APP_ERROR;
    }

    fseek(pFile, 0, SEEK_END);
    size = ftell(pFile);

    *ppCode = NULL;
    *ppCode = malloc(size * sizeof(char));
    if (!*ppCode) {
        APP_ERROR("could not allocate memory");
    }

    rewind(pFile);
    fread(*ppCode, sizeof(char), size, pFile);

    fclose(pFile);
    return APP_SUCCESS;
}

/// Must clean up `pShaderModule`
static AppResult createShaderModule(
    const VkDevice     device,
    const char*        pCode,
    const uint32_t     size,
    VkShaderModule*    pShaderModule)
{
    const VkShaderModuleCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = size,
        .pCode = (const uint32_t *)pCode,
    };

    APP_EXPECT(
        vkCreateShaderModule(device, &createInfo, NULL, pShaderModule),
        "failed to create shader module"
    );

    return APP_SUCCESS;
}

/// Must clean up `pApp->descriptorPool`
AppResult createDescriptorPool(Application* pApp)
{
    const VkDescriptorPoolSize poolSize = {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = (uint32_t)MAX_FRAMES_IN_FLIGHT,
    };

    const VkDescriptorPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize,
        .maxSets = (uint32_t)MAX_FRAMES_IN_FLIGHT,
    };

    APP_EXPECT(
        vkCreateDescriptorPool(pApp->device, &poolInfo, NULL, &pApp->descriptorPool),
        "failed to create descriptor pool!"
    );

    return APP_SUCCESS;
}

/// Must clean up `pApp->descriptorSets`
AppResult createDescriptorSets(Application* pApp)
{
    VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT];
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        layouts[i] = pApp->descriptorSetLayout;
    }

    const VkDescriptorSetAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = pApp->descriptorPool,
        .descriptorSetCount = (uint32_t)MAX_FRAMES_IN_FLIGHT,
        .pSetLayouts = layouts,
    };

    APP_EXPECT(
        vkAllocateDescriptorSets(pApp->device, &allocInfo, pApp->descriptorSets),
        "failed to allocate descriptor sets!"
    );

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        const VkDescriptorBufferInfo bufferInfo = {
            .buffer = pApp->uniformBuffers[i],
            .offset = 0,
            .range = sizeof(UniformBufferObject),
        };

        const VkWriteDescriptorSet descriptorWrite = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = pApp->descriptorSets[i],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .pBufferInfo = &bufferInfo,
            .pImageInfo = NULL,
            .pTexelBufferView = NULL,
        };

        vkUpdateDescriptorSets(pApp->device, 1, &descriptorWrite, 0, NULL);
    }

    return APP_SUCCESS;
}

/// Must clean up `pApp->descriptorSetLayout`
AppResult createDescriptorSetLayout(Application* pApp)
{
    const VkDescriptorSetLayoutBinding uboLayoutBinding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = NULL,
    };

    const VkDescriptorSetLayoutCreateInfo layoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &uboLayoutBinding,
    };

    APP_EXPECT(
        vkCreateDescriptorSetLayout(
            pApp->device,
            &layoutInfo,
            NULL,
            &pApp->descriptorSetLayout
        ),
        "failed to create descriptor set layout"
    );

    return APP_SUCCESS;
}

static VkVertexInputBindingDescription vertexGetBindingDescription()
{
    const VkVertexInputBindingDescription bindingDescription = {
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };

    return bindingDescription;
}

static void vertexGetAttributeDescriptions(
    VkVertexInputAttributeDescription
    attributeDescriptions[ATTRIBUTE_DESCRIPTION_COUNT])
{
    attributeDescriptions[0] = (VkVertexInputAttributeDescription) {
        .binding = 0,
        .location = 0,
        .format = VK_FORMAT_R32G32_SFLOAT,
        .offset = offsetof(Vertex, pos),
    };

    attributeDescriptions[1] = (VkVertexInputAttributeDescription) {
        .binding = 0,
        .location = 1,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(Vertex, color),
    };
}

/// Must clean up `pApp->graphicsPipeline`
AppResult createGraphicsPipeline(Application* pApp)
{
    uint32_t vertShaderSize, fragShaderSize;
    char *pVertShaderCode, *pFragShaderCode;

    // TODO: make constants
    readFile("./bin/shader.vert.spv", &pVertShaderCode, vertShaderSize);
    readFile("./bin/shader.frag.spv", &pFragShaderCode, fragShaderSize);

    VkShaderModule vertShaderModule, fragShaderModule;
    createShaderModule(
        pApp->device,
        pVertShaderCode,
        vertShaderSize,
        &vertShaderModule
    );
    createShaderModule(
        pApp->device,
        pFragShaderCode,
        fragShaderSize,
        &fragShaderModule
    );

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

    VkVertexInputBindingDescription bindingDescription =
        vertexGetBindingDescription();

    VkVertexInputAttributeDescription
    attributeDescriptions[ATTRIBUTE_DESCRIPTION_COUNT];

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
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
    };

    const VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    };

    const VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT,
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
        .setLayoutCount = 1,
        .pSetLayouts = &pApp->descriptorSetLayout,
        .pushConstantRangeCount = 0,
    };

    APP_EXPECT(
        vkCreatePipelineLayout(
            pApp->device,
            &pipelineLayoutInfo,
            NULL,
            &pApp->pipelineLayout
        ),
        "failed to create pipeline layout"
    );

    const VkGraphicsPipelineCreateInfo pipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = NULL,
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicState,
        .layout = pApp->pipelineLayout,
        .renderPass = pApp->renderPass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1,              // optional
    };

    APP_EXPECT(
        vkCreateGraphicsPipelines(
            pApp->device,
            NULL,
            1,
            &pipelineInfo,
            NULL,
            &pApp->graphicsPipeline
        ),
        "failed to create graphics pipeline"
    );

    vkDestroyShaderModule(pApp->device, vertShaderModule, NULL);
    vkDestroyShaderModule(pApp->device, fragShaderModule, NULL);
    free(pVertShaderCode);
    free(pFragShaderCode);
    return APP_SUCCESS;
}
