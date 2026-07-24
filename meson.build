project('grpc-demo', 'c', 'cpp', version : '0.1')

protobuf_c_dep = dependency('protobuf-c', required: true)

protoc = find_program('protoc')

subdir('proto')
subdir('src/client')
subdir('src/server')