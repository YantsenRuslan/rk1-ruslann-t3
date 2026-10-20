#!/bin/bash
#SBATCH --job-name=t3-omp
#SBATCH --partition=compute           # раздел из sinfo
#SBATCH --exclude=pbgpu01             # без GPU-узла: у него другой процессор
#SBATCH --nodes=1                     # OpenMP работает в пределах одного узла
#SBATCH --ntasks=1                    # один процесс...
#SBATCH --cpus-per-task=32            # ...которому выдаётся 32 ядра под потоки
#SBATCH --exclusive                   # узел целиком наш: замеры без чужих задач
#SBATCH --mem=0                       # вся память узла (иначе DefMemPerCPU x 96 > 256 ГБ)
#SBATCH --time=00:30:00
#SBATCH --output=logs/omp_%j.out

# Запускать из корня репозитория: sbatch slurm/omp.sbatch
# Бинарники собираются ЗАРАНЕЕ на входном узле (на вычислительных нет заголовков C):
#   gcc -O3 src/seq.c -o seq -lm
#   gcc -O3 -fopenmp src/omp.c -o omp -lm

echo "# node: $(hostname)"
echo "# compiler: $(gcc --version | head -1)"
lscpu | sed 's/^/# /'                 # характеристики узла попадают в лог с пометкой #

export OMP_PROC_BIND=close            # потоки привязаны к ядрам, не мигрируют
export OMP_PLACES=cores

# эталон T1: 3 запуска последовательной версии
for r in 1 2 3; do
    ./seq
done

# OpenMP: 1..32 потока, по 3 запуска на каждое значение
for p in 1 2 4 8 16 32; do
    for r in 1 2 3; do
        OMP_NUM_THREADS=$p ./omp
    done
done
