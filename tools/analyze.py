#!/usr/bin/env python3
"""analyze.py — разбор логов Slurm для трека 3 (OpenMP).

Читает строки вида  version;p;time_s;CHECK  из logs/*.out,
берёт минимум из запусков для каждого p, считает
  S = T1 / T(p),  E = S / p,  e (Карп–Флатт) = (1/S - 1/p) / (1 - 1/p),
сохраняет plots/speedup.png, plots/efficiency.png и results.json.

Запуск:  python3 tools/analyze.py logs/omp_12345.out
"""
import json, sys, glob, os
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

files = sys.argv[1:] or sorted(glob.glob("logs/omp_*.out"))
if not files:
    sys.exit("нет логов: укажите путь к logs/omp_<jobid>.out")

runs, node = {}, []
for f in files:
    for line in open(f, encoding="utf-8", errors="replace"):
        line = line.strip()
        if line.startswith("#"):
            node.append(line[1:].strip())
            continue
        parts = line.split(";")
        if len(parts) != 4:
            continue
        ver, p, t, chk = parts[0], int(parts[1]), float(parts[2]), parts[3]
        runs.setdefault((ver, p), []).append((t, chk))

bad = [k for k, v in runs.items() if any(c != "OK" for _, c in v)]
if bad:
    print("ВНИМАНИЕ: есть запуски без CHECK OK:", bad)

T1 = min(t for t, _ in runs[("seq", 1)])
rows = []
for (ver, p), v in sorted(runs.items(), key=lambda kv: (kv[0][0] != "seq", kv[0][1])):
    if ver != "omp":
        continue
    tp = min(t for t, _ in v)
    S = T1 / tp
    E = S / p
    e = (1 / S - 1 / p) / (1 - 1 / p) if p > 1 else None
    rows.append(dict(p=p, runs=[t for t, _ in v], T=tp, S=S, E=E, e=e,
                     check="OK" if all(c == "OK" for _, c in v) else "FAIL"))

print(f"T1 (seq, min из {len(runs[('seq', 1)])}) = {T1:.3f} c")
print(f"{'p':>3} {'T(p), c':>9} {'S':>6} {'E':>6} {'e K-F':>8}  CHECK")
for r in rows:
    e = f"{r['e']:.4f}" if r["e"] is not None else "—"
    print(f"{r['p']:>3} {r['T']:>9.3f} {r['S']:>6.2f} {r['E']:>6.2f} {e:>8}  {r['check']}")

os.makedirs("plots", exist_ok=True)
ps = [r["p"] for r in rows]

plt.figure(figsize=(6, 4.2))
plt.plot(ps, ps, "--", color="gray", label="идеал S = p")
plt.plot(ps, [r["S"] for r in rows], "o-", color="#2E5C9A", label="OpenMP, schedule(static)")
plt.xscale("log", base=2); plt.yscale("log", base=2)
plt.xticks(ps, ps); plt.yticks(ps, ps)
plt.xlabel("число потоков p"); plt.ylabel("ускорение S(p)")
plt.title("Трек 3 · Якоби 4096², 500 итераций")
plt.grid(alpha=.3); plt.legend(); plt.tight_layout()
plt.savefig("plots/speedup.png", dpi=150); plt.close()

plt.figure(figsize=(6, 4.2))
plt.plot(ps, [1] * len(ps), "--", color="gray", label="идеал E = 1")
plt.plot(ps, [r["E"] for r in rows], "o-", color="#C2410C", label="OpenMP, schedule(static)")
plt.xscale("log", base=2); plt.xticks(ps, ps)
plt.ylim(0, max(1.2, max(r["E"] for r in rows) * 1.1))
plt.xlabel("число потоков p"); plt.ylabel("эффективность E(p)")
plt.title("Трек 3 · эффективность")
plt.grid(alpha=.3); plt.legend(); plt.tight_layout()
plt.savefig("plots/efficiency.png", dpi=150); plt.close()

json.dump(dict(T1=T1, seq_runs=[t for t, _ in runs[("seq", 1)]], rows=rows,
               node=node, logs=files), open("results.json", "w", encoding="utf-8"),
          ensure_ascii=False, indent=1)
print("сохранено: plots/speedup.png, plots/efficiency.png, results.json")
