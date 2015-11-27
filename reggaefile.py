from reggae import *

build_type = user_vars.get('build', 'debug')

common_flags = '-std=c++14 -Wall -Wextra -Werror'
if build_type == 'debug':
    flags = common_flags + ' -g -O0 -fno-inline'
else:
    flags = common_flags + ' -O3 -flto'

if user_vars.get('asan', 'no') == 'yes':
    flags += ' -fsanitize=address'

if user_vars.get('profile', 'no') == 'yes':
    flags += ' -pg'

includes = ['src', 'cereal/src', 'gsl/include']

cereal = static_library('cereal.a',
                        flags=flags,
                        src_dirs=['cereal/src'],
                        includes=includes)
mqttlib = static_library('mqtt.a',
                         flags=flags,
                         src_dirs=['src', 'cereal/src'],
                         includes=includes)

main_objs = object_files(flags=flags,
                         includes=includes + ['src/boost'],
                         src_dirs=['src/boost'],
                         src_files=['main.cpp'])
mqtt = link(exe_name='mqtt',
            flags='-lboost_system -lpthread',
            dependencies=[main_objs, mqttlib, cereal])

ut_objs = object_files(flags=flags,
                       src_dirs=['tests'],
                       includes=includes)
ut = link(exe_name='ut',
          dependencies=[ut_objs, mqttlib, cereal])

# hybrid D/C++ binary
hyb_cpp_objs = object_files(flags=flags,
                            includes=includes,
                            src_dirs=['d/dinterface'])
hyb_d_objs = object_files(flags='',
                          includes=['d'],
                          src_dirs=['d/mqttd', 'd/cerealed'],
                          src_files=['d/impl.d'])
hybrid = link(exe_name='hybrid',
              flags='-L-lstdc++ -L-lboost_system -L-lpthread',
              dependencies=[hyb_cpp_objs, hyb_d_objs])

build = Build(mqtt, ut, hybrid)
