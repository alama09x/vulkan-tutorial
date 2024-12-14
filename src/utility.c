#include "utility.h"

void clamp(int *val, int min, int max)
{
    if (*val < min) {
        *val = min;
    } else if (*val > max) {
        *val = max;
    }
}
