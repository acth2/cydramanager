project(
    'cydramanager',
    'c',
    version: '0.1.0',
    default_options: [
        'warning_level=2',
        'c_std=c11'
    ]
)

cc = meson.get_compiler('c')

add_project_arguments('-DWLR_USE_UNSTABLE', language: 'c')
add_project_arguments('-include', '/usr/lib/gcc/x86_64-pc-linux-gnu/14.2.0/include/stddef.h', language: 'c')

curl_dep = dependency('libcurl', version: '>=8.12.1', required: true)

executable(
    'cydramanager',
    [
        'src/main.c',
        'src/update/update_manager.c',
        'src/arguments/debug/debug.c',
        'src/install/install_manager.c',
        'src/remove/remove_manager.c',
        'src/utilities/utils.c'
    ],
    dependencies: [ curl_dep ],
    include_directories: include_directories('.'),
    install: true
)
