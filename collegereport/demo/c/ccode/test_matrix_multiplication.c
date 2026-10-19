#define M 4   // Rows of A
#define N 4   // Columns of A = Rows of B
#define P 4   // Columns of B

void matmul(
    const int A[M][N],
    const int B[N][P],
    int C[M][P]
)
{
    int i, j, k;

    for (i = 0; i < M; i++) {
        for (j = 0; j < P; j++) {
            int sum = 0;

            for (k = 0; k < N; k++) {
                sum += A[i][k] * B[k][j];
            }

            C[i][j] = sum;
        }
    }
}

int C[M][P];
int main(void)
{
    int A[M][N] = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9,10,11,12},
        {13,14,15,16}
    };

    int B[N][P] = {
        {16,15,14,13},
        {12,11,10, 9},
        { 8, 7, 6, 5},
        { 4, 3, 2, 1}
    };


    matmul(A, B, C);

    return 1;
}
