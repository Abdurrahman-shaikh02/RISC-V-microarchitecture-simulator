#!/usr/bin/env python3
"""Experiment driver for the RV32I pipeline simulator report."""
import json, os, re, subprocess, sys, time, copy, tempfile
from concurrent.futures import ProcessPoolExecutor

P = "/home/abdurrahman/Documents/projects/compute_engine"
E = os.path.dirname(os.path.abspath(__file__))
SIM = os.environ.get("SIM", os.path.join(P, "simulate"))
OUT = os.environ.get("OUT", "results.json")
SUBSET = os.environ.get("SUBSET")
BIN = os.path.join(E, "bin")
RUNS = os.path.join(E, "runs")
BASE = json.load(open(os.path.join(P, "essentials/template.json")))
WORKLOADS = ["fib", "qsort", "radix", "matmul", "bubble", "coremark"]


def cfg(**over):
    c = copy.deepcopy(BASE)
    for k, v in over.items():
        if k in ("l1_d", "l1_i", "l2", "l3"):
            c[k].update(v)
        else:
            c[k] = v
    return c


def parse_stats(text):
    d = {}
    for line in text.splitlines():
        m = re.match(r"\s*([A-Za-z0-9 ]+?)\s*:\s*(\d+)\s*$", line)
        if m:
            d[m.group(1).strip()] = int(m.group(2))
    return d


def run_sim(args):
    name, wl, c = args
    d = tempfile.mkdtemp(dir=RUNS, prefix=f"{name}_{wl}_")
    cp = os.path.join(d, "cfg.json")
    json.dump(c, open(cp, "w"))
    t0 = time.time()
    r = subprocess.run([SIM, "-e", os.path.join(BIN, wl + ".elf"), "-r", "-c", cp],
                       cwd=d, capture_output=True, text=True, timeout=int(os.environ.get("TMO","3600")))
    dt = time.time() - t0
    st = parse_stats(r.stdout)
    a0 = None
    rd = os.path.join(d, "register_dump")
    if os.path.exists(rd):
        for line in open(rd):
            if line.startswith("r10"):
                a0 = int(line.split(":")[1], 16)
    return dict(exp=name, wl=wl, cfg=c, stats=st, a0=a0, wall=dt, rc=r.returncode,
                err=r.stderr[-500:])


def run_spike(wl, ic="64:2:16", dc="64:2:16", log=True):
    elf = os.path.join(BIN, wl + ".spike.elf")
    logf = os.path.join(RUNS, f"spike_{wl}.log")
    cmd = ["spike", "--isa=rv32i", f"--ic={ic}", f"--dc={dc}"]
    if log:
        cmd += ["-l", f"--log={logf}"]
    t0 = time.time()
    r = subprocess.run(cmd + [elf], capture_output=True, text=True)
    dt = time.time() - t0
    out = r.stdout + r.stderr
    m = re.search(r"tohost = (\d+)", out)
    res = dict(wl=wl, a0=int(m.group(1)) if m else None, wall=dt)
    for k in ["D\\$ Read Accesses", "D\\$ Write Accesses", "D\\$ Read Misses",
              "D\\$ Write Misses", "I\\$ Read Accesses", "I\\$ Read Misses"]:
        mm = re.search(k + r":\s+(\d+)", out)
        res[k.replace("\\", "")] = int(mm.group(1)) if mm else None
    if log:
        n = 0
        with open(logf) as f:
            for line in f:
                if "core   0: 0x8" in line:
                    n += 1
        res["insns"] = n
        os.remove(logf)
    return res


def native(wl):
    if wl == "coremark":
        return None
    return int(subprocess.run([os.path.join(BIN, wl + ".native")], capture_output=True,
                              text=True).stdout.strip())


def main():
    os.makedirs(RUNS, exist_ok=True)
    ideal = dict(CACHE_LEVELS=-1)
    jobs = []
    for wl in WORKLOADS:
        # E1 default configuration
        jobs.append(("default", wl, cfg()))
        # E2 forwarding on/off with ideal memory
        for fw in (1, 0):
            jobs.append((f"fwd{fw}", wl, cfg(FORWARDING_SWITCH=fw, **ideal)))
        # E3 branch prediction strategies (ideal memory)
        for bp in (0, 1, 2):
            jobs.append((f"bp{bp}", wl, cfg(BRANCH_PREDICTION_STRATEGY=bp, **ideal)))
        # E6 hierarchy depth
        for lv in (0, 1, 2, 3):
            jobs.append((f"levels{lv}", wl, cfg(CACHE_LEVELS=lv)))
        # E5 L1D write policy (write-through vs write-back, both write-allocate)
        for wp in (0, 1):
            jobs.append((f"wp{wp}", wl, cfg(l1_d=dict(write_policy=wp))))
    # E4 L1D size / associativity / replacement sweep (write-back, write-allocate L1D)
    for wl in ["qsort", "radix", "matmul", "coremark"]:
        for size in (256, 512, 1024, 2048, 4096, 8192):
            for assoc in (1, 2, 4):
                for rp in (0, 1):
                    jobs.append((f"l1d_{size}_{assoc}_{rp}", wl,
                                 cfg(l1_d=dict(cache_size=size, associativity=assoc,
                                               replacement_policy=rp, write_policy=1))))
    if SUBSET:
        jobs = [j for j in jobs if j[0].startswith(tuple(SUBSET.split(",")))]
    print(f"{len(jobs)} simulator jobs", flush=True)
    with ProcessPoolExecutor(max_workers=os.cpu_count()) as ex:
        sims = list(ex.map(run_sim, jobs))
    spikes = {}
    if SUBSET:
        json.dump(dict(sims=sims), open(os.path.join(E, OUT), "w"), indent=1)
        print("done"); return
    for wl in WORKLOADS:
        spikes[wl] = run_spike(wl)
        spikes[wl]["native"] = native(wl)
        # spike cache model on the same L1D geometries (sets:ways:block)
        spikes[wl]["dc_sweep"] = {}
        if wl in ["qsort", "radix", "matmul", "coremark"]:
            for size in (256, 512, 1024, 2048, 4096, 8192):
                for assoc in (1, 2, 4):
                    sets = size // (16 * assoc)
                    s = run_spike(wl, dc=f"{sets}:{assoc}:16", log=False)
                    spikes[wl]["dc_sweep"][f"{size}_{assoc}"] = s
        # spike raw speed (no log, no caches)
        t0 = time.time()
        subprocess.run(["spike", "--isa=rv32i", os.path.join(BIN, wl + ".spike.elf")],
                       capture_output=True)
        spikes[wl]["wall_nolog"] = time.time() - t0
    json.dump(dict(sims=sims, spikes=spikes), open(os.path.join(E, OUT), "w"),
              indent=1)
    print("done")


if __name__ == "__main__":
    main()
