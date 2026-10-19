#!/usr/bin/env bash
# Runs the test suites and prints one summary for all of them.
#
#   tests/run_tests.sh unit        <unit test binaries...>
#   tests/run_tests.sh integration
#   tests/run_tests.sh all         <unit test binaries...>
#   tests/run_tests.sh slow        <unit test binaries...>   (all + long C workloads)
#
# Normally called through make: make test / test-unit / test-integration / test-slow
# Exit status is non zero if any test FAILED (XFAIL/XPASS do not fail the run).

mode="$1"
shift
root="$(cd "$(dirname "$0")/.." && pwd)"
log="$(mktemp)"
trap 'rm -f "$log"' EXIT

status=0

run_unit(){
	for bin in "$@"; do
		"$bin" | tee -a "$log"
		[ "${PIPESTATUS[0]}" -eq 0 ] || status=1
		echo
	done
}

run_integration(){
	python3 "$root/tests/integration/run.py" "$@" | tee -a "$log"
	[ "${PIPESTATUS[0]}" -eq 0 ] || status=1
	echo
}

case "$mode" in
	unit) run_unit "$@" ;;
	integration) run_integration ;;
	all) run_unit "$@"; run_integration ;;
	slow) run_unit "$@"; run_integration --slow ;;
	*) echo "usage: $0 unit|integration|all|slow [unit binaries...]"; exit 2 ;;
esac

# summary from the "@@RESULT suite passed failed xfail xpass" lines
awk '
	/^@@RESULT / { n++; name[n] = $2; p[n] = $3; f[n] = $4; xf[n] = $5; xp[n] = $6
		tp += $3; tf += $4; txf += $5; txp += $6 }
	END {
		print "==================================== SUMMARY ===================================="
		printf "%-34s %8s %8s %8s %8s\n", "suite", "passed", "FAILED", "xfail", "xpass"
		for(i = 1; i <= n; i++){
			mark = f[i] > 0 ? "   <== FAILING" : (xp[i] > 0 ? "   <== XPASS: a known bug got fixed?" : "")
			printf "%-34s %8d %8d %8d %8d%s\n", name[i], p[i], f[i], xf[i], xp[i], mark
		}
		printf "%-34s %8d %8d %8d %8d\n", "TOTAL", tp, tf, txf, txp
		print "=================================================================================="
		if(tf > 0) print "RESULT: FAILED (" tf " failing tests, details above, search for FAIL)"
		else print "RESULT: OK (" txf " known bugs tracked as xfail, see tests/README.md)"
	}' "$log"

exit $status
