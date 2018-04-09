#ifndef TEST_H
#define TEST_H
#include "lokatt/error.h"

/* inspired by the tests in the Wayland project */

const char *test_data_dir(void);

struct test {
        const char *namespace;
        const char *name;
        void (*func)(void);
};

extern const struct test __start_test_section, __stop_test_section;

#define TEST(namespace, name)                                                  \
        static void lokatt_test_##namespace##_##name(void);                    \
        const struct test test_##namespace##name                               \
            __attribute__((section("test_section"))) = {                       \
                #namespace, #name, lokatt_test_##namespace##_##name};          \
        static void lokatt_test_##namespace##_##name(void)

#define EXIT_VALGRIND 126
#define EXIT_SKIPPED 127

#define ASSERT_EQ(lhs, rhs)                                                    \
        do {                                                                   \
                if ((lhs) != (rhs))                                            \
                        die("assertion '%s' == '%s' failed", #lhs, #rhs);      \
        } while (0)

#define ASSERT_NE(lhs, rhs)                                                    \
        do {                                                                   \
                if ((lhs) == (rhs))                                            \
                        die("assertion '%s' != '%s' failed", #lhs, #rhs);      \
        } while (0)

#define ASSERT_GT(lhs, rhs)                                                    \
        do {                                                                   \
                if ((lhs) <= (rhs))                                            \
                        die("assertion '%s' > '%s' failed", #lhs, #rhs);       \
        } while (0)

#define ASSERT_GE(lhs, rhs)                                                    \
        do {                                                                   \
                if ((lhs) < (rhs))                                             \
                        die("assertion '%s' >= '%s' failed", #lhs, #rhs);      \
        } while (0)

#define ASSERT_LT(lhs, rhs)                                                    \
        do {                                                                   \
                if ((lhs) >= (rhs))                                            \
                        die("assertion '%s' < '%s' failed", #lhs, #rhs);       \
        } while (0)

#define ASSERT_LE(lhs, rhs)                                                    \
        do {                                                                   \
                if ((lhs) > (rhs))                                             \
                        die("assertion '%s' <= '%s' failed", #lhs, #rhs);      \
        } while (0)

#endif
