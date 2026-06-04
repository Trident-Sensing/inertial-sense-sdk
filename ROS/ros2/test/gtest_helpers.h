#include <gtest/gtest.h>

#include <cstdio>

// Colored progress output for tests. We deliberately avoid gtest's internal
// ColoredPrintf/GTestColor: those are private implementation details that
// differ across versions (Humble exposes them in the public header, Jazzy
// hides them inside gtest.cc), which makes any direct use non-portable. gtest
// itself emits color via raw ANSI escapes, so we do the same: green prefix,
// yellow message, reset.
#define PRINTF(...)                                  \
    do {                                             \
        printf("\033[0;32m[          ] \033[0;33m"); \
        printf(__VA_ARGS__);                         \
        printf("\033[0m");                           \
    } while (0)

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
