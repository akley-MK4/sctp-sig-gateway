executable(
    'client',
    sources: [
        'main.c',
        'grpc_wrapper.cpp',  # C++ 适配层
    ],
    dependencies: [
        dependency('grpc++'),  # C++ gRPC 库
        dependency('grpc'),
        dependency('protobuf'),
        proto_dep
    ],
    link_args: ['-lstdc++'],  # 链接 C++ 标准库
    install: true
)