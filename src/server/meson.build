executable(
    'server',
    sources: [
        'main.c',
        'server.cpp'
    ],
    dependencies: [
        proto_dep,
        dependency('grpc++'),
        dependency('grpc'),
        dependency('protobuf')
    ],
    install: true
)