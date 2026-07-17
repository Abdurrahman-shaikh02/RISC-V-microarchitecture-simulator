int __mulsi3(int a, int b)
{
    int result = 0;

    while (b) {
        if (b & 1)
            result += a;

        a <<= 1;
        b >>= 1;
    }

    return result;
}
