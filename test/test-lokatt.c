#include <stdio.h>

#include "test.h"

int main()
{
        const struct test *t;

        for (t = &__start_test_section; t < &__stop_test_section; t++) {
                printf("%s/%s\n", t->namespace, t->name);
        }

        return 0;
}
