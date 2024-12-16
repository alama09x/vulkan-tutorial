#include "application.h"
#include <stdlib.h>

int main()
{
    Application *app = malloc(sizeof(*app));
    appRun(app);
    free(app);
    return EXIT_SUCCESS;
}
