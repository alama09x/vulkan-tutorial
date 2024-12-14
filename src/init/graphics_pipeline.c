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
    printf("size: %d\n", *size);

    *code = malloc(*size * sizeof(char));
    rewind(f);

    fread(*code, 1, *size, f);

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
    char *vert_shader_code, *frag_shader_code;

    read_file("./bin/vert.spv", &vert_shader_code, &vert_shader_size);
    read_file("./bin/frag.spv", &frag_shader_code, &vert_shader_size);

    VkShaderModule vert_shader_module, frag_shader_module;
    create_shader_module(app->device, vert_shader_code, vert_shader_size, &vert_shader_module);
    create_shader_module(app->device, frag_shader_code, frag_shader_size, &frag_shader_module);

    vkDestroyShaderModule(app->device, vert_shader_module, NULL);
    vkDestroyShaderModule(app->device, frag_shader_module, NULL);
    free(vert_shader_code);
    free(frag_shader_code);
    return APP_SUCCESS;
}
