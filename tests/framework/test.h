// Minimal unit test framework.
//
// Every test case runs in its own forked child process, inside its own fresh
// temporary working directory. This gives us:
//   - full isolation of the simulator's global/static state between tests
//   - a crash (SIGSEGV...) or an unexpected exit(1) from log_fatal() in the
//     code under test is reported as a failure of that test only
//   - files written by the code under test (log.log, dumps...) never leak
//
// Usage:
//   TEST(adds_numbers){ CHECK_EQ(1 + 1, 2); }
//   TEST_SUITE("alu", T(adds_numbers), XFAIL(known_bug, "why it fails"));
//
// A test listed with XFAIL() documents a known bug: it asserts the CORRECT
// behaviour and is expected to fail on the current code. It does not fail the
// suite. When the bug gets fixed it shows up as XPASS - move it to T().
//
// Command line of every test binary:
//   ./test_x [-v] [filter...]    -v shows the stdout of the code under test,
//                                filters select tests by substring.

#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <ftw.h>
#include <sys/wait.h>
#include <sys/stat.h>

typedef struct test_case {
	const char * name;
	void (*fn)(void);
	const char * xfail;	// NULL for a normal test, the reason for a known failure otherwise
} test_case;

#define TEST(fn) static void fn(void)
#define T(fn) {#fn, fn, NULL}
#define XFAIL(fn, reason) {#fn, fn, reason}

#define TEST_PASS_CODE 100
#define TEST_FAIL_CODE 101
#define TEST_TIMEOUT_SECONDS 20

static int test_failures = 0;
static FILE * test_fail_log = NULL;	// failure details of the running test
static int test_verbose = 0;

static void test_report(const char * file, int line, const char * fmt, ...) __attribute__((format(printf, 3, 4)));

#include <stdarg.h>
static void test_report(const char * file, int line, const char * fmt, ...){
	test_failures++;
	FILE * out = test_fail_log ? test_fail_log : stderr;
	fprintf(out, "      %s:%d: ", file, line);
	va_list ap;
	va_start(ap, fmt);
	vfprintf(out, fmt, ap);
	va_end(ap);
	fprintf(out, "\n");
	fflush(out);
}

//------------------------------------------------------------------------------
// assertions (they record the failure and continue with the test)
//------------------------------------------------------------------------------

#define CHECK(cond) do { \
	if(!(cond)) test_report(__FILE__, __LINE__, "CHECK(%s) failed", #cond); \
} while(0)

#define CHECK_MSG(cond, ...) do { \
	if(!(cond)) test_report(__FILE__, __LINE__, __VA_ARGS__); \
} while(0)

#define CHECK_EQ(actual, expected) do { \
	uint64_t a_ = (uint64_t)(actual), e_ = (uint64_t)(expected); \
	if(a_ != e_) test_report(__FILE__, __LINE__, \
		"CHECK_EQ(%s, %s) failed\n        expected: 0x%08llx (%lld)\n        actual:   0x%08llx (%lld)", \
		#actual, #expected, (unsigned long long)e_, (long long)e_, (unsigned long long)a_, (long long)a_); \
} while(0)

#define CHECK_STR_EQ(actual, expected) do { \
	const char * a_ = (actual), * e_ = (expected); \
	if(a_ == NULL || e_ == NULL || strcmp(a_, e_) != 0) test_report(__FILE__, __LINE__, \
		"CHECK_STR_EQ(%s, %s) failed\n        expected: \"%s\"\n        actual:   \"%s\"", \
		#actual, #expected, e_ ? e_ : "(null)", a_ ? a_ : "(null)"); \
} while(0)

#define CHECK_CONTAINS(haystack, needle) do { \
	const char * h_ = (haystack), * n_ = (needle); \
	if(h_ == NULL || strstr(h_, n_) == NULL) test_report(__FILE__, __LINE__, \
		"CHECK_CONTAINS(%s, \"%s\") failed\n        in: \"%s\"", #haystack, n_, h_ ? h_ : "(null)"); \
} while(0)

//------------------------------------------------------------------------------
// running code that is expected to terminate the process
//------------------------------------------------------------------------------

// Runs stmt in a child process and stores its exit status (or 128 + signal) in
// status. Everything the child printed to stdout/stderr is stored in out (a char array).
#define RUN_ISOLATED(out, status, stmt) do { \
	int fd_ = test_isolate_begin(); \
	pid_t pid_ = fork(); \
	if(pid_ == 0){ \
		dup2(fd_, 1); \
		dup2(fd_, 2); \
		stmt; \
		fflush(stdout); \
		_exit(0); \
	} \
	(status) = test_isolate_end(fd_, pid_, out, sizeof(out)); \
} while(0)

static int test_isolate_begin(void){
	char path[] = "isolated_output_XXXXXX";
	int fd = mkstemp(path);
	unlink(path);
	fflush(stdout);
	fflush(stderr);
	return fd;
}

static int test_isolate_end(int fd, pid_t pid, char * out, size_t out_size){
	int status = 0;
	waitpid(pid, &status, 0);
	lseek(fd, 0, SEEK_SET);
	ssize_t n = read(fd, out, out_size - 1);
	out[n > 0 ? n : 0] = '\0';
	close(fd);
	if(WIFEXITED(status)) return WEXITSTATUS(status);
	return 128 + WTERMSIG(status);
}

// ends an isolated child with an explicit status (flushes what it printed)
#define TEST_EXIT(code) do { int c_ = (code); fflush(stdout); fflush(stderr); _exit(c_); } while(0)

// stmt must make the process exit with status 1 and print "[FATAL]: <msg>"
#define CHECK_FATAL(stmt, msg) do { \
	char out_[4096]; \
	int st_; \
	RUN_ISOLATED(out_, st_, stmt); \
	if(st_ != 1) test_report(__FILE__, __LINE__, \
		"CHECK_FATAL(%s) failed: expected exit status 1, got %d\n        output: \"%s\"", #stmt, st_, out_); \
	else if(strstr(out_, "[FATAL]: " msg) == NULL) test_report(__FILE__, __LINE__, \
		"CHECK_FATAL(%s) failed: expected \"[FATAL]: %s\"\n        output: \"%s\"", #stmt, msg, out_); \
} while(0)

// stmt must NOT terminate the process
#define CHECK_SURVIVES(stmt) do { \
	char out_[4096]; \
	int st_; \
	RUN_ISOLATED(out_, st_, stmt); \
	if(st_ != 0) test_report(__FILE__, __LINE__, \
		"CHECK_SURVIVES(%s) failed: process ended with status %d\n        output: \"%s\"", #stmt, st_, out_); \
} while(0)

//------------------------------------------------------------------------------
// helpers
//------------------------------------------------------------------------------

// reads a whole (small) file into a static buffer, NULL if it can't be read
static char * test_read_file(const char * path){
	static char buf[1 << 16];
	FILE * f = fopen(path, "rb");
	if(!f) return NULL;
	size_t n = fread(buf, 1, sizeof(buf) - 1, f);
	buf[n] = '\0';
	fclose(f);
	return buf;
}

static void test_write_file(const char * path, const char * content){
	FILE * f = fopen(path, "wb");
	if(!f){
		perror(path);
		exit(TEST_FAIL_CODE);
	}
	fputs(content, f);
	fclose(f);
}

//------------------------------------------------------------------------------
// runner
//------------------------------------------------------------------------------

static int test_rm_entry(const char * path, const struct stat * sb, int flag, struct FTW * ftw){
	(void)sb; (void)flag; (void)ftw;
	return remove(path);
}

static void test_print_file(const char * path, const char * prefix, int max_lines){
	FILE * f = fopen(path, "r");
	if(!f) return;
	// show the last max_lines lines (that is where the fatal message is)
	char lines[64][512];
	int count = 0;
	while(fgets(lines[count % 64], sizeof(lines[0]), f)) count++;
	fclose(f);
	int start = count > max_lines ? count - max_lines : 0;
	for(int i = start; i < count; i++){
		char * l = lines[i % 64];
		size_t len = strlen(l);
		if(len && l[len - 1] == '\n') l[len - 1] = '\0';
		printf("%s%s\n", prefix, l);
	}
}

static int test_matches(const char * name, int nfilters, char ** filters){
	if(nfilters == 0) return 1;
	for(int i = 0; i < nfilters; i++){
		if(strstr(name, filters[i])) return 1;
	}
	return 0;
}

static int test_main(const char * suite, const test_case * tests, int n, int argc, char ** argv){
	char * filters[64];
	int nfilters = 0;
	for(int i = 1; i < argc; i++){
		if(strcmp(argv[i], "-v") == 0) test_verbose = 1;
		else if(nfilters < 64) filters[nfilters++] = argv[i];
	}

	int passed = 0, failed = 0, xfailed = 0, xpassed = 0;
	const int use_color = isatty(1);
	const char * red = use_color ? "\033[31m" : "", * green = use_color ? "\033[32m" : "";
	const char * yellow = use_color ? "\033[33m" : "", * reset = use_color ? "\033[0m" : "";

	char base[] = "/tmp/simtest_XXXXXX";
	const char * tmp = getenv("TMPDIR");
	char base_buf[4096];
	if(tmp && *tmp){
		snprintf(base_buf, sizeof(base_buf), "%s/simtest_XXXXXX", tmp);
	}else{
		snprintf(base_buf, sizeof(base_buf), "%s", base);
	}
	if(mkdtemp(base_buf) == NULL){
		perror("mkdtemp");
		return 2;
	}

	printf("=== %s ===\n", suite);
	for(int i = 0; i < n; i++){
		const test_case * t = &tests[i];
		if(!test_matches(t->name, nfilters, filters)) continue;

		char dir[4352], fail_path[4400], out_path[4400];
		snprintf(dir, sizeof(dir), "%s/%s", base_buf, t->name);
		mkdir(dir, 0755);
		snprintf(fail_path, sizeof(fail_path), "%s/.failures", dir);
		snprintf(out_path, sizeof(out_path), "%s/.stdout", dir);

		fflush(stdout);
		pid_t pid = fork();
		if(pid == 0){
			if(chdir(dir) != 0) _exit(2);
			test_fail_log = fopen(fail_path, "w");
			if(!test_verbose){
				int fd = open(out_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
				dup2(fd, 1);
				close(fd);
			}
			alarm(TEST_TIMEOUT_SECONDS);
			t->fn();
			fflush(stdout);
			fclose(test_fail_log);
			_exit(test_failures ? TEST_FAIL_CODE : TEST_PASS_CODE);
		}

		int status = 0;
		waitpid(pid, &status, 0);
		int ok = WIFEXITED(status) && WEXITSTATUS(status) == TEST_PASS_CODE;
		char reason[256] = "";
		if(WIFSIGNALED(status)){
			int sig = WTERMSIG(status);
			snprintf(reason, sizeof(reason), "crashed with signal %d (%s)%s", sig, strsignal(sig),
				sig == SIGALRM ? " - timed out" : "");
		}else if(WIFEXITED(status) && WEXITSTATUS(status) != TEST_PASS_CODE && WEXITSTATUS(status) != TEST_FAIL_CODE){
			snprintf(reason, sizeof(reason), "code under test terminated the process with exit(%d)", WEXITSTATUS(status));
		}

		if(t->xfail){
			if(ok){
				xpassed++;
				printf("  %sXPASS%s %s  (known bug seems FIXED, move it from XFAIL() to T(): %s)\n", yellow, reset, t->name, t->xfail);
			}else{
				xfailed++;
				printf("  %sXFAIL%s %s  (known bug: %s)\n", yellow, reset, t->name, t->xfail);
				if(test_verbose){
					if(reason[0]) printf("      %s\n", reason);
					test_print_file(fail_path, "", 64);
				}
			}
		}else if(ok){
			passed++;
			printf("  %sPASS%s  %s\n", green, reset, t->name);
		}else{
			failed++;
			printf("  %sFAIL%s  %s\n", red, reset, t->name);
			if(reason[0]) printf("      %s\n", reason);
			test_print_file(fail_path, "", 64);
			if(reason[0] && !test_verbose){
				printf("      last output of the code under test:\n");
				test_print_file(out_path, "        | ", 8);
			}
		}
	}

	nftw(base_buf, test_rm_entry, 16, FTW_DEPTH | FTW_PHYS);

	printf("--- %s: %d passed, %d failed, %d xfail, %d xpass\n", suite, passed, failed, xfailed, xpassed);
	// machine readable line for tests/run_tests.sh
	printf("@@RESULT %s %d %d %d %d\n", suite, passed, failed, xfailed, xpassed);
	return failed ? 1 : 0;
}

#define TEST_SUITE(suite_name, ...) \
	static const test_case test_list_[] = { __VA_ARGS__ }; \
	int main(int argc, char ** argv){ \
		return test_main(suite_name, test_list_, sizeof(test_list_) / sizeof(test_list_[0]), argc, argv); \
	}

#endif
