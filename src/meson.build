# platform detection

if host_machine.system() == 'windows'
    add_project_arguments(['-D_WIN_'], language: 'cpp') 
endif
if host_machine.system() == 'linux'
    add_project_arguments(['-D_LINUX_'], language: 'cpp') 
endif
if host_machine.system() == 'darwin'
    add_project_arguments(['-D_MAC_'], language: 'cpp') 
endif

if host_machine.system() == 'windows'
    dl_dep = declare_dependency() # dummy
else
    dl_dep = declare_dependency(link_args: ['-ldl'])
endif

srcs = ['GS5_Intf.cpp', 'GS5_Ext.cpp', 'GS5.cpp', 'GS5_Inspector.cpp']

lib_softwareshield = static_library('softwareshield-sdk', srcs, dependencies: [dl_dep])

softwareshield_dep = declare_dependency(include_directories: '.', link_with: lib_softwareshield)