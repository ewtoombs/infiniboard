// vi:fo=qacj com=b\://


#include <iostream>
using namespace std;

#include "helpers.hpp"

int main(int argc, const char **argv)
{
    unsigned nx = 11;
    complex<float> *x = linspacecf(0.f, 10.f, nx);
    for (unsigned a = 0; a < nx; a++)
        cout << x[a] << endl;
    cout << endl;

    complex<float> *y;
    unsigned ny;
    line_strip_to_lines(x, nx, &y, &ny);
    for (unsigned a = 0; a < ny; a++)
        cout << y[a] << endl;
    cout << endl;

    free(x);
    free(y);


    nx = 3;
    x = linspacecf(2.f, 4.f + 2if, nx);
    for (unsigned a = 0; a < nx; a++)
        cout << x[a] << endl;
    cout << endl;

    line_strip_to_lines(x, nx, &y, &ny);
    for (unsigned a = 0; a < ny; a++)
        cout << y[a] << endl;
    cout << endl;

    free(x);
    free(y);

    return 0;
}
