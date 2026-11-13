#!/usr/bin/env python3
"""Black box tests of ./assemble, ./simulate and ./cachesim.

These tests only use the command line interface and the output files, so they
must keep passing, unchanged, through any refactoring of the C code.

  python3 tests/integration/run.py [options] [filter...]

    filter            only run tests whose "group::name" contains one of the filters
    --slow            also run the long C workloads (programs/c quick/radix sort)
    --update-golden   rewrite the golden files from the current build (only do
                      this after an INTENTIONAL change of timing or output format)
    --keep            keep the temporary run directories (printed at the end)
    -v                print the command lines that are run

Test programs (tests/integration/asm/*.s, *.obj) declare what they expect in
comments:
    // EXPECT r5 = 0x78              register value after the run
    // EXPECT mem32[0x40000] = 55    word in memory_dump.bin (mem8[...] for a byte)
    // EXPECT exit = 1               exit status of ./simulate (default 0)
    // EXPECT output: <text>         text that must appear in stdout/stderr
    // CONFIGS: ideal full           run only under these configs (default: all)
    // XFAIL: <reason>               known bug: the program is expected to fail
Every program runs under every machine configuration in CONFIGS below, and the
results must be the same everywhere: the configuration changes timing, never
results.
"""

import concurrent.futures as futures
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
ASM_DIR = os.path.join(HERE, "asm")
C_DIR = os.path.join(HERE, "c")
GOLDEN = os.path.join(HERE, "golden")
ASSEMBLE = os.path.join(ROOT, "assemble")
SIMULATE = os.path.join(ROOT, "simulate")
CACHESIM = os.path.join(ROOT, "cachesim")
ESSENTIALS = os.path.join(ROOT, "essentials")
RISCV_GCC = shutil.which("riscv64-unknown-elf-gcc")
RISCV_AS = shutil.which("riscv64-unknown-elf-as")
RISCV_LD = shutil.which("riscv64-unknown-elf-ld")
RISCV_OBJCOPY = shutil.which("riscv64-unknown-elf-objcopy")
HOST_CC = shutil.which("gcc") or shutil.which("cc")

TIMING = None	# GoldenTiming of golden/timing.json, set in main()

ARGS = {"slow": False, "update": False, "keep": False, "verbose": False, "filters": []}

###############################################################################
# machine configurations
###############################################################################

def cache(size, block, assoc, rl, wl, wmp, rp, wp):
	return {"cache_size": size, "block_size": block, "associativity": assoc, "read_latency": rl,
		"write_latency": wl, "write_miss_policy": wmp, "replacement_policy": rp, "write_policy": wp}

# essentials/template.json (the documented default machine)
BASE = {
	"CACHE_LEVELS": 3, "L1_SPLIT": 1,
	"l1_d": cache(2048, 16, 2, 1, 1, 1, 0, 0),
	"l1_i": cache(2048, 16, 2, 1, 1, 1, 0, 0),
	"l2": cache(8192, 32, 4, 10, 10, 0, 0, 1),
	"l3": cache(16384, 64, 8, 30, 30, 0, 0, 1),
	"DRAM_READ_LATENCY": 200, "DRAM_WRITE_LATENCY": 200, "DRAM_SIZE": 1073741824,
	"FORWARDING_SWITCH": 1, "BRANCH_PREDICTION_STRATEGY": 0,
	"text_segment_limit": 131071, "rodata_segment_limit": 262143,
}

def config(**overrides):
	c = json.loads(json.dumps(BASE))
	for k, v in overrides.items():
		if isinstance(v, dict):
			c[k].update(v)
		else:
			c[k] = v
	return c

MB = 1 << 20

# assembly programs: 1MB of DRAM (memory dumps stay small, the stack of
# recursion.s starts at 1MB)
CONFIGS = {
	"ideal": config(CACHE_LEVELS=-1, DRAM_SIZE=MB),
	"ideal_nofwd": config(CACHE_LEVELS=-1, FORWARDING_SWITCH=0, DRAM_SIZE=MB),
	"ideal_taken": config(CACHE_LEVELS=-1, BRANCH_PREDICTION_STRATEGY=1, DRAM_SIZE=MB),
	"ideal_btfn": config(CACHE_LEVELS=-1, BRANCH_PREDICTION_STRATEGY=2, DRAM_SIZE=MB),
	"nocache": config(CACHE_LEVELS=0, DRAM_SIZE=MB),
	"l1": config(CACHE_LEVELS=1, DRAM_SIZE=MB),
	"l2": config(CACHE_LEVELS=2, DRAM_SIZE=MB),
	"full": config(DRAM_SIZE=MB),
	"full_wb": config(l1_d={"write_policy": 1}, DRAM_SIZE=MB),
	"full_lru": config(l1_d={"replacement_policy": 1}, l1_i={"replacement_policy": 1},
		l2={"replacement_policy": 1}, l3={"replacement_policy": 1}, DRAM_SIZE=MB),
	"full_nowa": config(l1_d={"write_miss_policy": 0}, l2={"write_miss_policy": 1},
		l3={"write_miss_policy": 1}, DRAM_SIZE=MB),
	"tiny": config(l1_d=cache(64, 16, 1, 1, 1, 1, 1, 1), l1_i=cache(128, 16, 2, 1, 1, 1, 0, 0),
		l2=cache(512, 32, 2, 4, 4, 0, 1, 1), l3=cache(2048, 64, 4, 8, 8, 1, 0, 1),
		DRAM_READ_LATENCY=20, DRAM_WRITE_LATENCY=25, DRAM_SIZE=MB),
	"nofwd_full_btfn": config(FORWARDING_SWITCH=0, BRANCH_PREDICTION_STRATEGY=2, DRAM_SIZE=MB),
}

# the configurations of collegereport/demo/c/configs (documented numbers)
DEMO_CONFIGS = {
	"01_ideal": config(CACHE_LEVELS=-1),
	"02_ideal_no_forwarding": config(CACHE_LEVELS=-1, FORWARDING_SWITCH=0),
	"03_ideal_always_taken": config(CACHE_LEVELS=-1, BRANCH_PREDICTION_STRATEGY=1),
	"04_ideal_btfn": config(CACHE_LEVELS=-1, BRANCH_PREDICTION_STRATEGY=2),
	"05_no_cache": config(CACHE_LEVELS=0),
	"06_l1_only": config(CACHE_LEVELS=1),
	"07_full_hierarchy": config(),
	"08_full_hierarchy_writeback": config(l1_d={"write_policy": 1}),
	"09_tiny_l1d": config(l1_d={"cache_size": 64, "associativity": 1}),
	"10_asm_memdump": config(CACHE_LEVELS=-1, DRAM_SIZE=MB),
}

# compiled C programs need the 1GB memory of essentials/linker.ld
C_CONFIGS = {
	"c_ideal": config(CACHE_LEVELS=-1),
	"c_full": config(),
	"c_nofwd_wb_btfn": config(FORWARDING_SWITCH=0, BRANCH_PREDICTION_STRATEGY=2, l1_d={"write_policy": 1}),
}

###############################################################################
# reporting
###############################################################################

class Fail(Exception):
	pass

class Skip(Exception):
	pass

COLOR = sys.stdout.isatty()
def color(code, s):
	return "\033[%sm%s\033[0m" % (code, s) if COLOR else s

class Group:
	def __init__(self, name):
		self.name = name
		self.counts = {"pass": 0, "fail": 0, "xfail": 0, "xpass": 0, "skip": 0}
		self.header = False

	def selected(self, test):
		full = "%s::%s" % (self.name, test)
		return not ARGS["filters"] or any(f in full for f in ARGS["filters"])

	def run(self, test, fn, xfail=None):
		if not self.selected(test):
			return
		if not self.header:
			print("=== integration:%s ===" % self.name)
			self.header = True
		try:
			fn()
			ok, msg = True, ""
		except Fail as e:
			ok, msg = False, str(e)
		except Skip as e:
			self.counts["skip"] += 1
			print("  %s  %s  (%s)" % (color("36", "SKIP"), test, e))
			return
		except Exception as e:	# a bug in the test itself
			ok, msg = False, "test raised %s: %s" % (type(e).__name__, e)
		if xfail:
			if ok:
				self.counts["xpass"] += 1
				print("  %s %s  (known bug seems FIXED, remove the XFAIL marker: %s)" % (color("33", "XPASS"), test, xfail))
			else:
				self.counts["xfail"] += 1
				print("  %s %s  (known bug: %s)" % (color("33", "XFAIL"), test, xfail))
				if ARGS["verbose"]:
					print(indent(msg))
		elif ok:
			self.counts["pass"] += 1
			print("  %s  %s" % (color("32", "PASS"), test))
		else:
			self.counts["fail"] += 1
			print("  %s  %s" % (color("31", "FAIL"), test))
			print(indent(msg))

	def finish(self):
		if not self.header:
			return 0
		c = self.counts
		print("--- integration:%s: %d passed, %d failed, %d xfail, %d xpass, %d skipped" %
			(self.name, c["pass"], c["fail"], c["xfail"], c["xpass"], c["skip"]))
		print("@@RESULT integration:%s %d %d %d %d" % (self.name, c["pass"], c["fail"], c["xfail"], c["xpass"]))
		print()
		return c["fail"]

def indent(text, prefix="      "):
	return "\n".join(prefix + l for l in str(text).splitlines())

def check(cond, msg):
	if not cond:
		raise Fail(msg)

def tail(text, n=8):
	lines = text.strip().splitlines()
	return "\n".join(("| " + l) for l in lines[-n:]) if lines else "| (no output)"

###############################################################################
# running the tools
###############################################################################

SESSION = tempfile.mkdtemp(prefix="simtest_integration_", dir=os.environ.get("TMPDIR"))
_dir_counter = [0]

def fresh_dir(label):
	_dir_counter[0] += 1
	d = os.path.join(SESSION, "%04d_%s" % (_dir_counter[0], re.sub(r"[^A-Za-z0-9_.@-]", "_", label)))
	os.makedirs(d)
	return d

def run(cmd, cwd, stdin=None, timeout=120):
	if ARGS["verbose"]:
		print("      $ (cd %s && %s)" % (cwd, " ".join(cmd)))
	try:
		p = subprocess.run(cmd, cwd=cwd, input=stdin, stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=timeout)
	except subprocess.TimeoutExpired:
		raise Fail("timed out after %ds: %s" % (timeout, " ".join(cmd)))
	out = p.stdout.decode(errors="replace")
	err = p.stderr.decode(errors="replace")
	return p.returncode, out, err

_config_files = {}
def config_file(name, cfg):
	if name not in _config_files:
		path = os.path.join(SESSION, "config_%s.json" % name)
		with open(path, "w") as f:
			json.dump(cfg, f, indent=4)
		_config_files[name] = path
	return _config_files[name]

def parse_stats(stdout):
	stats = {}
	in_stats = False
	for line in stdout.splitlines():
		if "Simulation Statistics" in line:
			in_stats = True
		m = re.match(r"^(\S.*?)\s*:\s*(\d+)$", line)
		if in_stats and m:
			stats[m.group(1)] = int(m.group(2))
	return stats

def parse_regs(path):
	regs = {}
	with open(path) as f:
		for line in f:
			m = re.match(r"^r(\d\d) : ([0-9a-f]{8})$", line.strip())
			if m:
				regs[int(m.group(1))] = int(m.group(2), 16)
	return regs

class SimResult:
	def __init__(self, code, out, err, cwd):
		self.code, self.out, self.err, self.cwd = code, out, err, cwd
		self.stats = parse_stats(out)

	def path(self, name):
		return os.path.join(self.cwd, name)

	def regs(self):
		p = self.path("register_dump")
		if not os.path.exists(p):
			raise Fail("register_dump was not written (exit %d)\n%s" % (self.code, tail(self.out + self.err)))
		return parse_regs(p)

	def mem(self, addr, size):
		p = self.path("memory_dump.bin")
		if not os.path.exists(p):
			raise Fail("memory_dump.bin was not written")
		with open(p, "rb") as f:
			f.seek(addr)
			return int.from_bytes(f.read(size), "little")

def simulate(program_flag, program, cfg_name, cfg, flags=(), label=None, timeout=120):
	cwd = fresh_dir(label or "%s@%s" % (os.path.basename(program), cfg_name))
	cmd = [SIMULATE, program_flag, program, "-c", config_file(cfg_name, cfg)] + list(flags)
	code, out, err = run(cmd, cwd, timeout=timeout)
	return SimResult(code, out, err, cwd)

_assembled = {}
def assembled(src):
	"""verbose object file of a test program (assembled once per session)"""
	if src.endswith(".obj"):
		return src
	if src not in _assembled:
		cwd = fresh_dir("asm_" + os.path.basename(src))
		obj = os.path.join(cwd, os.path.basename(src)[:-2] + ".obj")
		code, out, err = run([ASSEMBLE, "-v", src, obj], cwd)
		if code != 0 or not os.path.exists(obj):
			raise Fail("assembling %s failed (exit %d)\n%s" % (src, code, tail(out + err)))
		_assembled[src] = obj
	return _assembled[src]

###############################################################################
# golden files
###############################################################################

def golden_path(*parts):
	return os.path.join(GOLDEN, *parts)

def check_golden_text(rel, actual, what):
	path = golden_path(rel)
	if ARGS["update"]:
		os.makedirs(os.path.dirname(path), exist_ok=True)
		with open(path, "w") as f:
			f.write(actual)
		return
	if not os.path.exists(path):
		raise Fail("missing golden file %s (create it with: make test-golden)" % os.path.relpath(path, ROOT))
	with open(path) as f:
		expected = f.read()
	if expected != actual:
		import difflib
		diff = list(difflib.unified_diff(expected.splitlines(), actual.splitlines(),
			"golden/" + rel, "actual " + what, lineterm="", n=1))
		shown = "\n".join(diff[:40]) + ("\n... (%d more diff lines)" % (len(diff) - 40) if len(diff) > 40 else "")
		raise Fail("%s differs from %s\n%s" % (what, os.path.relpath(path, ROOT), shown))

class GoldenTiming:
	"""tests/integration/golden/timing.json: the statistics of every run"""

	def __init__(self, name):
		self.path = golden_path(name)
		self.data = {}
		if os.path.exists(self.path):
			with open(self.path) as f:
				self.data = json.load(f)
		self.new = {}

	def compare(self, key, stats):
		self.new[key] = stats
		if ARGS["update"]:
			return None
		if key not in self.data:
			return "%s: no golden statistics (create them with: make test-golden)" % key
		exp = self.data[key]
		diffs = ["%s %d -> %d" % (k, exp.get(k, -1), stats.get(k, -1))
			for k in sorted(set(exp) | set(stats)) if exp.get(k) != stats.get(k)]
		if diffs:
			return "%s: %s" % (key, ", ".join(diffs))
		return None

	def save(self):
		if not ARGS["update"] or not self.new:
			return
		merged = dict(self.data)
		merged.update(self.new)
		os.makedirs(os.path.dirname(self.path), exist_ok=True)
		with open(self.path, "w") as f:
			json.dump(dict(sorted(merged.items())), f, indent=1, sort_keys=False)
			f.write("\n")

###############################################################################
# test programs and their expectations
###############################################################################

class Program:
	def __init__(self, path):
		self.path = path
		self.name = os.path.splitext(os.path.basename(path))[0]
		self.regs, self.mem, self.output = {}, [], []
		self.exit = 0
		self.xfail = None
		self.configs = list(CONFIGS)
		with open(path) as f:
			for line in f:
				m = re.match(r"\s*//\s*(EXPECT|XFAIL:|CONFIGS:)\s*(.*?)\s*$", line)
				if not m:
					continue
				kind, rest = m.groups()
				if kind == "XFAIL:":
					self.xfail = rest
				elif kind == "CONFIGS:":
					self.configs = rest.split()
				elif rest.startswith("output:"):
					self.output.append(rest[len("output:"):].strip())
				else:
					m2 = re.match(r"(r\d+|exit|mem(?:8|32)\[(0x[0-9a-fA-F]+|\d+)\])\s*=\s*(-?(?:0x[0-9a-fA-F]+|\d+))$", rest)
					if not m2:
						raise ValueError("%s: cannot parse expectation '%s'" % (path, rest))
					target, value = m2.group(1), int(m2.group(3), 0)
					if target == "exit":
						self.exit = value
					elif target.startswith("r"):
						self.regs[int(target[1:])] = value & 0xFFFFFFFF
					else:
						size = 4 if target.startswith("mem32") else 1
						self.mem.append((int(m2.group(2), 0), size, value & (0xFFFFFFFF if size == 4 else 0xFF)))

	def flags(self):
		return ["-r"] + (["-m"] if self.mem else [])

	def check(self, r):
		"""returns the list of problems with a simulation result"""
		problems = []
		if r.code != self.exit:
			problems.append("exit status %d, expected %d\n%s" % (r.code, self.exit, tail(r.out + r.err)))
		text = r.out + r.err
		for o in self.output:
			if o not in text:
				problems.append("output does not contain \"%s\"\n%s" % (o, tail(text)))
		if self.regs:
			try:
				regs = r.regs()
				for n, v in sorted(self.regs.items()):
					if regs.get(n) != v:
						got = regs.get(n)
						problems.append("r%d = %s, expected 0x%08x" % (n, "0x%08x" % got if got is not None else "missing", v))
			except Fail as e:
				problems.append(str(e))
		for addr, size, v in self.mem:
			try:
				got = r.mem(addr, size)
				if got != v:
					problems.append("mem%d[0x%x] = 0x%x, expected 0x%x" % (size * 8, addr, got, v))
			except Fail as e:
				problems.append(str(e))
		return problems

def programs():
	files = sorted(f for f in os.listdir(ASM_DIR) if f.endswith(".s") or f.endswith(".obj"))
	return [Program(os.path.join(ASM_DIR, f)) for f in files]

def run_matrix(progs):
	"""runs every program under each of its configs in parallel: {(prog, cfg): SimResult}"""
	jobs = {}
	with futures.ThreadPoolExecutor(max_workers=os.cpu_count() or 4) as pool:
		for p in progs:
			try:
				obj = assembled(p.path)
			except Fail as e:
				for c in p.configs:
					jobs[(p.name, c)] = e
				continue
			for c in p.configs:
				jobs[(p.name, c)] = pool.submit(simulate, "-a", obj, c, CONFIGS[c], p.flags())
		results = {}
		for k, j in jobs.items():
			try:
				results[k] = j if isinstance(j, Fail) else j.result()
			except Fail as e:
				results[k] = e
	return results

###############################################################################
# groups
###############################################################################

def group_programs(progs, results, timing):
	g = Group("programs")
	for p in progs:
		def test(p=p):
			problems = []
			for c in p.configs:
				r = results[(p.name, c)]
				if isinstance(r, Fail):
					problems.append("[%s] %s" % (c, r))
					continue
				for prob in p.check(r):
					problems.append("[%s] %s" % (c, prob))
			check(not problems, "\n".join(problems))
		g.run(p.name, test, p.xfail)

	# timing: the statistics of every run must match the golden statistics
	t = Group("timing")
	for p in progs:
		if p.xfail or p.exit != 0:
			continue
		def test(p=p):
			problems = []
			for c in p.configs:
				r = results[(p.name, c)]
				if isinstance(r, Fail) or r.code != 0:
					problems.append("[%s] run failed, no statistics" % c)
					continue
				d = timing.compare("%s@%s" % (p.name, c), r.stats)
				if d:
					problems.append(d)
			check(not problems, "\n".join(problems))
		t.run(p.name, test)
	return g.finish() + t.finish()

def group_assembler_cli():
	g = Group("assembler_cli")
	src = os.path.join(ASM_DIR, "demo_02_sum_loop.s")

	def verbose_with_output_file():
		d = fresh_dir("asm_v_out")
		code, out, err = run([ASSEMBLE, "-v", src, "x.obj"], d)
		check(code == 0, "exit %d\n%s" % (code, tail(out + err)))
		lines = open(os.path.join(d, "x.obj")).read().splitlines()
		check(lines[0].rstrip() == "[0] 00000000   00000093           addi    r1, r0, 0",
			"unexpected first line: %r" % lines[0])
		check("//" not in "".join(lines), "comments are kept in the verbose output")
		check("loop:" in lines, "label line missing from the verbose output")
		check(not os.path.exists(os.path.join(d, "output.txt")), "output.txt written as well")
	g.run("verbose_with_output_file", verbose_with_output_file)

	def plain_default_output():
		d = fresh_dir("asm_plain")
		code, out, err = run([ASSEMBLE, src], d)
		check(code == 0, "exit %d\n%s" % (code, tail(out + err)))
		words = open(os.path.join(d, "output.txt")).read().split("\n")
		check(words[:2] == ["00000093", "00100113"] and words[-1] == "", "unexpected output.txt: %r" % words[:3])
		check(all(re.fullmatch(r"[0-9a-f]{8}", w) for w in words[:-1]), "non hex lines in output.txt")
	g.run("plain_writes_output_txt", plain_default_output)

	def verbose_default_output():
		d = fresh_dir("asm_v")
		code, out, err = run([ASSEMBLE, "-v", src], d)
		check(code == 0, "exit %d" % code)
		check(open(os.path.join(d, "output.txt")).read().startswith("[0] 00000000   00000093"), "output.txt is not verbose")
	g.run("verbose_writes_output_txt", verbose_default_output)

	def plain_with_output_file():
		d = fresh_dir("asm_plain_out")
		code, out, err = run([ASSEMBLE, src, "x.hex"], d)
		check(code == 0, "exit %d (expected 0)" % code)
		check(os.path.exists(os.path.join(d, "x.hex")), "x.hex not written")
	g.run("plain_with_output_file", plain_with_output_file,
		xfail="'assemble <in> <out>' returns -1 without writing anything (stray return in the argc == 3 branch of main.c)")

	def argument_errors():
		d = fresh_dir("asm_args")
		for args, msg in [([], "Invalid arguments\n"), (["a", "b", "c"], "Invalid arguments1\n"),
				(["a", "b", "c", "d"], "Invalid arguments\n"), (["missing.s"], "File not found\n"),
				(["-v", "missing.s", "o"], "File not found\n")]:
			code, out, err = run([ASSEMBLE] + args, d)
			check(code == 255 and out == msg, "assemble %s: exit %d output %r, expected 255 %r" % (" ".join(args), code, out, msg))
	g.run("argument_errors", argument_errors)

	def error(source, expected_lines, label):
		d = fresh_dir(label)
		with open(os.path.join(d, "in.s"), "w") as f:
			f.write(source)
		code, out, err = run([ASSEMBLE, "-v", "in.s", "out.obj"], d)
		check(code == 255, "exit %d, expected 255\n%s" % (code, tail(out)))
		for e in expected_lines:
			check(e in out, "missing %r in output\n%s" % (e, tail(out)))

	g.run("error_unknown_instruction", lambda: error("addi r1, r0, 1\n\n// c\nmul r2, r1, r1\n",
		["Invalid operation name", "Syntax error at line number : 4", "[FATAL]: Invalid syntax encountered"], "asm_e1"))
	g.run("error_operand_count", lambda: error("add r1, r2\n",
		["Invalid number of operands", "Syntax error at line number : 1"], "asm_e2"))
	g.run("error_bad_operand", lambda: error("nop\naddi r1, r0, 5000\n",
		["Invalid operand 3", "Semantics error at line number : 2", "[FATAL]: Invalid semantics encountered"], "asm_e3"))
	g.run("error_bad_register", lambda: error("add r1, r2, x3\n", ["Invalid operand 3", "Semantics error at line number : 1"], "asm_e4"))
	g.run("error_parsing", lambda: error("addi r1, r0, 1\nadd r1 r2 r3\n",
		["Invalid parsing at line number : 2", "[FATAL]: Invalid parsing encountered"], "asm_e5"))
	g.run("error_undefined_label", lambda: error("beq r1, r2, nowhere\n",
		["Opcode couldnt be generated.", "[FATAL]: Some error in opcode generation..."], "asm_e6"))
	g.run("error_duplicate_label", lambda: error("a:\nnop\na:\nnop\n", ["[FATAL]: Couldnt add symbol to table."], "asm_e7"))
	g.run("error_branch_out_of_range", lambda: error("beq r0, r0, far\n" + "nop\n" * 1100 + "far:\nnop\n",
		["Opcode couldnt be generated."], "asm_e8"))

	def labels_and_comments_layout():
		d = fresh_dir("asm_layout")
		with open(os.path.join(d, "in.s"), "w") as f:
			f.write("// header\n\n   \nStart:\n  ADDI R1, R0, 1   // one\nloop:\n  bne r1, r0, START\n")
		code, out, err = run([ASSEMBLE, "-v", "in.s", "o.obj"], d)
		check(code == 0, "exit %d\n%s" % (code, tail(out)))
		text = open(os.path.join(d, "o.obj")).read()
		expected = "Start:\n[0] 00000000   00100093     ADDI R1, R0, 1   \nloop:\n[1] 0x000004   fe009ee3     bne r1, r0, START\n"
		check(text == expected, "verbose output\n%r\nexpected\n%r" % (text, expected))
	g.run("labels_comments_case_and_layout", labels_and_comments_layout)

	# golden output of every test program, plain and verbose
	for f in sorted(os.listdir(ASM_DIR)):
		if not f.endswith(".s"):
			continue
		def golden(f=f):
			d = fresh_dir("asm_golden_" + f)
			src = os.path.join(ASM_DIR, f)
			code, out, err = run([ASSEMBLE, "-v", src, "v.obj"], d)
			check(code == 0, "exit %d\n%s" % (code, tail(out)))
			verbose = open(os.path.join(d, "v.obj")).read()
			code, out, err = run([ASSEMBLE, src], d)
			plain = open(os.path.join(d, "output.txt")).read()
			from_verbose = "".join(l.split()[2] + "\n" for l in verbose.splitlines() if l.startswith("["))
			check(plain == from_verbose, "plain output differs from the machine code column of the verbose output")
			check_golden_text(os.path.join("asm", f[:-2] + ".obj"), verbose, "assembler output")
		g.run("golden_" + f[:-2], golden)
	return g.finish()

def gnu_assemble(src_text, d):
	"""machine code words of a program assembled by GNU as (our syntax translated)"""
	lines = []
	for line in src_text.splitlines():
		code = line.split("//")[0]
		stripped = code.strip().lower()
		if stripped == "ebreak":
			code = ".word 0x02000073"	# the project encoding (the spec one is tested in the unit tests)
		elif stripped == "fence":
			code = ".word 0x0000000f"	# gcc emits fence iorw,iorw
		code = re.sub(r"\b[rR](\d+)\b", r"x\1", code)
		lines.append(code)
	with open(os.path.join(d, "g.s"), "w") as f:
		f.write("\n".join(lines) + "\n")
	for cmd in ([RISCV_AS, "-march=rv32i", "-mabi=ilp32", "-mno-relax", "g.s", "-o", "g.o"],
			[RISCV_LD, "-m", "elf32lriscv", "-e", "0", "-Ttext=0", "g.o", "-o", "g.elf"],
			[RISCV_OBJCOPY, "-O", "binary", "-j", ".text", "g.elf", "g.bin"]):
		code, out, err = run(cmd, d)
		check(code == 0, "%s failed\n%s" % (os.path.basename(cmd[0]), tail(out + err)))
	data = open(os.path.join(d, "g.bin"), "rb").read()
	return ["%08x" % int.from_bytes(data[i:i + 4], "little") for i in range(0, len(data), 4)]

def group_assembler_oracle():
	g = Group("assembler_vs_gnu_as")
	for f in sorted(os.listdir(ASM_DIR)):
		if not f.endswith(".s"):
			continue
		def test(f=f):
			if not (RISCV_AS and RISCV_LD and RISCV_OBJCOPY):
				raise Skip("riscv64-unknown-elf binutils not installed")
			d = fresh_dir("gnu_" + f)
			src = os.path.join(ASM_DIR, f)
			expected = gnu_assemble(open(src).read(), d)
			code, out, err = run([ASSEMBLE, src], d)
			check(code == 0, "assemble failed")
			ours = open(os.path.join(d, "output.txt")).read().split()
			diffs = ["word %d: ours %s, GNU as %s" % (i, a, b) for i, (a, b) in enumerate(zip(ours, expected)) if a != b]
			if len(ours) != len(expected):
				diffs.append("%d words, GNU as produced %d" % (len(ours), len(expected)))
			check(not diffs, "\n".join(diffs[:20]))
		g.run(f[:-2], test)
	return g.finish()

def group_simulator_cli():
	g = Group("simulator_cli")
	obj = assembled(os.path.join(ASM_DIR, "demo_01_basics.s"))
	cfg = config_file("ideal", CONFIGS["ideal"])

	def arg_error(args, message):
		def test():
			d = fresh_dir("cli")
			code, out, err = run([SIMULATE] + args, d)
			check(code == 1, "exit %d, expected 1" % code)
			check(err == message + "\n", "stderr %r, expected %r" % (err, message))
			check(out == "", "nothing must be printed on stdout, got %r" % out[:200])
		return test

	for flag in ["-p", "-t", "-m", "-r", "-l"]:
		g.run("duplicate_%s" % flag[1], arg_error([flag, "-a", obj, flag], "Error: duplicate %s flag" % flag))
	g.run("duplicate_a", arg_error(["-a", obj, "-a", obj], "Error: duplicate -a flag"))
	g.run("duplicate_e", arg_error(["-e", "x.elf", "-e", "x.elf"], "Error: duplicate -e flag"))
	g.run("duplicate_c", arg_error(["-c", cfg, "-c", cfg], "Error: duplicate -c flag"))
	g.run("a_and_e", arg_error(["-a", obj, "-e", "x.elf"], "Error: already supplied custom object file"))
	g.run("e_and_a", arg_error(["-e", "x.elf", "-a", obj], "Error: already supplied elf object file"))
	g.run("a_without_file", arg_error(["-a"], "Error: -a requires a filename"))
	g.run("a_followed_by_flag", arg_error(["-a", "-r"], "Error: -a requires a filename"))
	g.run("e_without_file", arg_error(["-e"], "Error: -e requires a filename"))
	g.run("c_without_file", arg_error(["-a", obj, "-c"], "Error: -c requires a filename"))
	g.run("unknown_argument", arg_error(["-a", obj, "-x"], "Error: unknown argument '-x'"))
	g.run("stray_argument", arg_error(["-a", obj, "extra"], "Error: unknown argument 'extra'"))

	def no_input():
		d = fresh_dir("cli_noinput")
		code, out, err = run([SIMULATE, "-c", cfg], d)
		check(code == 1, "exit %d" % code)
		check(out == "Configuring System...\nInitializing Memory...\n", "stdout %r" % out)
		check(err == "Input file not specified\n", "stderr %r" % err)
	g.run("no_input_file", no_input)

	def missing_object():
		d = fresh_dir("cli_missing")
		code, out, err = run([SIMULATE, "-a", "nope.obj", "-c", cfg], d)
		check(code == 1, "exit %d" % code)
		check("[FATAL]: failed to open memory image file" in out, tail(out))
	g.run("missing_object_file", missing_object)

	def missing_elf():
		d = fresh_dir("cli_missing_elf")
		code, out, err = run([SIMULATE, "-e", "nope.elf", "-c", cfg], d)
		check(code == 1, "exit %d" % code)
		check("[FATAL]: Couldnt open ELF file.\n[FATAL]: Elf loader failed." in out, tail(out))
	g.run("missing_elf_file", missing_elf)

	def not_an_elf():
		d = fresh_dir("cli_bad_elf")
		code, out, err = run([SIMULATE, "-e", obj, "-c", cfg], d)
		check(code == 1, "exit %d" % code)
		check("[FATAL]: Not an ELF file." in out, tail(out))
	g.run("object_file_given_as_elf", not_an_elf)

	def missing_config():
		d = fresh_dir("cli_missing_cfg")
		code, out, err = run([SIMULATE, "-a", obj, "-c", "nope.json", "-r"], d)
		check(code == 0, "exit %d" % code)
		check("Could not open config file nope.json" in out, tail(out))
		check(parse_regs(os.path.join(d, "register_dump"))[3] == 0x11, "wrong result with the default machine")
		check(parse_stats(out).get("L1 I Read Misses", 0) > 0, "the default machine has caches")
	g.run("missing_config_uses_defaults", missing_config)

	def no_config_uses_defaults():
		d = fresh_dir("cli_no_cfg")
		code, out, err = run([SIMULATE, "-a", obj], d)
		check(code == 0, "exit %d" % code)
		check(parse_stats(out).get("L1 I Read Misses", 0) > 0, "the default machine has caches")
	g.run("no_config_uses_defaults", no_config_uses_defaults)

	def default_machine_is_the_template():
		# running without -c must be the machine documented in essentials/template.json
		array = assembled(os.path.join(ASM_DIR, "demo_04_array.s"))
		stats = []
		for args in ([], ["-c", os.path.join(ESSENTIALS, "template.json")]):
			d = fresh_dir("cli_defaults")
			code, out, err = run([SIMULATE, "-a", array] + args, d)
			check(code == 0, "exit %d" % code)
			stats.append(parse_stats(out))
		check(stats[0] == stats[1], "statistics without -c differ from -c essentials/template.json:\n%s" %
			", ".join("%s %s vs %s" % (k, stats[0].get(k), stats[1].get(k)) for k in stats[0] if stats[0].get(k) != stats[1].get(k)))
		with open(os.path.join(ESSENTIALS, "template.json")) as f:
			check(json.load(f) == BASE, "essentials/template.json differs from BASE in run.py")
		d = TIMING.compare("demo_04_array@no_config", stats[0])
		check(d is None, d)
	g.run("default_machine_is_the_template", default_machine_is_the_template)

	def stdout_layout():
		d = fresh_dir("cli_stdout")
		code, out, err = run([SIMULATE, "-a", obj, "-c", cfg], d)
		check(code == 0 and err == "", "exit %d stderr %r" % (code, err))
		head = "Configuring System...\nInitializing Memory...\nRunning Simulation...\nDumping Machine State...\nGenerating Stats...\n"
		check(out.startswith(head), "stdout starts with %r" % out[:200])
		check(out.endswith("\n=======================================================\n"), "stdout ends with %r" % out[-100:])
		keys = list(parse_stats(out))
		check(len(keys) == 36 and keys[0] == "L1 D Read Hits" and keys[-1] == "Total Cycles",
			"statistics keys: %s" % keys)
	g.run("stdout_layout", stdout_layout)

	def output_files(flags, expected, full_args=None):
		def test():
			d = fresh_dir("cli_files")
			code, out, err = run([SIMULATE] + (full_args or ["-a", obj, "-c", cfg] + flags), d)
			check(code == 0, "exit %d\n%s" % (code, tail(out + err)))
			files = sorted(os.listdir(d))
			check(files == sorted(expected), "files written: %s, expected %s" % (files, sorted(expected)))
		return test
	g.run("no_flags_no_files", output_files([], []))
	g.run("flag_r_register_dumps", output_files(["-r"], ["register_dump", "internal_register_dump"]))
	g.run("flag_p_pipeline_visuals", output_files(["-p"], ["pipevis.txt"]))
	g.run("flag_m_memory_dump", output_files(["-m"], ["memory_dump.bin"]))
	g.run("flag_t_traces", output_files(["-t"], ["instruction_trace.txt", "data_trace.txt"]))
	g.run("flag_l_log", output_files(["-l"], ["log.log"]))
	g.run("all_flags", output_files(["-l", "-t", "-m", "-r", "-p"], ["log.log", "instruction_trace.txt", "data_trace.txt",
		"memory_dump.bin", "register_dump", "internal_register_dump", "pipevis.txt"]))
	g.run("flags_in_any_order", output_files(None, ["pipevis.txt", "register_dump", "internal_register_dump"],
		full_args=["-p", "-c", cfg, "-r", "-a", obj]))

	def memory_dump_size():
		d = fresh_dir("cli_dump")
		code, out, err = run([SIMULATE, "-a", obj, "-c", cfg, "-m"], d)
		check(os.path.getsize(os.path.join(d, "memory_dump.bin")) == MB, "memory dump is not DRAM_SIZE bytes")
		with open(os.path.join(d, "memory_dump.bin"), "rb") as f:
			check(f.read(4) == bytes.fromhex("9300c000"), "the program is not at address 0")
	g.run("memory_dump_is_whole_dram", memory_dump_size)

	def log_content():
		d = fresh_dir("cli_log")
		run([SIMULATE, "-a", obj, "-c", cfg, "-l"], d)
		text = open(os.path.join(d, "log.log")).read()
		check(text.startswith("Date and Time : "), "log starts with %r" % text[:40])
		for s in ["[INFO]: New cycle starts", "[INFO]: Trapping for ecall", "[DEBUG]: "]:
			check(s in text, "log does not contain %r" % s)
	g.run("log_content", log_content)
	return g.finish()

def group_documented():
	"""numbers documented in collegereport/demo/README.md"""
	g = Group("documented")

	def run_demo(prog, cfg, flags=()):
		r = simulate("-a", assembled(os.path.join(ASM_DIR, prog + ".s")), "demo_" + cfg, DEMO_CONFIGS[cfg], flags)
		check(r.code == 0, "%s@%s exit %d\n%s" % (prog, cfg, r.code, tail(r.out)))
		return r.stats

	def expect(prog, cfg, **values):
		def test():
			s = run_demo(prog, cfg)
			bad = ["%s = %s, documented %d" % (k, s.get(k.replace("_", " ")), v) for k, v in values.items() if s.get(k.replace("_", " ")) != v]
			check(not bad, "\n".join(bad))
		return test

	g.run("basics_ideal", expect("demo_01_basics", "01_ideal", Total_Cycles=18, Instructions_Retired=14))
	g.run("hazards_forwarding_on", expect("demo_03_hazards", "01_ideal", Total_Cycles=14))
	g.run("hazards_forwarding_off", expect("demo_03_hazards", "02_ideal_no_forwarding", Total_Cycles=28,
		Forwards_to_EX=0, Forwards_to_MA=0))
	g.run("sum_loop_not_taken", expect("demo_02_sum_loop", "01_ideal", Branch_Misses=9, Total_Cycles=58))
	g.run("sum_loop_always_taken", expect("demo_02_sum_loop", "03_ideal_always_taken", Branch_Misses=1, Total_Cycles=51))
	g.run("sum_loop_btfn", expect("demo_02_sum_loop", "04_ideal_btfn", Branch_Misses=1, Total_Cycles=51))
	g.run("array_not_taken", expect("demo_04_array", "01_ideal", Branch_Misses=189, Total_Cycles=1417))
	g.run("array_btfn", expect("demo_04_array", "04_ideal_btfn", Branch_Misses=3, Total_Cycles=1234))
	g.run("array_no_cache", expect("demo_04_array", "05_no_cache", Total_Cycles=220387))
	g.run("array_l1_only", expect("demo_04_array", "06_l1_only", Total_Cycles=18231))
	g.run("array_full_hierarchy", expect("demo_04_array", "07_full_hierarchy", Total_Cycles=3541,
		L1_D_Read_Hits=128, L1_D_Read_Misses=0, L1_D_Write_Misses=16))
	g.run("array_l1d_write_back", expect("demo_04_array", "08_full_hierarchy_writeback", Total_Cycles=2901))
	g.run("array_tiny_l1d", expect("demo_04_array", "09_tiny_l1d", Total_Cycles=3861,
		L1_D_Read_Hits=96, L1_D_Read_Misses=32, L1_D_Write_Misses=16))

	def hazards_results_identical():
		dumps = []
		for cfg in ["01_ideal", "02_ideal_no_forwarding"]:
			r = simulate("-a", assembled(os.path.join(ASM_DIR, "demo_03_hazards.s")), "demo_" + cfg, DEMO_CONFIGS[cfg], ["-r"])
			dumps.append(open(r.path("register_dump")).read())
		check(dumps[0] == dumps[1], "register_dump differs between forwarding on and off")
	g.run("hazards_same_registers_with_and_without_forwarding", hazards_results_identical)

	def sum_memdump():
		r = simulate("-a", assembled(os.path.join(ASM_DIR, "demo_02_sum_loop.s")), "demo_10", DEMO_CONFIGS["10_asm_memdump"], ["-r", "-m"])
		check(r.regs()[1] == 0x37, "r01 is not 0x37")
		check(r.mem(0x40000, 16) == 0x37, "memory at 0x40000 is not 37 00 00 00 00 ...")
	g.run("sum_loop_memory_dump", sum_memdump)

	def cachesim_matches():
		r = simulate("-a", assembled(os.path.join(ASM_DIR, "demo_04_array.s")), "demo_07", DEMO_CONFIGS["07_full_hierarchy"], ["-t"])
		with open(r.path("data_trace.txt"), "rb") as f:
			code, out, err = run([CACHESIM], r.cwd, stdin=f.read())
		s = parse_stats(out)
		got = [s.get("L1 D Read Hits"), s.get("L1 D Read Misses"), s.get("L1 D Write Hits"), s.get("L1 D Write Misses")]
		check(got == [128, 0, 48, 16], "cachesim L1 D read hits/misses, write hits/misses = %s, documented 128/0/48/16" % got)
		sim = [r.stats.get(k) for k in ["L1 D Read Hits", "L1 D Read Misses", "L1 D Write Hits", "L1 D Write Misses"]]
		check(got == sim, "cachesim %s disagrees with the simulator %s" % (got, sim))
	g.run("cachesim_replays_simulator_trace", cachesim_matches)
	return g.finish()

def group_golden_outputs():
	g = Group("golden_outputs")

	def pipevis(prog, cfg):
		def test():
			r = simulate("-a", assembled(os.path.join(ASM_DIR, prog + ".s")), cfg, CONFIGS[cfg], ["-p"])
			check(r.code == 0, "exit %d" % r.code)
			text = open(r.path("pipevis.txt")).read()
			lines = text.splitlines()
			check(len(lines) == r.stats.get("Total Cycles"), "pipevis has %d lines for %s cycles" % (len(lines), r.stats.get("Total Cycles")))
			check_golden_text(os.path.join("pipevis", "%s@%s.txt" % (prog, cfg)), text, "pipevis.txt")
		return test

	for prog, cfg in [("demo_01_basics", "ideal"), ("demo_03_hazards", "ideal"), ("demo_03_hazards", "ideal_nofwd"),
			("demo_02_sum_loop", "ideal"), ("demo_02_sum_loop", "ideal_taken"), ("isa_hazards", "full"),
			("isa_jumps", "ideal_btfn"), ("system_calls", "l1"), ("ebreak", "ideal")]:
		g.run("pipevis_%s@%s" % (prog, cfg), pipevis(prog, cfg))

	def dumps(prog, cfg):
		def test():
			r = simulate("-a", assembled(os.path.join(ASM_DIR, prog + ".s")), cfg, CONFIGS[cfg], ["-r"])
			check(r.code == 0, "exit %d" % r.code)
			check_golden_text(os.path.join("dumps", "%s@%s.register_dump" % (prog, cfg)),
				open(r.path("register_dump")).read(), "register_dump")
			check_golden_text(os.path.join("dumps", "%s@%s.internal_register_dump" % (prog, cfg)),
				open(r.path("internal_register_dump")).read(), "internal_register_dump")
		return test
	g.run("dumps_demo_01_basics@ideal", dumps("demo_01_basics", "ideal"))
	g.run("dumps_isa_loadstore@full", dumps("isa_loadstore", "full"))

	def memory_dump(prog, cfg):
		def test():
			r = simulate("-a", assembled(os.path.join(ASM_DIR, prog + ".s")), cfg, CONFIGS[cfg], ["-m"])
			check(r.code == 0, "exit %d" % r.code)
			digest = hashlib.sha256(open(r.path("memory_dump.bin"), "rb").read()).hexdigest()
			check_golden_text(os.path.join("dumps", "%s@%s.memory_dump.sha256" % (prog, cfg)), digest + "\n", "memory_dump.bin sha256")
		return test
	g.run("memory_dump_isa_loadstore@ideal", memory_dump("isa_loadstore", "ideal"))
	g.run("memory_dump_recursion@tiny", memory_dump("recursion", "tiny"))

	def traces(prog, cfg):
		def test():
			r = simulate("-a", assembled(os.path.join(ASM_DIR, prog + ".s")), cfg, CONFIGS[cfg], ["-t"])
			check(r.code == 0, "exit %d" % r.code)
			itrace = open(r.path("instruction_trace.txt")).read()
			dtrace = open(r.path("data_trace.txt")).read()
			check(len(itrace.splitlines()) == r.stats.get("Instruction Reads"), "instruction trace length != Instruction Reads")
			check(len(dtrace.splitlines()) == r.stats.get("Data Reads", 0) + r.stats.get("Data Writes", 0),
				"data trace length != Data Reads + Data Writes")
			check_golden_text(os.path.join("traces", "%s@%s.instruction_trace.txt" % (prog, cfg)), itrace, "instruction_trace.txt")
			check_golden_text(os.path.join("traces", "%s@%s.data_trace.txt" % (prog, cfg)), dtrace, "data_trace.txt")
		return test
	g.run("traces_demo_04_array@full", traces("demo_04_array", "full"))
	g.run("traces_isa_loadstore@ideal", traces("isa_loadstore", "ideal"))
	return g.finish()

def group_cachesim():
	g = Group("cachesim")

	def replay(name, trace):
		def test():
			d = fresh_dir("cachesim_" + name)
			code, out, err = run([CACHESIM], d, stdin=trace.encode())
			check(code == 1, "cachesim exit status %d (it currently always returns 1)" % code)
			check_golden_text(os.path.join("cachesim", name + ".txt"), out, "cachesim output")
		return test

	seq = "".join("r %#08x\n" % (0x40000 + 4 * i) for i in range(64))
	g.run("sequential_reads", replay("sequential_reads", seq))
	conflict = "".join("%s %#08x\n" % ("w" if i % 3 == 0 else "r", 0x40000 + (i % 12) * 0x800) for i in range(96))
	g.run("conflicting_reads_writes", replay("conflicts", conflict))
	mixed = "".join("i %#08x\nr %#08x\nw %#08x\n" % (4 * i, 0x40000 + 8 * i, 0x40000 + 8 * i + 4) for i in range(80))
	g.run("instruction_and_data_mix", replay("mixed", mixed))

	def first_access_latencies():
		d = fresh_dir("cachesim_lat")
		# cachesim has its own built in machine: L1 1, L2 2, L3 3, DRAM 4
		code, out, err = run([CACHESIM], d, stdin=b"r 0x40000\nr 0x40000\nr 0x40010\nr 0x40020\nw 0x50000\ni 0x0\n")
		lines = out.splitlines()[:6]
		check(lines == ["1 : 10", "2 : 1", "3 : 3", "4 : 6", "5 : 13", "6 : 10"], "cachesim access latencies %s" % lines)
	g.run("built_in_machine_latencies", first_access_latencies)

	def bad_operation():
		d = fresh_dir("cachesim_bad")
		code, out, err = run([CACHESIM], d, stdin=b"r 0x40000\nx 0x40000\n")
		check(code == 1 and out.endswith("Illegal operation"), "exit %d output %r" % (code, out[-60:]))
	g.run("illegal_operation", bad_operation)
	return g.finish()

###############################################################################
# C programs
###############################################################################

def toolchain_version():
	code, out, err = run([RISCV_GCC, "--version"], SESSION)
	return out.splitlines()[0] if out else "unknown"

def compile_c(sources, opt, name):
	d = os.path.join(ROOT, "build", "tests", "c")
	os.makedirs(d, exist_ok=True)
	elf = os.path.join(d, "%s%s.elf" % (name, opt))
	inputs = [os.path.join(ESSENTIALS, "crt0.S")] + sources + \
		[os.path.join(ESSENTIALS, "stdlib", f) for f in sorted(os.listdir(os.path.join(ESSENTIALS, "stdlib")))] + \
		[os.path.join(ESSENTIALS, "memcpy.c")]
	if os.path.exists(elf) and os.path.getmtime(elf) >= max(os.path.getmtime(f) for f in inputs + [os.path.join(C_DIR, "c_test.h")]):
		return elf
	cmd = [RISCV_GCC, "-march=rv32i", "-mabi=ilp32", "-nostdlib", "-ffreestanding", "-fno-builtin",
		"-I" + C_DIR, opt, "-T", os.path.join(ESSENTIALS, "linker.ld")] + inputs + ["-o", elf]
	code, out, err = run(cmd, d)
	check(code == 0, "riscv64-unknown-elf-gcc failed\n%s" % tail(out + err, 20))
	return elf

def host_result(src):
	if not HOST_CC:
		raise Skip("no host C compiler")
	d = fresh_dir("host_" + os.path.basename(src))
	code, out, err = run([HOST_CC, "-DHOST", "-O1", "-o", "prog", src], d)
	check(code == 0, "host compile failed\n%s" % tail(out + err))
	code, out, err = run([os.path.join(d, "prog")], d)
	return int(out.strip()) & 0xFFFFFFFF

def group_c(timing, slow):
	g = Group("c_programs_slow" if slow else "c_programs")
	version = toolchain_version() if RISCV_GCC else None

	def program(src, opt, expected, configs, timeout=120):
		def test():
			if not RISCV_GCC:
				raise Skip("riscv64-unknown-elf-gcc not installed")
			exp = expected() if callable(expected) else expected
			elf = compile_c([src], opt, os.path.splitext(os.path.basename(src))[0])
			problems = []
			with futures.ThreadPoolExecutor(max_workers=len(configs)) as pool:
				jobs = {c: pool.submit(simulate, "-e", elf, c, C_CONFIGS[c], ["-r"], None, timeout) for c in configs}
				for c, j in jobs.items():
					r = j.result()
					if r.code != 0:
						problems.append("[%s] exit %d\n%s" % (c, r.code, tail(r.out + r.err)))
						continue
					got = r.regs().get(10)
					if got != exp:
						problems.append("[%s] main returned (r10) 0x%08x, expected 0x%08x" % (c, got, exp))
					key = "%s%s@%s" % (os.path.basename(src), opt, c)
					d = timing.compare(key, r.stats)
					if d and timing.data.get("_toolchain") == version:
						problems.append("timing " + d)
			if ARGS["update"]:
				timing.new["_toolchain"] = version
			check(not problems, "\n".join(problems))
		return test

	pc = os.path.join(ROOT, "programs", "c")
	fast = list(C_CONFIGS)
	if not slow:
		g.run("factorial-O0", program(os.path.join(pc, "factorial.c"), "-O0", 120, fast))
		g.run("nth_fibonacci-O2", program(os.path.join(pc, "nth_fibonacci.c"), "-O2", 6765, fast))
		g.run("matrix_multiplication-O2", program(os.path.join(pc, "test_matrix_multiplication.c"), "-O2", 1, fast))
		for f in sorted(os.listdir(C_DIR)):
			if f.endswith(".c"):
				src = os.path.join(C_DIR, f)
				for opt in ["-O0", "-O2"]:
					g.run("%s%s" % (f[:-2], opt), program(src, opt, lambda src=src: host_result(src), fast))
	else:
		g.run("quick_sort-O2", program(os.path.join(pc, "quick_sort.c"), "-O2", 0, ["c_ideal", "c_full"], timeout=600))
		g.run("radix_sort-O2", program(os.path.join(pc, "radix_sort.c"), "-O2", 0, ["c_ideal", "c_full"], timeout=600))
	if version and timing.data.get("_toolchain") not in (None, version) and not ARGS["update"]:
		print("  note: C timing not compared, golden made with '%s', this is '%s'" % (timing.data.get("_toolchain"), version))
	return g.finish()

###############################################################################
# main
###############################################################################

def main():
	for a in sys.argv[1:]:
		if a == "--slow":
			ARGS["slow"] = True
		elif a == "--update-golden":
			ARGS["update"] = True
		elif a == "--keep":
			ARGS["keep"] = True
		elif a == "-v":
			ARGS["verbose"] = True
		elif a.startswith("-"):
			print(__doc__)
			return 2
		else:
			ARGS["filters"].append(a)

	for b in (ASSEMBLE, SIMULATE, CACHESIM):
		if not os.access(b, os.X_OK):
			print("missing %s, build it first (make)" % b)
			return 2

	global TIMING
	failures = 0
	timing = TIMING = GoldenTiming("timing.json")
	c_timing = GoldenTiming("timing_c.json")
	try:
		failures += group_assembler_cli()
		failures += group_assembler_oracle()
		failures += group_simulator_cli()
		progs = programs()
		selected = [p for p in progs if not ARGS["filters"] or
			any(f in "programs::%s timing::%s" % (p.name, p.name) for f in ARGS["filters"])]
		failures += group_programs(selected, run_matrix(selected), timing)
		failures += group_documented()
		failures += group_golden_outputs()
		failures += group_cachesim()
		failures += group_c(c_timing, slow=False)
		if ARGS["slow"]:
			failures += group_c(c_timing, slow=True)
	finally:
		timing.save()
		c_timing.save()
		if ARGS["keep"]:
			print("run directories kept in %s" % SESSION)
		else:
			shutil.rmtree(SESSION, ignore_errors=True)
	if ARGS["update"]:
		print("golden files updated in %s, review them with git diff" % os.path.relpath(GOLDEN, ROOT))
	return 1 if failures else 0

if __name__ == "__main__":
	sys.exit(main())
