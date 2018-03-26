#ifndef TEST_H
#define TEST_H
#include "lokatt/error.h"

/* inspired by the tests in the Wayland project */

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
        static void lokatt_test_##namespace##_##name()

#define EXIT_VALGRIND 126
#define EXIT_SKIPPED 127

#define ASSERT_EQ(expr, value)                                                 \
        do {                                                                   \
                if ((expr) != (value))                                         \
                        die("assertion '%s' == '%s' failed", #expr, #value);   \
        } while (0)

#define ASSERT_NE(expr, value)                                                 \
        do {                                                                   \
                if ((expr) == (value))                                         \
                        die("assertion '%s' != '%s' failed", #expr, #value);   \
        } while (0)

#define ASSERT_GT(expr, value)                                                 \
        do {                                                                   \
                if ((expr) <= (value))                                         \
                        die("assertion '%s' > '%s' failed", #expr, #value);    \
        } while (0)

#define ASSERT_GE(expr, value)                                                 \
        do {                                                                   \
                if ((expr) < (value))                                          \
                        die("assertion '%s' >= '%s' failed", #expr, #value);   \
        } while (0)

#define ASSERT_LT(expr, value)                                                 \
        do {                                                                   \
                if ((expr) >= (value))                                         \
                        die("assertion '%s' < '%s' failed", #expr, #value);    \
        } while (0)

#define ASSERT_LE(expr, value)                                                 \
        do {                                                                   \
                if ((expr) > (value))                                          \
                        die("assertion '%s' <= '%s' failed", #expr, #value);   \
        } while (0)

#endif
