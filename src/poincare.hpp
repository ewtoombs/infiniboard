// vi:fo=qacj com=b\://

#pragma once

#include <complex>
#include <cmath>

namespace poincare {

complex<float> S(std::complex<float> a, complex<float> x);
void tiling(unsigned p, unsigned q, unsigned res, unsigned niter,
        complex<float> **py, unsigned *pny);

}
