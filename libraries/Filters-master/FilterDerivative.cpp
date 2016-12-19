#include "FilterDerivative.h"
#include <cmath>
#include "os_Task.h"

#ifndef PI
#define   PI         3.14159265358979323846
#endif
#define   TWO_PI     (2 * (PI))

float FilterDerivative::input(float inVal)
{
    long thisUS = os::Task::getTickCount();
    float dt = 1e-3 * float(thisUS - LastUS);   // cast to float here, for math
    LastUS = thisUS;                          // update this now

    Derivative = (inVal - LastInput) / dt;

    LastInput = inVal;
    return output();
}

float FilterDerivative::output() { return Derivative; }

void testFilterDerivative()
{
    FilterDerivative der;

    while (true) {
        float t = 1e-3 * float(os::Task::getTickCount());
        float value = 100 * std::sin(TWO_PI * t);

        der.input(value);

        os::ThisTask::sleep(std::chrono::milliseconds(10));
    }
}
