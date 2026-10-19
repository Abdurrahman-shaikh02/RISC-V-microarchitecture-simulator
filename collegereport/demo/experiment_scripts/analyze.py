import json, os
import matplotlib; matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

E = os.path.dirname(os.path.abspath(__file__))
FIG = os.path.join(os.path.dirname(E), "fig")
R = json.load(open(os.path.join(E, "results.json")))
SP = R["spikes"]
WL = ["fib", "qsort", "radix", "matmul", "bubble", "coremark"]
WLN = {"fib": "fib(20)", "qsort": "quicksort", "radix": "radix sort", "matmul": "matmul 24²",
       "bubble": "bubble sort", "coremark": "CoreMark"}
BLUE, ORANGE, AQUA = "#2a78d6", "#eb6834", "#1baf7a"
INK, SEC, MUTED, GRID, SURF = "#0b0b0b", "#52514e", "#8a8984", "#e6e5e1", "#ffffff"
plt.rcParams.update({"font.family": "DejaVu Sans", "font.size": 9, "axes.edgecolor": MUTED,
                     "axes.labelcolor": SEC, "xtick.color": SEC, "ytick.color": SEC,
                     "axes.spines.top": False, "axes.spines.right": False,
                     "axes.grid": True, "grid.color": GRID, "grid.linewidth": 0.6,
                     "axes.axisbelow": True, "legend.frameon": False})


def ref(wl):
    return SP[wl]["native"] if SP[wl]["native"] is not None else SP[wl]["a0"]


def get(res, exp, wl):
    for s in res["sims"]:
        if s["exp"] == exp and s["wl"] == wl:
            return s


def cpi(s):
    st = s["stats"]
    return st["Total Cycles"] / st["Instructions Retired"] if st.get("Instructions Retired") else None


def ok(s):
    return s["rc"] == 0 and s["a0"] is not None and (s["a0"] & 0xffffffff) == (ref(s["wl"]) & 0xffffffff)


N = {}
# correctness summary
N["runs"] = len(R["sims"])
N["runs_ok"] = sum(ok(s) for s in R["sims"])

# E1 validation table
N["validation"] = []
for wl in WL:
    s = get(R, "default", wl); st = s["stats"]; sp = SP[wl]
    N["validation"].append(dict(wl=WLN[wl], expected=ref(wl) & 0xffffffff, got=s["a0"], ok=ok(s),
                                ins=st["Instructions Retired"], spike_ins=sp["insns"],
                                reads=st["Data Reads"], spike_reads=sp["D$ Read Accesses"],
                                writes=st["Data Writes"], spike_writes=sp["D$ Write Accesses"]))

# E2 forwarding (original + patched), ideal memory
N["fwd"] = []
for wl in WL:
    a, b = get(R, "fwd1", wl), get(R, "fwd0", wl)
    N["fwd"].append(dict(wl=WLN[wl], on=cpi(a), off=cpi(b),
                         speedup=b["stats"]["Total Cycles"] / a["stats"]["Total Cycles"],
                         fex=a["stats"]["Forwards to EX"], fma=a["stats"]["Forwards to MA"]))
fig, ax = plt.subplots(figsize=(6.6, 2.9))
x = np.arange(len(WL)); w = 0.27
for i, (key, lab, col) in enumerate([("off", "Forwarding OFF (stall on hazard)", ORANGE),
                                      ("on", "Forwarding ON", BLUE)]):
    vals = [r[key] for r in N["fwd"]]
    bars = ax.bar(x + (i - 0.5) * w * 1.25, vals, w * 1.25 - 0.04, color=col, label=lab)
    for xx, v in zip(x + (i - 0.5) * w * 1.25, vals):
        ax.text(xx, v + 0.03, f"{v:.2f}", ha="center", fontsize=6.5, color=SEC)
ax.axhline(1.0, color=MUTED, lw=1, ls="--"); ax.text(-0.5, 0.88, "ideal CPI = 1", fontsize=7, color=SEC, ha="left", va="top")
ax.set_xticks(x, [WLN[w_] for w_ in WL]); ax.set_ylabel("CPI (ideal memory)"); ax.set_ylim(0, 3.4)
ax.legend(fontsize=7.5, loc="upper left", ncol=1)
plt.tight_layout(); plt.savefig(os.path.join(FIG, "fig_forwarding.png"), dpi=200); plt.close()

# E3 branch prediction (patched simulator), ideal memory
N["bp"] = []
for wl in WL:
    row = dict(wl=WLN[wl])
    for k in ("bp0", "bp1", "bp2"):
        s = get(R, k, wl); st = s["stats"]
        row[k + "_acc"] = st["Branch Hits"] / st["Branch Instructions"]
        row[k + "_cpi"] = cpi(s)
        row[k + "_ok"] = ok(s)
    st = get(R, "bp0", wl)["stats"]
    row["taken_frac"] = st["Branches Taken"] / st["Branch Instructions"]
    row["br_frac"] = st["Branch Instructions"] / st["Instructions Retired"]
    N["bp"].append(row)
fig, ax = plt.subplots(figsize=(6.6, 2.9))
for i, (key, lab, col) in enumerate([("bp0_acc", "Always not-taken", ORANGE),
                                      ("bp1_acc", "Always taken", BLUE),
                                      ("bp2_acc", "BTFN (backward taken / forward not)", AQUA)]):
    vals = [100 * r[key] for r in N["bp"]]
    ax.bar(x + (i - 1) * w, vals, w - 0.03, color=col, label=lab)
    for xx, v in zip(x + (i - 1) * w, vals):
        ax.text(xx, v + 1.5, f"{v:.0f}", ha="center", fontsize=6.5, color=SEC)
ax.set_xticks(x, [WLN[w_] for w_ in WL]); ax.set_ylabel("Prediction accuracy (%)"); ax.set_ylim(0, 118)
ax.legend(fontsize=7.5, loc="upper center", ncol=3, bbox_to_anchor=(0.5, 1.13))
plt.tight_layout(); plt.savefig(os.path.join(FIG, "fig_branch.png"), dpi=200); plt.close()

# E4 L1D sweep vs spike (direct-mapped and 2-way)
SIZES = [256, 512, 1024, 2048, 4096, 8192]
N["l1d"] = {}
fig, axs = plt.subplots(2, 2, figsize=(6.6, 4.6), sharex=True)
for ax, wl in zip(axs.flat, ["qsort", "radix", "matmul", "coremark"]):
    rows = []
    for size in SIZES:
        r = dict(size=size)
        for assoc in (1, 2, 4):
            for rp in (0, 1):
                st = get(R, f"l1d_{size}_{assoc}_{rp}", wl)["stats"]
                acc = st["L1 D Read Hits"] + st["L1 D Write Hits"] + st["L1 D Read Misses"] + st["L1 D Write Misses"]
                r[f"sim_{assoc}_{rp}"] = 100 * (st["L1 D Read Misses"] + st["L1 D Write Misses"]) / acc
                r[f"cyc_{assoc}_{rp}"] = st["Total Cycles"]
            ss = SP[wl]["dc_sweep"][f"{size}_{assoc}"]
            r[f"spike_{assoc}"] = 100 * (ss["D$ Read Misses"] + ss["D$ Write Misses"]) / (ss["D$ Read Accesses"] + ss["D$ Write Accesses"])
        rows.append(r)
    N["l1d"][wl] = rows
    xs = [r["size"] for r in rows]
    ax.plot(xs, [r["sim_1_0"] for r in rows], color=BLUE, lw=2, marker="o", ms=4, label="This simulator, direct-mapped")
    ax.plot(xs, [r["spike_1"] for r in rows], color=ORANGE, lw=0, marker="x", ms=7, mew=1.6, label="Spike cache model, direct-mapped")
    ax.plot(xs, [r["sim_2_1"] for r in rows], color=AQUA, lw=2, ls="--", marker="s", ms=3.5, label="This simulator, 2-way LRU")
    ax.set_xscale("log", base=2); ax.set_xticks(SIZES, ["256B", "512B", "1K", "2K", "4K", "8K"])
    ax.set_title(WLN[wl], fontsize=9, color=INK, loc="left"); ax.set_ylim(bottom=0)
for ax in axs[:, 0]: ax.set_ylabel("L1-D miss rate (%)")
for ax in axs[1, :]: ax.set_xlabel("L1-D capacity (16-byte blocks)")
h, l = axs[0, 0].get_legend_handles_labels()
fig.legend(h, l, loc="upper center", ncol=3, fontsize=7.5, bbox_to_anchor=(0.5, 1.02))
plt.tight_layout(rect=(0, 0, 1, 0.95)); plt.savefig(os.path.join(FIG, "fig_l1d.png"), dpi=200); plt.close()
# agreement stats for direct-mapped
dm = [abs(r["sim_1_0"] - r["spike_1"]) for wl in N["l1d"] for r in N["l1d"][wl]]
N["dm_max_abs_diff"] = max(dm); N["dm_mean_abs_diff"] = sum(dm) / len(dm)
sa = [abs(r[f"sim_{a}_{p}"] - r[f"spike_{a}"]) for wl in N["l1d"] for r in N["l1d"][wl] for a in (2, 4) for p in (0, 1)]
N["sa_mean_abs_diff"] = sum(sa) / len(sa); N["sa_max_abs_diff"] = max(sa)
lru_better = [(r[f"sim_{a}_0"] - r[f"sim_{a}_1"]) for wl in N["l1d"] for r in N["l1d"][wl] for a in (2, 4)]
N["lru_vs_fifo"] = dict(lru_better=sum(d > 0.005 for d in lru_better), equal=sum(abs(d) <= 0.005 for d in lru_better),
                        fifo_better=sum(d < -0.005 for d in lru_better), n=len(lru_better))

# E5 hierarchy depth
N["levels"] = []
for wl in WL:
    row = dict(wl=WLN[wl], ideal=cpi(get(R, "fwd1", wl)))
    for lv in range(4):
        row[f"l{lv}"] = cpi(get(R, f"levels{lv}", wl))
    N["levels"].append(row)
fig, ax = plt.subplots(figsize=(6.6, 2.9))
w4 = 0.2
for i, (key, lab, col) in enumerate([("l1", "L1 only + DRAM", ORANGE), ("l2", "L1 + L2 + DRAM", BLUE),
                                      ("l3", "L1 + L2 + L3 + DRAM (default)", AQUA)]):
    vals = [r[key] for r in N["levels"]]
    ax.bar(x + (i - 1) * w, vals, w - 0.03, color=col, label=lab)
    for xx, v in zip(x + (i - 1) * w, vals):
        ax.text(xx, v * 1.08, f"{v:.1f}", ha="center", fontsize=6.5, color=SEC)
ax.plot(x, [r["ideal"] for r in N["levels"]], lw=0, marker="_", ms=22, mew=2, color=INK, label="Ideal memory (0 stalls)")
ax.set_yscale("log"); ax.set_ylim(0.8, 80); ax.set_yticks([1, 2, 5, 10, 20, 50], ["1", "2", "5", "10", "20", "50"])
ax.set_xticks(x, [WLN[w_] for w_ in WL]); ax.set_ylabel("CPI (log scale)")
ax.legend(fontsize=7.2, loc="upper center", ncol=4, bbox_to_anchor=(0.5, 1.14))
plt.tight_layout(); plt.savefig(os.path.join(FIG, "fig_levels.png"), dpi=200); plt.close()

# E6 write policy
N["wp"] = []
for wl in WL:
    a, b = get(R, "wp0", wl), get(R, "wp1", wl)
    N["wp"].append(dict(wl=WLN[wl], wt=cpi(a), wb=cpi(b), wt_wstall=a["stats"]["Data Write Stalls"],
                        wb_wstall=b["stats"]["Data Write Stalls"],
                        gain=a["stats"]["Total Cycles"] / b["stats"]["Total Cycles"]))

# CoreMark
cm = lambda s: 10 * 1e6 / s["stats"]["Total Cycles"]
N["coremark"] = dict(default=cm(get(R, "default", "coremark")), ideal_nt=cm(get(R, "fwd1", "coremark")),
                     ideal_btfn=cm(get(R, "bp2", "coremark")),
                     wb=cm(get(R, "wp1", "coremark")), ins=get(R, "default", "coremark")["stats"]["Instructions Retired"])
json.dump(N, open(os.path.join(E, "numbers.json"), "w"), indent=1)
print(json.dumps({k: N[k] for k in ("runs", "runs_ok", "dm_max_abs_diff",
                                    "dm_mean_abs_diff", "sa_mean_abs_diff", "sa_max_abs_diff", "lru_vs_fifo", "coremark")}, indent=1))
for k in ("fwd", "bp", "levels", "wp"):
    for r in N[k]: print(k, {a: (round(b, 3) if isinstance(b, float) else b) for a, b in r.items()})
