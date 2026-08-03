project('sctp-sig-gateway', 'c', 'cpp', version : '0.1')

# External dependencies
deps = [
    dependency('protobuf-c', required: true), 
    dependency('grpc++'), 
    dependency('grpc'), 
    dependency('protobuf'),
    dependency('nlohmann_json'),
    dependency('libevent'),
]

subdir('proto')

#internal dependencies
deps += [
    proto_dep,
]

srcs = []
subdir('src')

heads = include_directories('src', 'src/config', 'src/framework', 'src/grpc_mgr', 'src/task')

executable(
    'gw',
    sources: srcs,
    dependencies: deps,
    link_args: ['-lstdc++'], 
    include_directories: heads,
    install: true
)