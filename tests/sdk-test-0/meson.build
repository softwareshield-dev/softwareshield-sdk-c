srcs = ['main.cpp', 'action-test.cpp',
    'lm-hard-date-test.cpp', 
    'lm-period-test.cpp']

executable('sdk-test-0', srcs, 
    dependencies: [
        catch2_dep, catch2_ex_dep, 
        lic_data_dep, 
        softwareshield_dep,
        dependency('threads')
    ])