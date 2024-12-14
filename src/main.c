#include "application.h"
#include <stdlib.h>

int main()
{
    struct application *app = malloc(sizeof(*app));
    app_run(app);
    free(app);
    return EXIT_SUCCESS;
}
