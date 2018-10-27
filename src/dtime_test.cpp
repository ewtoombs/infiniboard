// vi:fo=qacj com=b\://

#include <time.h>

#include <iostream>
using namespace std;

#include "helpers.hpp"

int main(int argc, const char **argv)
{
    print(time(NULL));
    cout.precision(16);
    print(dtime());
    return 0;
}
