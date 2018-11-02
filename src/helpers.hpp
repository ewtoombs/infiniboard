// vi:fo=qacj com=b\://

#pragma once

#include <complex>
#include <cstring>

#include <GL/glew.h>  // needed for shaders and shit.
#include <GLFW/glfw3.h>

#define TAU 6.283185307179586

#include <iostream>
#define print(x) cout << x << '\n'
#define printe(x) cerr << x << '\n'

#define DEF_ARRAY(type, a, size) \
    type *a = (type *)malloc((size)*sizeof(type))
#define ASSN_ARRAY(type, size) \
    ((type *)malloc((size)*sizeof(type)))
#define ZERO_ARRAY(a, n) memset((a), 0, (n)*sizeof(*(a)))
#define COPY_ARRAY(a, b, n) memcpy((b), (a), (n)*sizeof(*(a)))

using namespace std;

float sq(float x);
float fact(unsigned n);
float modsq(complex<float> x);
float pown(float x, unsigned n);
float pown_l(float x, unsigned n);
char *load(const char *fn);

#ifdef	NDEBUG
# define gl_assert() ((void) (0))
#else
# define gl_assert() \
     (_gl_assert(__FILE__, __LINE__, __extension__ __PRETTY_FUNCTION__))
#endif
void _gl_assert(const char *file, unsigned int line, const char *function);

GLuint shader_program(const char *vertfile, const char *fragfile);

complex<float> *linspacecf(complex<float> a, complex<float> b, unsigned N);
void line_strip_to_lines(complex<float> *x, unsigned n,
        complex<float> **py, unsigned *pny);
