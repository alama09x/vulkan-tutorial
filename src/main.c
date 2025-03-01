#include "application.h"
#include <stdlib.h>

int main()
{
    // TODO: why did I allocate `app` on the heap?
    Application* app = malloc(sizeof(*app));
    appRun(app);
    free(app);
    return EXIT_SUCCESS;
}
