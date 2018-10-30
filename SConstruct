# vi:ft=python fo=qacj
if int(ARGUMENTS.get('debug', 1)) == 1:
    env = Environment(CXX = 'g++', CXXFLAGS = '-ggdb')
else:
    env = Environment(CXX = 'g++', CXXFLAGS = '-s -O2 -DNDEBUG')
env.libs = ['glfw', 'GL', 'GLU', 'GLEW', 'rt']
Export('env')

SConscript(['src/SConscript'], variant_dir='build', duplicate=0)
