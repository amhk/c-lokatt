#ifndef TEST_H
#define TEST_H

/* inspired by the tests in the Wayland project */

const char *test_data_dir(void);
void __fail(const char *file, unsigned int line, const char *func,
            const char *fmt, ...)
    __attribute__((__noreturn__, __format__(__printf__, 4, 5)));

struct test {
        const char *namespace;
        const char *name;
        void (*func)(void);
};

extern const struct test __start_test_section, __stop_test_section;

#define TEST(namespace, name)                                                  \
        static void lokatt_test_##namespace##_##name(void);                    \
        const struct test test_##namespace##_##name                            \
            __attribute__((section("test_section"))) = {                       \
                #namespace, #name, lokatt_test_##namespace##_##name};          \
        static void lokatt_test_##namespace##_##name(void)

#define EXIT_VALGRIND 126
#define EXIT_SKIPPED 127

#define ASSERT_EQ(lhs, rhs)                                                    \
        do {                                                                   \
                if ((lhs) != (rhs)) {                                          \
                        __fail(__FILE__, __LINE__, __FUNCTION__,               \
                               "assertion '%s' == '%s' failed", #lhs, #rhs);   \
                }                                                              \
        } while (0)

#define ASSERT_NE(lhs, rhs)                                                    \
        do {                                                                   \
                if ((lhs) == (rhs)) {                                          \
                        __fail(__FILE__, __LINE__, __FUNCTION__,               \
                               "assertion '%s' != '%s' failed", #lhs, #rhs);   \
                }                                                              \
        } while (0)

#define ASSERT_GT(lhs, rhs)                                                    \
        do {                                                                   \
                if ((lhs) <= (rhs)) {                                          \
                        __fail(__FILE__, __LINE__, __FUNCTION__,               \
                               "assertion '%s' > '%s' failed", #lhs, #rhs);    \
                }                                                              \
        } while (0)

#define ASSERT_GE(lhs, rhs)                                                    \
        do {                                                                   \
                if ((lhs) < (rhs)) {                                           \
                        __fail(__FILE__, __LINE__, __FUNCTION__,               \
                               "assertion '%s' >= '%s' failed", #lhs, #rhs);   \
                }                                                              \
        } while (0)

#define ASSERT_LT(lhs, rhs)                                                    \
        do {                                                                   \
                if ((lhs) >= (rhs)) {                                          \
                        __fail(__FILE__, __LINE__, __FUNCTION__,               \
                               "assertion '%s' < '%s' failed", #lhs, #rhs);    \
                }                                                              \
        } while (0)

#define ASSERT_LE(lhs, rhs)                                                    \
        do {                                                                   \
                if ((lhs) > (rhs)) {                                           \
                        __fail(__FILE__, __LINE__, __FUNCTION__,               \
                               "assertion '%s' <= '%s' failed", #lhs, #rhs);   \
                }                                                              \
        } while (0)

#endif
