// vi:fo=qacj com=b\://

#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <complex>

#include <GL/glew.h>  // needed for shaders and shit.
#include <SDL2/SDL_opengl.h>

#include "helpers.hpp"

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

// Read a file to a string.
char *load(const char *fn)
{
    int fd = open(fn, O_RDONLY);
    assert(fd != -1);

    off_t size = lseek(fd, 0, SEEK_END);
    assert(size != -1);
    assert(lseek(fd, 0, SEEK_SET) != -1);

    size++;  // null terminator
    char *buf = (char *)malloc(size);

    char *p = buf;
    for (;;) {
        // File has gotten bigger since we started? Fuck that.
        assert(p - buf < size);
        ssize_t nread = read(fd, (char *)p, 0x10000);
        assert(nread != -1);
        if (nread == 0) {
            *p = '\0';
            break;
        }

        // Null character? Fuck that shit.
        void *nullbyte = memchr((char *)p, '\0', nread);
        assert(nullbyte == NULL);

        p += nread;
    }
    assert(close(fd) == 0);

    return buf;
}

// Do not call this function directly. Use the macro gl_assert(). Use it after
// an OpenGL API call to abort with a helpful error message if anything goes
// wrong. Example:
/// glCompileShader(shader); gl_assert();
void _gl_assert(const char *file, unsigned int line, const char *function)
{
    GLenum e = glGetError();
    if (e != GL_NO_ERROR) {
        fprintf(stderr, "GL assertion failed. file: %s\n"
                "function: %s\nline: %d     GL error: %s\n",
                file, function, line, gluErrorString(e));
        abort();
    }
}

// Compile the "type" shader named "filename" and attach it to
// "shader_program".
static void compile_shader(GLenum type, const char *filename,
        GLuint shader_program)
{
    GLuint shader = glCreateShader(type);
    char *source = load(filename);
    glShaderSource(shader, 1, &source, 0);
    glCompileShader(shader);
#ifndef NDEBUG
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint log_length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
        char *log = (char *)malloc(log_length);
        glGetShaderInfoLog(shader, log_length, NULL, log);
        fprintf(stderr, "Failed to compile %s:\n%s", filename, log);
        abort();
    }
#endif
    glAttachShader(shader_program, shader);
}

// Compile the vertex shader named "vertfile" and the fragment shader named
// "fragfile". Attach both to shader_program.
void compile_shaders(const char *vertfile, const char *fragfile,
        GLuint shader_program)
{
    compile_shader(GL_VERTEX_SHADER, vertfile, shader_program);
    compile_shader(GL_FRAGMENT_SHADER, fragfile, shader_program);
}
