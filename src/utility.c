#include "utility.h"

void clamp(int* pVal, int min, int max)
{
    if (*pVal < min) {
        *pVal = min;
    } else if (*pVal > max) {
        *pVal = max;
    }
}
