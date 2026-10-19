#include <stdint.h>

uint32_t __udivsi3(uint32_t dividend, uint32_t divisor)
{
    if (divisor == 0)
        return 0;

    uint32_t quotient = 0;
    uint32_t bit = 1;

    while ((divisor < dividend) && ((divisor & 0x80000000u) == 0))
    {
        divisor <<= 1;
        bit <<= 1;
    }

    while (bit)
    {
        if (dividend >= divisor)
        {
            dividend -= divisor;
            quotient |= bit;
        }

        divisor >>= 1;
        bit >>= 1;
    }

    return quotient;
}
