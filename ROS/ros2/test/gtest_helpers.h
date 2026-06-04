#include <gtest/gtest.h>

// Older gtest exposes testing::internal::ColoredPrintf; newer gtest removed it.
// Define GTEST_HAS_COLORED_PRINTF (e.g. via the build system) to use it.
// if we failed to define, assume that it is a newer version and thus colored printf is not included
#ifndef GTEST_HAS_COLORED_PRINTF
# define GTEST_HAS_COLORED_PRINTF 0
#endif
namespace testing
{
    namespace internal
    {
#if !GTEST_HAS_COLORED_PRINTF
        enum GTestColor {
            COLOR_DEFAULT,
            COLOR_RED,
            COLOR_GREEN,
            COLOR_YELLOW
        };
#endif

        extern void ColoredPrintf(GTestColor color, const char* fmt, ...);
    }
}
#if !GTEST_HAS_COLORED_PRINTF
#define PRINTF(...)  do { testing::internal::ColoredPrintf(testing::internal::COLOR_GREEN, "[          ] "); testing::internal::ColoredPrintf(testing::internal::COLOR_YELLOW, __VA_ARGS__); } while(0)
#else
#define PRINTF(...)
#endif

// C++ stream interface
class TestCout : public std::stringstream
{
public:
    ~TestCout()
    {
        printf("%s",str().c_str());
    }
};

#define TEST_COUT  TestCout()
