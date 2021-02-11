srcs = ['GS5_Intf.cpp', 'GS5_Ext.cpp', 'GS5.cpp']

lib_softwareshield = static_library('softwareshield-sdk', srcs)

softwareshield_dep = declare_dependency(include_directories: '.', link_with: lib_softwareshield)