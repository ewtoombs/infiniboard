// vi:fo=qacj com=b\://

#include <assert.h>

#include <cmath>  // Actually c++ math. Supports c++ types.
#include <complex>
using namespace std;
using namespace std::literals;

#include "helpers.hpp"

#include "poincare.hpp"


namespace poincare {

complex<float> S(complex<float> a, complex<float> x)
{
    return (x + a)/(1.f + conj(a)*x);
}


void tiling(unsigned p, unsigned q, unsigned res, unsigned niter,
        complex<float> **py, unsigned *pny)
{
    assert(res >= 2);
    assert(2*p + 2*q < p*q);

    // Some preliminary constants.
    float theta = TAU/p;
    float phi = TAU/q;

    float u = cos((theta + phi)/2);
    float v = cos((theta - phi)/2);
    float d = sqrt(u/v);

    float w = sqrt(u*v);
    float c = w/cos(phi/2);
    float f = w/cos(theta/2);

    complex<float> A = exp(1if*theta);
    // D = B^-1.
    complex<float> D = exp(-1if*phi);


    // pi, the primary edge.
    complex<float> *ls = linspacecf(0, -c*exp(1if*theta/2.f), res);

    complex<float> *pi_A;
    unsigned npi;
    line_strip_to_lines(ls, res, &pi_A, &npi);
    free(ls);

    DEF_ARRAY(complex<float>, pi_B, npi);
    for (unsigned u = 0; u < npi; u++)
        pi_B[u] = S(d, pi_A[u]);


    // Determine how much room is needed for each of alpha, beta, gamma, and
    // delta.
    unsigned nalpha = npi;
    unsigned nbeta = npi;
    unsigned ngamma, ndelta;
    for (unsigned in = 0; in < niter; in++) {
        ngamma = npi + nbeta + (q - 4)*nalpha;
        ndelta = ngamma + nalpha;
        nbeta = npi + ngamma + (p - 4)*ndelta;
        nalpha = nbeta + ndelta;
    }
    // Allocate.
    DEF_ARRAY(complex<float>, alpha, nalpha);
    DEF_ARRAY(complex<float>, beta, nbeta);
    DEF_ARRAY(complex<float>, gamma, ngamma);
    DEF_ARRAY(complex<float>, delta, ndelta);



    // Begin tree building.
    nalpha = npi;
    nbeta = npi;
    COPY_ARRAY(pi_B, alpha, npi);
    COPY_ARRAY(pi_B, beta, npi);
    for (unsigned in = 0; in < niter; in++) {
        // gamma block
        unsigned ngamma = npi + nbeta + (q - 4)*nalpha;

        complex<float> *pos = gamma;

        COPY_ARRAY(pi_B, pos, npi); pos += npi;

        for (unsigned u = 0; u < nbeta; u++)
            pos[u] = D*beta[u];
        pos += nbeta;

        for (unsigned a = 0; a < q - 4; a++) {
            for (unsigned u = 0; u < nalpha; u++)
                pos[u] = pow(D, (float)(a + 2))*alpha[u];
            pos += nalpha;
        }

        for (unsigned u = 0; u < ngamma; u++)
            gamma[u] = S(-d, gamma[u]);


        // delta block
        unsigned ndelta = ngamma + nalpha;

        pos = delta;

        COPY_ARRAY(gamma, pos, ngamma); pos += ngamma;
        for (unsigned u = 0; u < nalpha; u++) {
            complex<float> z = pow(D, (float)(q - 2))*alpha[u];
            pos[u] = S(-d, z);
        }


        // beta block
        nbeta = npi + ngamma + (p - 4)*ndelta;

        pos = beta;

        COPY_ARRAY(pi_A, pos, npi); pos += npi;

        for (unsigned u = 0; u < ngamma; u++)
            pos[u] = A*gamma[u];
        pos += ngamma;

        for (unsigned a = 0; a < p - 4; a++) {
            for (unsigned u = 0; u < ndelta; u++)
                pos[u] = pow(A, (float)(a + 2))*delta[u];
            pos += ndelta;
        }

        for (unsigned u = 0; u < nbeta; u++)
            beta[u] = S(d, beta[u]);


        // alpha block
        nalpha = nbeta + ndelta;

        pos = alpha;

        COPY_ARRAY(beta, pos, nbeta); pos += nbeta;
        for (unsigned u = 0; u < ndelta; u++) {
            complex<float> z = pow(A, (float)(p - 2))*delta[u];
            pos[u] = S(d, z);
        }
    }


    // I block
    unsigned nI = q*nalpha;
    DEF_ARRAY(complex<float>, I, nI);
    complex<float> *pos = I;
    for (unsigned a = 0; a < q; a++) {
        for (unsigned u = 0; u < nalpha; u++)
            pos[u] = pow(D, (float)a)*alpha[u];
        pos += nalpha;
    }


    free(pi_A);
    free(pi_B);
    free(alpha);
    free(beta);
    free(gamma);
    free(delta);
    *py = I;
    *pny = nI;
}

}
