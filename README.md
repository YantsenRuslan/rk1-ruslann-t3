# РК1 · Трек 3 — 2D теплопроводность, метод Якоби (OpenMP)

Сетка 4096×4096, 500 итераций. Верхняя граница = 100, остальное = 0.
OpenMP: ленты по строкам, `schedule(static)`, один parallel-регион, first touch.

## Структура
```
src/     heat.h seq.c omp.c        код (heat.h — параметры, контрольная сумма, CHECK)
slurm/   omp.sbatch                серия замеров: seq ×3, omp p=1..32 ×3
logs/    *.out                     логи Slurm без правок
plots/   speedup.png efficiency.png
tools/   analyze.py make_report.py разбор логов и сборка отчёта
```

## Запуск на ПАРАМ БИЛИМ
```bash
sinfo
```
Посмотреть разделы. Используется `compute`, узел целиком (`--exclusive --mem=0`).
```bash
gcc -O3 src/seq.c -o seq -lm
gcc -O3 -fopenmp src/omp.c -o omp -lm
```
Собрать программы на входном узле bilim-master (на вычислительных узлах нет заголовков C).
```bash
sbatch slurm/omp.sbatch
```
Поставить серию замеров в очередь (из корня репозитория).
```bash
squeue -u $USER
```
Проверить, что задача в очереди или выполняется.
```bash
python3 tools/analyze.py logs/omp_<jobid>.out
```
Посчитать S, E, Карпа–Флатта (минимум из 3 запусков) и построить графики.
```bash
python3 tools/make_report.py
```
Собрать отчёт `report_t3.docx` из `results.json` и графиков.

## Формат вывода
Одна строка на запуск: `version;p;time_s;CHECK`, например `omp;8;2.345678;OK`.
CHECK = сумма внутренних точек совпадает с эталоном seq.c (4949330.8767278139, допуск 1e-9).
`./seq -v` или `./omp -v` дополнительно печатает контрольную сумму.

## Результат (задача 2440, узел pbcn01, 2 × Xeon Gold 6336Y)
T1 = 9.109 c; на 32 потоках T = 0.678 c, S = 13.44, E = 0.42. Все запуски CHECK: OK.
