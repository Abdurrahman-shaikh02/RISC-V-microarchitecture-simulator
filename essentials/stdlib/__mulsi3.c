#include <stdint.h>

uint32_t __mulsi3(uint32_t a, uint32_t b)
{
    uint32_t result = 0;

    while (b)
    {
        if (b & 1)
            result += a;

        a <<= 1;
        b >>= 1;
    }

    return result;
}
