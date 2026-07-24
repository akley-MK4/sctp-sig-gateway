protoc = find_program('protoc')

# Find for gRPC C++plugins
grpc_cpp_plugin = find_program('grpc_cpp_plugin')

# Get the current generated directory
gen_dir = meson.current_build_dir()

proto_file = files('task.proto')

# Generate protobuf and gRPC code
proto_gen = custom_target(
    'generate-grpc-cpp',
    output: [
        'task.pb.cc',
        'task.pb.h',
        'task.grpc.pb.cc',
        'task.grpc.pb.h'
    ],
    command: [
        protoc,
        '--cpp_out=' + gen_dir,
        '--grpc_out=' + gen_dir,
        '--plugin=protoc-gen-grpc=' + grpc_cpp_plugin.full_path(),
        '-I' + meson.current_source_dir(),
        '-I' + join_paths(get_option('prefix'), get_option('includedir')),
        '@PLAINNAME@'
    ],
    input: proto_file,
    depend_files: proto_file,
)

# Declaration of Dependency
proto_dep = declare_dependency(
    sources: proto_gen,
    include_directories: include_directories('.')
)