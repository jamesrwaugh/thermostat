rust_dep = dependency('cargo', required: true)

rust_lib = static_library('rust_code', language: 'rust', install: false, sources: 'Cargo.toml')

rust_dep = declare_dependency(link_with: rust_lib)