#include "lokatt/foo.h"

#include "test.h"

TEST(foo, correct)
{
        ASSERT_EQ(foo_bar(), 1234);
}

TEST(foo, incorrect)
{
        ASSERT_NE(foo_bar(), 1);
}
