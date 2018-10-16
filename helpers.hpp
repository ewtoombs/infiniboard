#pragma once

#include <complex>
#include <cstring>
#include <cstdlib> // for NULL

using namespace std;

double hermite(unsigned n, double u);
double sq(double x);
double fact(unsigned n);
double modsq(complex<double> x);
double pown(double x, unsigned n);
double pown_l(double x, unsigned n);
