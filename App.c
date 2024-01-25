#include "App.h"
#include "instance.h"
#include "validation.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

int run(App *app)
{
    initWindow(app);
    initVulkan(app);
    mainLoop(app);
    cleanup(app);
    return 0;
}

int initWindow(App *app)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    app->window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", NULL, NULL);
    return 0;
}

int initVulkan(App *app)
{
    createInstance(app);
    setupDebugMessenger(app);
    return EXIT_SUCCESS;
}

int mainLoop(App *app)
{
    while (!glfwWindowShouldClose(app->window))
        glfwPollEvents();

    return 0;
}

int cleanup(App *app)
{
    cleanupValidation(app);
    cleanupInstance(app);

    glfwDestroyWindow(app->window);
    glfwTerminate();

    return 0;
}
