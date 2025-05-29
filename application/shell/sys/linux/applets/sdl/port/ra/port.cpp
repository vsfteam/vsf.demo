#include <vsf.h>

unsigned long Get_CPU_Clock(unsigned long & high)
{
    high = 0;
    return 0;
}

void Assert_Failure (char *expression, int line, char *file)
{
    vsf_trace_assert(expression, file, line, "unknown_function");
}
