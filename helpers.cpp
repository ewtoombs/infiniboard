#include <iostream>
#include <complex>

using namespace std;

// Computes x^n, where n is a natural number.
double pown(double x, unsigned n)
{
    double y = 1;
    // n = 2*d + r. x^n = (x^2)^d * x^r.
    unsigned d = n >> 1;
    unsigned r = n & 1;
    double x_2_d = d == 0? 1 : pown(x*x, d);
    double x_r = r == 0? 1 : x;
    return x_2_d*x_r;
}
// The linear implementation.
double pown_l(double x, unsigned n)
{
    double y = 1;
    for (unsigned i = 0; i < n; i++)
        y *= x;
    return y;
}

// factorials
double fact(unsigned n)
{
    double m = 1;
    for (unsigned i = n; i; i--)
        m *= i;
    return m;
}

double sq(double x)
{
    return x*x;
}

double modsq(complex<double> x)
{
    return sq(x.real()) + sq(x.imag());
}

// probabilists' Hermite polynomials
double hermite(unsigned n, double u)
{
    double h = 0;
    double c = 1;
    unsigned a = n;
#ifdef HERMITE_TEST
    cout << "H_" << n << " = ";
#endif
    for (;;)
    {
#ifdef HERMITE_TEST
        cout << c << "u^" << a;
#endif
        h += c*pown(u, a);
        if (a < 2)
            break;
#ifdef HERMITE_TEST
        cout << " + ";
#endif
        c *= - (double)(a*(a-1))/(n+2-a);
        a -= 2;
    }
#ifdef HERMITE_TEST
    cout << "\n";
#endif
    return h;
}
