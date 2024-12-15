#include "init/graphics_pipeline.h"
#include <stdio.h>
#include <stdlib.h>

static enum app_result read_file(const char *filename, char **code, uint32_t *size)
{
    FILE *f = fopen(filename, "rb+");
    if (!f) {
        fprintf(stderr, "Error: file \"%s\" could not be opened!\n", filename);
        return APP_ERROR;
    }

    fseek(f, 0, SEEK_END);
    *size = ftell(f);

    *code = NULL;
    *code = malloc(*size * sizeof(char));
    if (!*code) {
        fputs("Error: could not allocate memory!\n", stderr);
        return APP_ERROR;
    }

    rewind(f);

    fread(*code, sizeof(char), *size, f);

    fclose(f);
    return APP_SUCCESS;
}

static enum app_result create_shader_module(
    const VkDevice device,
    const char *code,
    const uint32_t size,
    VkShaderModule *shader_module)
{
    const VkShaderModuleCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = size,
        .pCode = (const uint32_t *)code,
    };

    if (vkCreateShaderModule(device, &create_info, NULL, shader_module) != VK_SUCCESS) {
        fputs("Error: failed to create shader module!\n", stderr);
        return APP_ERROR;
    }

    return APP_SUCCESS;
}

enum app_result create_graphics_pipeline(struct application *app)
{
    uint32_t vert_shader_size, frag_shader_size;
    char *vert_shader_code;
    char *frag_shader_code;

    read_file("./bin/vert.spv", &vert_shader_code, &vert_shader_size);
    read_file("./bin/frag.spv", &frag_shader_code, &frag_shader_size);

    VkShaderModule vert_shader_module, frag_shader_module;
    create_shader_module(app->device, vert_shader_code, vert_shader_size, &vert_shader_module);
    create_shader_module(app->device, frag_shader_code, frag_shader_size, &frag_shader_module);

    const VkPipelineShaderStageCreateInfo vert_shader_stage_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vert_shader_module,
        .pName = "main",
    };

    const VkPipelineShaderStageCreateInfo frag_shader_stage_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = frag_shader_module,
        .pName = "main",
    };

    const VkPipelineShaderStageCreateInfo shader_stages[] = {
        vert_shader_stage_info,
        frag_shader_stage_info,
    };

    const VkPipelineVertexInputStateCreateInfo vertex_input_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 0,
        .vertexAttributeDescriptionCount = 0,
    };

    const VkPipelineInputAssemblyStateCreateInfo input_assembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float)app->swapchain_extent.width,
        .height = (float)app->swapchain_extent.height,
        .minDepth= 0.0f,
        .maxDepth = 1.0f,
    };

    VkRect2D scissor = {
        .offset = {
            .x = 0,
            .y = 0,
        },
        .extent = app->swapchain_extent,
    };

    const uint32_t dynamic_state_count = 2;
    const VkDynamicState dynamic_states[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

    const VkPipelineDynamicStateCreateInfo dynamic_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = dynamic_state_count,
        .pDynamicStates = dynamic_states,
    };

    const VkPipelineViewportStateCreateInfo viewport_state = {
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

    const VkPipelineColorBlendAttachmentState color_blend_attachment = {
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

    const VkPipelineColorBlendStateCreateInfo color_blending = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY, // optional
        .attachmentCount = 1,
        .pAttachments = &color_blend_attachment,
        .blendConstants = { 0.0f, 0.0f, 0.0f, 0.0f },
    };

    const VkPipelineLayoutCreateInfo pipeline_layout_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0,
        .pushConstantRangeCount = 0,
    };

    if (vkCreatePipelineLayout(app->device, &pipeline_layout_info, NULL, &app->pipeline_layout)
        != VK_SUCCESS)
    {
        fputs("Error: failed to create pipeline layout!\n", stderr);
        return APP_ERROR;
    }

    const VkGraphicsPipelineCreateInfo pipeline_info = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shader_stages,
        .pVertexInputState = &vertex_input_info,
        .pInputAssemblyState = &input_assembly,
        .pViewportState = &viewport_state,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = NULL, // optional
        .pColorBlendState = &color_blending,
        .pDynamicState = &dynamic_state,
        .layout = app->pipeline_layout,
        .renderPass = app->render_pass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE, // optional
        .basePipelineIndex = -1,              // optional
    };

    if (vkCreateGraphicsPipelines(
        app->device, NULL, 1, &pipeline_info, NULL, &app->graphics_pipeline)
        != VK_SUCCESS)
    {
        fputs("Error: failed to create graphics pipeline!\n", stderr);
        return APP_ERROR;
    }

    vkDestroyShaderModule(app->device, vert_shader_module, NULL);
    vkDestroyShaderModule(app->device, frag_shader_module, NULL);
    free(vert_shader_code);
    free(frag_shader_code);
    return APP_SUCCESS;
}
