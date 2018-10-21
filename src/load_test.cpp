// vi:fo=qacj com=b\://

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "helpers.hpp"

using namespace std;

int main(int argc, const char **argv)
{
    puts(load("load_test.cpp"));
    puts(load("data/evil_test_file"));
    return 0;
}
