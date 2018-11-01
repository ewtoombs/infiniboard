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


void tiling_usual(unsigned p, unsigned q, unsigned res, unsigned niter,
        complex<float> **py, unsigned *pny)
{
    // Some preliminary constants.
    float theta = TAU/p;
    float phi = TAU/q;

    float u = cos((theta + phi)/2);
    float v = cos((theta - phi)/2);
    float d = sqrt(u/v);

    float c = d*v/cos(phi/2);

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
void tiling_3q(unsigned q, unsigned res, unsigned niter,
        complex<float> **py, unsigned *pny)
{
    // Some preliminary constants.
    unsigned p = 3;
    float theta = TAU/p;
    float phi = TAU/q;

    float u = cos((theta + phi)/2);
    float v = cos((theta - phi)/2);
    float d = sqrt(u/v);

    float c = d*v/cos(phi/2);

    complex<float> A = exp(1if*theta);
    // D = B^-1.
    complex<float> D = exp(-1if*phi);

    complex<float> A00 = A - d*d,   A01 = -d*(A - 1.f),
                   A10 = d*(A - 1.f), A11 = 1.f - A*d*d;



    // pi, the primary edge.
    complex<float> *ls = linspacecf(0, -c*exp(1if*theta/2.f), res);

    complex<float> *pi;
    unsigned npi;
    line_strip_to_lines(ls, res, &pi, &npi);
    free(ls);

    for (unsigned u = 0; u < npi; u++)
        pi[u] = S(d, pi[u]);


    // Determine how much room is needed for each of alpha, zeta, gamma, and
    // eta.
    unsigned nalpha = npi;
    unsigned nzeta = npi;
    unsigned ngamma, neta;
    for (unsigned in = 0; in < niter; in++) {
        neta = 2*npi + (q - 6)*nalpha + nzeta;
        ngamma = neta + nalpha;
        nzeta = npi + neta;
        nalpha = npi + ngamma;
    }
    // Allocate.
    DEF_ARRAY(complex<float>, alpha, nalpha);
    DEF_ARRAY(complex<float>, zeta, nzeta);
    DEF_ARRAY(complex<float>, gamma, ngamma);
    DEF_ARRAY(complex<float>, eta, neta);



    // Begin tree building.
    nalpha = npi;
    nzeta = npi;
    COPY_ARRAY(pi, alpha, npi);
    COPY_ARRAY(pi, zeta, npi);
    for (unsigned in = 0; in < niter; in++) {
        // eta block
        unsigned neta = 2*npi + (q - 6)*nalpha + nzeta;

        complex<float> *pos = eta;

        COPY_ARRAY(pi, pos, npi); pos += npi;

        for (unsigned u = 0; u < npi; u++)
            pos[u] = D*pi[u];
        pos += npi;

        for (unsigned a = 0; a < q - 6; a++) {
            for (unsigned u = 0; u < nalpha; u++)
                pos[u] = pow(D, (float)(a + 2))*alpha[u];
            pos += nalpha;
        }

        for (unsigned u = 0; u < nzeta; u++)
            pos[u] = pow(D, (float)(q - 4))*zeta[u];


        // gamma block
        unsigned ngamma = neta + nalpha;

        pos = gamma;

        unsigned ncommon = neta - nzeta;
        COPY_ARRAY(eta, pos, ncommon); pos += ncommon;

        for (unsigned u = 0; u < nalpha; u++)
            pos[u] = pow(D, (float)(q - 4))*alpha[u];
        pos += nalpha;

        for (unsigned u = 0; u < nzeta; u++)
            pos[u] = pow(D, (float)(q - 3))*zeta[u];


        // alpha block
        nalpha = npi + ngamma;

        pos = alpha;

        COPY_ARRAY(pi, pos, npi); pos += npi;

        for (unsigned u = 0; u < ngamma; u++)
            pos[u] = (A00*gamma[u] + A01)/(A10*gamma[u] + A11);


        // zeta block
        nzeta = npi + neta;

        pos = zeta;

        COPY_ARRAY(pi, pos, npi); pos += npi;

        for (unsigned u = 0; u < neta; u++)
            pos[u] = (A00*eta[u] + A01)/(A10*eta[u] + A11);
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


    free(pi);
    free(alpha);
    free(zeta);
    free(gamma);
    free(eta);
    *py = I;
    *pny = nI;
}
static void tiling_p3(unsigned p, unsigned res, unsigned niter,
        complex<float> **py, unsigned *pny)
{
    // Some preliminary constants.
    unsigned q = 3;
    float theta = TAU/p;
    float phi = TAU/q;

    float u = cos((theta + phi)/2);
    float v = cos((theta - phi)/2);
    float d = sqrt(u/v);

    float c = d*v/cos(phi/2);

    complex<float> A = exp(1if*theta);
    // D = B^-1.
    complex<float> D = exp(-1if*phi);

    complex<float> D00 = D - d*d,      D01 = d*(D - 1.f),
                   D10 = -d*(D - 1.f), D11 = 1.f - D*d*d;



    // pi, the primary edge.
    complex<float> *ls = linspacecf(0, -c*exp(1if*theta/2.f), res);

    complex<float> *pi;
    unsigned npi;
    line_strip_to_lines(ls, res, &pi, &npi);
    free(ls);


    // Determine how much room is needed for each of beta, zeta, delta, and
    // epsilon.
    unsigned nbeta = npi;
    unsigned nzeta = npi;
    unsigned ndelta, nepsilon;
    for (unsigned in = 0; in < niter; in++) {
        ndelta = npi + nbeta;
        nepsilon = npi + nzeta;
        nzeta = 2*npi + (p - 6)*ndelta + nepsilon;
        nbeta = nzeta + ndelta;
    }
    // Allocate.
    DEF_ARRAY(complex<float>, beta, nbeta);
    DEF_ARRAY(complex<float>, zeta, nzeta);
    DEF_ARRAY(complex<float>, delta, ndelta);
    DEF_ARRAY(complex<float>, epsilon, nepsilon);



    // Begin tree building.
    nbeta = npi;
    nzeta = npi;
    COPY_ARRAY(pi, beta, npi);
    COPY_ARRAY(pi, zeta, npi);
    for (unsigned in = 0;; in++) {
        // delta block
        ndelta = npi + nbeta;

        complex<float> *pos = delta;

        COPY_ARRAY(pi, pos, npi); pos += npi;

        for (unsigned u = 0; u < nbeta; u++)
            pos[u] = (D00*beta[u] + D01)/(D10*beta[u] + D11);


        // epsilon block
        nepsilon = npi + nzeta;

        pos = epsilon;

        COPY_ARRAY(pi, pos, npi); pos += npi;

        for (unsigned u = 0; u < nzeta; u++)
            pos[u] = (D00*zeta[u] + D01)/(D10*zeta[u] + D11);


        if (in >= niter - 1)
            break;


        // zeta block
        nzeta = 2*npi + (p - 6)*ndelta + nepsilon;

        pos = zeta;

        COPY_ARRAY(pi, pos, npi); pos += npi;

        for (unsigned u = 0; u < npi; u++)
            pos[u] = A*pi[u];
        pos += npi;

        for (unsigned a = 0; a < p - 6; a++) {
            for (unsigned u = 0; u < ndelta; u++)
                pos[u] = pow(A, (float)(a + 2))*delta[u];
            pos += ndelta;
        }

        for (unsigned u = 0; u < nepsilon; u++)
            pos[u] = pow(A, (float)(p - 4))*epsilon[u];


        // beta block
        nbeta = nzeta + ndelta;

        pos = beta;

        unsigned ncommon = nzeta - nepsilon;
        COPY_ARRAY(zeta, pos, ncommon); pos += ncommon;

        for (unsigned u = 0; u < ndelta; u++)
            pos[u] = pow(A, (float)(p - 4))*delta[u];
        pos += ndelta;

        for (unsigned u = 0; u < nepsilon; u++)
            pos[u] = pow(A, (float)(p - 3))*epsilon[u];
    }


    // I block
    unsigned nalpha = 2*npi + (p - 4)*ndelta + nepsilon;
    unsigned nI = q*nalpha;
    DEF_ARRAY(complex<float>, I, nI);

    // Put one alpha block at the beginning of I.
    complex<float> *pos = I;

    COPY_ARRAY(pi, pos, npi); pos += npi;

    for (unsigned u = 0; u < npi; u++)
        pos[u] = A*pi[u];
    pos += npi;

    for (unsigned a = 0; a < p - 4; a++) {
        for (unsigned u = 0; u < ndelta; u++)
            pos[u] = pow(A, (float)(a + 2))*delta[u];
        pos += ndelta;
    }

    for (unsigned u = 0; u < nepsilon; u++)
        pos[u] = pow(A, (float)(p - 2))*epsilon[u];

    // Centre the alpha block on B.
    for (unsigned u = 0; u < nalpha; u++)
        I[u] = S(d, I[u]);

    pos = I + nalpha;
    // Copy to the rest of I.
    for (unsigned a = 1; a < q; a++) {
        for (unsigned u = 0; u < nalpha; u++)
            pos[u] = pow(D, (float)a)*I[u];
        pos += nalpha;
    }


    free(pi);
    free(beta);
    free(zeta);
    free(delta);
    free(epsilon);
    *py = I;
    *pny = nI;
}
void tiling(unsigned p, unsigned q, unsigned res, unsigned niter,
        complex<float> **py, unsigned *pny)
{
    assert(res >= 2);
    assert(2*p + 2*q < p*q);

    if (p == 3)
        tiling_3q(q, res, niter, py, pny);
    else if (q == 3)
        tiling_p3(p, res, niter, py, pny);
    else
        tiling_usual(p, q, res, niter, py, pny);
}

}
