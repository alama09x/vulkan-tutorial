#include "application.h"
#include <stdlib.h>

int main()
{
    struct application *app = malloc(APPLICATION_STRUCT_SIZE);
    app_run(app);
    free(app);
    return EXIT_SUCCESS;
}
