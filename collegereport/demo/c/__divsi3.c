#include <stdint.h>

extern uint32_t __udivsi3(uint32_t, uint32_t);
// Caveat: the signed division implementation above assumes normal two's-complement arithmetic.
// The edge case INT_MIN / -1 is undefined behavior in C anyway, and GCC's runtime handles it with extra care.
// For CoreMark and typical embedded workloads, this simpler implementation is perfectly adequate.
int32_t __divsi3(int32_t a, int32_t b)
{
    if (b == 0)
        return 0;

    int negative = 0;

    uint32_t ua = (uint32_t)a;
    uint32_t ub = (uint32_t)b;

    if (a < 0)
    {
        ua = (uint32_t)(-a);
        negative ^= 1;
    }

    if (b < 0)
    {
        ub = (uint32_t)(-b);
        negative ^= 1;
    }

    uint32_t q = __udivsi3(ua, ub);

    return negative ? -(int32_t)q : (int32_t)q;
}
