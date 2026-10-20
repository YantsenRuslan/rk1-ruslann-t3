/* heat.h — общие параметры и функции для трека 3
 * 2D теплопроводность, метод Якоби, сетка N x N, ITERS итераций.
 *
 * Граничные условия (фиксированные, Дирихле):
 *   верхняя граница  (i = 0)      = 100.0
 *   остальные границы              = 0.0
 *   внутренние точки в начале      = 0.0
 *
 * Шаг Якоби для внутренней точки:
 *   new[i][j] = 0.25 * (old[i-1][j] + old[i+1][j] + old[i][j-1] + old[i][j+1])
 */
#ifndef HEAT_H
#define HEAT_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef N
#define N 4096          /* размер сетки вместе с граничными строками/столбцами */
#endif
#ifndef ITERS
#define ITERS 500
#endif

#define TOP_T 100.0
#define IDX(i, j) ((size_t)(i) * N + (j))

/* Эталонная контрольная сумма для N=4096, ITERS=500 (получена seq.c).
 * Каждая ячейка считается по одной и той же формуле независимо от числа
 * потоков, поэтому сетка совпадает с seq бит в бит. */
#define REF_CHECKSUM 4949330.8767278139
#define REL_TOL 1e-9

/* Контрольная сумма: последовательная сумма внутренних точек.
 * Считается ВНЕ замера времени, всегда в одном порядке. */
static double checksum(const double *u) {
    double s = 0.0;
    for (int i = 1; i < N - 1; i++)
        for (int j = 1; j < N - 1; j++)
            s += u[IDX(i, j)];
    return s;
}

static const char *check(double cs) {
    if (N != 4096 || ITERS != 500) return "OK(no-ref)";
    return fabs(cs - REF_CHECKSUM) <= REL_TOL * fabs(REF_CHECKSUM) ? "OK" : "FAIL";
}

#endif
