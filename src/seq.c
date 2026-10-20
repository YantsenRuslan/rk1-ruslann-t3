/* seq.c — последовательная версия (эталон результата и время T1)
 * Сборка:  gcc -O3 seq.c -o seq -lm
 * Запуск:  ./seq        -> seq;1;<время>;<OK|FAIL>
 *          ./seq -v     -> плюс контрольная сумма (для получения эталона)
 */
#include <string.h>
#include <time.h>
#include "heat.h"

static double wtime(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec * 1e-9;
}

static void init(double *u) {
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            u[IDX(i, j)] = (i == 0) ? TOP_T : 0.0;
}

int main(int argc, char **argv) {
    double *u    = malloc(sizeof(double) * N * N);
    double *unew = malloc(sizeof(double) * N * N);
    if (!u || !unew) { fprintf(stderr, "malloc failed\n"); return 1; }

    init(u);
    init(unew);                 /* границы есть в обоих массивах */

    double t0 = wtime();        /* замеряем только вычислительное ядро */
    for (int it = 0; it < ITERS; it++) {
        for (int i = 1; i < N - 1; i++)
            for (int j = 1; j < N - 1; j++)
                unew[IDX(i, j)] = 0.25 * (u[IDX(i - 1, j)] + u[IDX(i + 1, j)]
                                        + u[IDX(i, j - 1)] + u[IDX(i, j + 1)]);
        double *tmp = u; u = unew; unew = tmp;   /* обмен указателей, без копирования */
    }
    double t = wtime() - t0;

    double cs = checksum(u);
    printf("seq;1;%.6f;%s\n", t, check(cs));
    if (argc > 1 && strcmp(argv[1], "-v") == 0)
        fprintf(stderr, "checksum=%.17g\n", cs);

    free(u); free(unew);
    return 0;
}
