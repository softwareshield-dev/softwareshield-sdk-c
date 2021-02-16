# softwareshield-sdk-c

SoftwareShield SDK for C/C++

This is c/c++ glue code to integrate any c/c++ projects with [SoftwareShield SDK](https://github.com/softwareshield-dev/softwareshield-sdk-main.git) binary. 

* Standard C++11 compatible compiler is required to build the source; 
* Standard C++17 compatible compiler is required to build the examples and tests; 
* [meson](https://mesonbuild.com/) is used for cross-platform builds; 

## Content

```
├───doc/: programming guide;
├───examples/: examples for the sdk usage;
├───src/: glue-code needed to integrate SDK with your own c/c++ source code;
└───tests/: testcases for src;
```

## Version

The SDK-C is a seperate sub-project to help C++ developers implement their own license logic in applications.

SDK-C has a different version from SDK-main because SDK-C supports both SoftwareShield SDK v5.x and new v6.x binaries, api backward compatiblity with all SoftwareShield SDK binary versions is our top priority.

## Usage

Copy sources in _src_ folder to a folder in your app project, and add them as part of your source code.

## Where should I deploy the SDK binary

When debugging your licensed app, the SDK binary (on Winodws: gsCore.dll, on Mac: libgsCore.dylib, on Linux: libgsCore.so) must be put in a folder that the SDK-C layer can find it and bind all apis, there are three options:

* copy gsCore to one of the folders specified by your system PATH, or whatever system path that the operating system will locate dynamic library by default.
* copy gsCore side by side with your app executable;
* setup environment variable **GS_SDK_BIN** to the folder where the gsCore is deployed.
```
${GS_SDK_BIN}
    ├── gsCore.dll
```

  If your app sources compiles in both 32bit and 64 bit, the gsCore can be deployed as following layout:

```
${GS_SDK_BIN}
    ├───win32/: gsCore.dll
    ├───win64/: gsCore.dll
```

## Should I deploy the SDK binary as part of my app release?

```c++
if(your_app_use_IRONWRAPPER){
    //NO, the SDK-binary has been wrapped inside your app release by Ironwrapper,
    //the internal version is always been used at runtime.
} else {
    //YES, you need to copy SDK binary to a path loadable by your app.
}
```

