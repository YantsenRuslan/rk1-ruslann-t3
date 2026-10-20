/* omp.c — OpenMP-версия (трек 3: schedule(static))
 * Сборка:  gcc -O3 -fopenmp omp.c -o omp -lm
 * Запуск:  OMP_NUM_THREADS=8 OMP_PROC_BIND=close OMP_PLACES=cores ./omp
 *          -> omp;8;<время>;<OK|FAIL>
 *
 * Идея распараллеливания:
 *  - Внешний цикл по строкам i делится между потоками статически
 *    (schedule(static)): поток k получает непрерывную полосу строк.
 *    Работа на каждую строку одинаковая, поэтому static даёт идеальный
 *    баланс без накладных расходов на раздачу итераций.
 *  - Гонок нет: каждый поток пишет только в свои строки unew,
 *    а читает из u, который в этой итерации никто не меняет.
 *  - Один parallel-регион на все 500 итераций: потоки создаются один раз.
 *    Обмен указателей делает один поток (single), неявный барьер после
 *    single гарантирует, что все увидят новые указатели.
 *  - First touch: массивы заполняются тем же разбиением schedule(static),
 *    поэтому страницы памяти оказываются на NUMA-узле того потока,
 *    который потом с ними работает.
 */
#include <string.h>
#include <omp.h>
#include "heat.h"

static void init_parallel(double *u) {
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            u[IDX(i, j)] = (i == 0) ? TOP_T : 0.0;
}

int main(int argc, char **argv) {
    double *u    = malloc(sizeof(double) * N * N);
    double *unew = malloc(sizeof(double) * N * N);
    if (!u || !unew) { fprintf(stderr, "malloc failed\n"); return 1; }

    init_parallel(u);           /* first touch тем же разбиением, что и расчёт */
    init_parallel(unew);

    int p = omp_get_max_threads();
    double t0 = omp_get_wtime();

    #pragma omp parallel
    {
        for (int it = 0; it < ITERS; it++) {
            #pragma omp for schedule(static)
            for (int i = 1; i < N - 1; i++)
                for (int j = 1; j < N - 1; j++)
                    unew[IDX(i, j)] = 0.25 * (u[IDX(i - 1, j)] + u[IDX(i + 1, j)]
                                            + u[IDX(i, j - 1)] + u[IDX(i, j + 1)]);
            /* неявный барьер после omp for: все строки посчитаны */

            #pragma omp single
            { double *tmp = u; u = unew; unew = tmp; }
            /* неявный барьер после single: новые указатели видны всем */
        }
    }

    double t = omp_get_wtime() - t0;

    double cs = checksum(u);    /* вне замера, последовательно, как в seq */
    printf("omp;%d;%.6f;%s\n", p, t, check(cs));
    if (argc > 1 && strcmp(argv[1], "-v") == 0)
        fprintf(stderr, "checksum=%.17g\n", cs);

    free(u); free(unew);
    return 0;
}
