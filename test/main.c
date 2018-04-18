#include <getopt.h>
#include <libgen.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "lokatt/error.h"
#include "lokatt/wrappers.h"

#include "test.h"

#define ANSI_GREEN "\033[32m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_RED "\033[31m"
#define ANSI_RESET "\033[0m"

extern const struct test __start_test_section, __stop_test_section;

static char test_data_dir_[1024];
static struct {
        char *filter;
        int list;
} opts = {NULL, 0};

const char *test_data_dir()
{
        return test_data_dir_;
}

static void on_exit(void)
{
        xfree(opts.filter);
}

static void init_test_data_dir(char *argv0)
{
        snprintf(test_data_dir_, sizeof(test_data_dir_), "%s/../test/data",
                 dirname(argv0));
}

struct results {
        int passed;
        int failed;
        int valgrind;
        int skipped;
};

static void cprintf(const char *color, const char *fmt, ...)
{
        va_list ap;
        va_start(ap, fmt);
        printf("%s", color);
        vprintf(fmt, ap);
        printf("%s", ANSI_RESET);
        va_end(ap);
}

static void run_single_test(const struct test *t, struct results *r)
{
        cprintf(ANSI_RESET, "[ RUN      ] %s/%s\n", t->namespace, t->name);
        switch (fork()) {
        case -1:
                /* error */
                die("fork");
        case 0:
                /* child */
                t->func();
                exit(EXIT_SUCCESS);
        }
        /* parent */
        siginfo_t info;
        if (waitid(P_ALL, 0, &info, WEXITED) != 0) {
                die("waitid");
        }
        if (info.si_code == CLD_EXITED && info.si_status == EXIT_SUCCESS) {
                r->passed++;
                cprintf(ANSI_RESET, "[       OK ] %s/%s\n", t->namespace,
                        t->name);
        } else if (info.si_code == CLD_EXITED &&
                   info.si_status == EXIT_VALGRIND) {
                r->failed++;
                r->valgrind++;
                cprintf(ANSI_RED, "[ VALGRIND ] %s/%s\n", t->namespace,
                        t->name);
        } else if (info.si_code == CLD_EXITED &&
                   info.si_status == EXIT_SKIPPED) {
                r->skipped++;
                cprintf(ANSI_YELLOW, "[  SKIPPED ] %s/%s\n", t->namespace,
                        t->name);
        } else {
                r->failed++;
                cprintf(ANSI_RED, "[     FAIL ] %s/%s\n", t->namespace,
                        t->name);
        }
}

static void run_all_tests(const char *namespace, const char *name,
                          struct results *r)
{
        for (const struct test *t = &__start_test_section;
             t < &__stop_test_section; t++) {
                int run = 0;

                if (!namespace) {
                        run = 1;
                } else if (!strcmp(namespace, t->namespace)) {
                        run = !name || !strcmp(name, t->name);
                }

                if (run) {
                        run_single_test(t, r);
                }
        }
}

static void print_results(const struct results *r)
{
        if (r->failed == 0 && r->passed > 0) {
                cprintf(ANSI_GREEN, "[  PASSED  ] %d test%s\n", r->passed,
                        r->passed == 1 ? "" : "s");
        } else if (r->failed > 0) {
                cprintf(ANSI_RED, "[   FAIL   ] %d test%s\n", r->failed,
                        r->failed == 1 ? "" : "s");
        } else {
                cprintf(ANSI_YELLOW, "[ NO TESTS ] 0 tests\n");
        }
}

static void list_all_tests(void)
{
        for (const struct test *t = &__start_test_section;
             t < &__stop_test_section; t++) {
                printf("%s/%s\n", t->namespace, t->name);
        }
}

static void parse_args(int argc, char **argv)
{
        while (1) {
                static const struct option long_options[] = {
                    {"filter", required_argument, 0, 'f'},
                    {"list", no_argument, 0, 'l'},
                    {0, 0, 0, 0},
                };
                int index;
                int c = getopt_long_only(argc, argv, "", long_options, &index);
                switch (c) {
                case -1:
                        return;
                case '?':
                        exit(1);
                case 'f':
                        if (strlen(optarg) > 0) {
                                xfree(opts.filter);
                                opts.filter = xalloc(strlen(optarg) + 1);
                                strcpy(opts.filter, optarg);
                        }
                        break;
                case 'l':
                        opts.list = 1;
                        break;
                }
        }
}

int main(int argc, char **argv)
{
        atexit(on_exit);
        init_test_data_dir(argv[0]);
        parse_args(argc, argv);

        if (opts.list) {
                list_all_tests();
                return 0;
        }

        char *namespace = NULL, *name = NULL;
        if (opts.filter) {
                namespace = opts.filter;
                name = strchr(namespace, '/');
                if (name) {
                        *name++ = '\0';
                }
        }

        struct results r = {0, 0, 0, 0};
        run_all_tests(namespace, name, &r);
        print_results(&r);

        return r.failed;
}
