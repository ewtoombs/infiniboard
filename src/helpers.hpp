// vi:fo=qacj com=b\://

#pragma once

#include <complex>

#include <GL/glew.h>  // needed for shaders and shit.
#include <SDL2/SDL_opengl.h>

using namespace std;

double sq(double x);
double fact(unsigned n);
double modsq(complex<double> x);
double pown(double x, unsigned n);
double pown_l(double x, unsigned n);
char *load(const char *fn);


#ifdef	NDEBUG
# define gl_assert() ((void) (0))
#else
# define gl_assert() \
     (_gl_assert(__FILE__, __LINE__, __extension__ __PRETTY_FUNCTION__))
#endif
void _gl_assert(const char *file, unsigned int line, const char *function);

void compile_shaders(const char *vertfile, const char *fragfile,
        GLuint shader_program);
