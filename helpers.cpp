#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
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

// Read a file to a string. Stop before first null character, because fuck that
// shit.
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
        assert(p - buf < size);
        ssize_t nread = read(fd, (char *)p, 0x10000);
        assert(nread != -1);
        if (nread == 0) {
            *p = '\0';
            break;
        }

        char *nullbyte = (char *)memchr((char *)p, '\0', nread);
        if (nullbyte != NULL) {
            buf = (char *)realloc((char *)buf, nullbyte - buf);
            break;
        }

        p += nread;
    }
    assert(close(fd) == 0);

    return buf;
}
