# vim:ft=python
if int(ARGUMENTS.get('debug', 0)) == 1:
    env = Environment(CXX = 'g++', CXXFLAGS = '-ggdb -DDEBUG')
else:
    env = Environment(CXX = 'g++', CXXFLAGS = '-s -O2')

helpers = env.Object('helpers.cpp')

env.Program('infiniboard', [env.Object('infiniboard.cpp'), helpers],
        LIBS=['SDL2', 'GLU', 'GL', 'GLEW'])
env.Program('load_test', [env.Object('load_test.cpp'), helpers])
