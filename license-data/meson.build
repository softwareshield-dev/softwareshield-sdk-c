# license data of test projects

# include all bin2c generated license data
srcs = []

subdir('sdk-test-0')

lib_lic_data = static_library('license-data', srcs)
lic_data_dep = declare_dependency(include_directories: '.', link_with: lib_lic_data)

