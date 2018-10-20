# vim:ft=python
if int(ARGUMENTS.get('debug', 1)) == 1:
    env = Environment(CXX = 'g++', CXXFLAGS = '-ggdb')
else:
    env = Environment(CXX = 'g++', CXXFLAGS = '-s -O2 -DNDEBUG')

env.gl_libs = ['GL', 'GLU', 'GLEW']

helpers = env.Object('helpers.cpp')

env.Program('infiniboard', ['infiniboard.cpp', helpers],
        LIBS=['SDL2'] + env.gl_libs)
env.Program('load_test', ['load_test.cpp', helpers],
        LIBS=env.gl_libs)
