#include <stdio.h>

#include "lokatt/bar.h"
#include "lokatt/foo.h"

int main()
{
        printf("foo_bar=%d\n", foo_bar());
        printf("bar_bar=%d\n", bar_bar());
        return 0;
}
