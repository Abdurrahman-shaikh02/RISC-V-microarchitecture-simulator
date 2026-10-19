int fib(int n)
{
    if (n == 0)
        return 0;
    if (n == 1)
        return 1;

    return fib(n - 1) + fib(n - 2);
}

int main()
{
    int n = 20;
    int result = fib(n);

    // result now contains 5

    return result;
}
