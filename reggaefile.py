from reggae import *

build_type = user_vars.get('build', 'debug')

common_flags = '-std=c++14'
if build_type == 'debug':
    flags = common_flags + ' -g -O0 -fno-inline'
else:
    flags = common_flags + ' -O3 -flto'

if user_vars.get('asan', 'no') == 'yes':
    flags += ' -fsanitize=address'

includes = ['src', 'cereal/src', 'gsl/include']
#cereal = static_library('cereal.a', src_dirs=['cereal/src'])
mqttlib = static_library('mqtt.a',
                         flags=flags,
                         src_dirs=['src', 'cereal/src'],
                         includes=includes)
mqtt = link(exe_name='mqtt', dependencies=mqttlib)

# if build_type == 'debug':
#     ut_objs = object_files(flags=flags,
#                            src_dirs=['tests'],
#                            includes=includes)
#     ut = link(exe_name='ut',
#               dependencies=ut_objs)
#     build = Build(mqtt, ut)
# else:
build = Build(mqtt)
