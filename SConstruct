

CXX_FLAGS = ''

try:
 from SConfig import *
except:
 CXX_FLAGS += '-g -O3 -Wall -c -fmessage-length=0'

VariantDir('build', 'lib')
env = Environment()

env.Replace(CXX='clang++')

env.Append(CXXFLAGS='-std=c++14')
if CXX_FLAGS:
	env.Append(CXXFLAGS=CXX_FLAGS)
env.Append(CXXFLAGS='-march=native')
env.Append(CPPPATH=['/usr/local/include'])
env.Append(LIBPATH=['/usr/local/lib'])
env.Append(LIBS=['glog', 'gflags', 'ssl', 'crypto', 'folly', 'wangle', 'zookeeper_mt', 'log4cxx', 'rocksdb', 'jemalloc', 'boost_program_options-mt'])

env.Program('bookie', Glob('src/*.cpp'))


