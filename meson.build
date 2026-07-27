project('sctp-sig-gateway', 'c', 'cpp', version : '0.1')

# External dependencies
deps = [
    dependency('protobuf-c', required: true), 
    dependency('grpc++'), 
    dependency('grpc'), 
    dependency('protobuf'),
]

subdir('proto')

#internal dependencies
deps += [
    proto_dep,
]

srcs = []
subdir('src')


executable(
    'gw',
    sources: src,
    dependencies: deps,
    link_args: ['-lstdc++'], 
    install: true
)