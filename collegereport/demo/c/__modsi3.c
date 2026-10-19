#include <stdint.h>

extern uint32_t __umodsi3(uint32_t, uint32_t);

// signed remainder: the result takes the sign of the dividend (C truncating division)
int32_t __modsi3(int32_t a, int32_t b)
{
    if (b == 0)
        return a;	// matches the RISC-V rem instruction: x % 0 = x

    int negative = 0;

    uint32_t ua = (uint32_t)a;
    uint32_t ub = (uint32_t)b;

    if (a < 0)
    {
        ua = (uint32_t)(-a);
        negative = 1;
    }

    if (b < 0)
    {
        ub = (uint32_t)(-b);
    }

    uint32_t r = __umodsi3(ua, ub);

    return negative ? -(int32_t)r : (int32_t)r;
}
