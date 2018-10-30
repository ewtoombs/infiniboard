// vi:fo=qacj com=b\://

#include <time.h>

#include <iostream>
using namespace std;

#include "helpers.hpp"

void callback(void *data)
{
    print(dtime());
}
int main(int argc, const char **argv)
{
    cout.precision(16);

    timer_t timer = create_callback_timer(&callback, NULL);
    print(dtime());
    timer_settime_d(timer, 0.1);
    dsleep(1.);
    return 0;
}
