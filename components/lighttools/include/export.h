#pragma once

#ifndef LIGHTTOOLS_BUILD
    // the library is being consumed by another project
    // mark all APIs as being imported
    #ifdef _WIN32
        #define LIGHTTOOLS_API __declspec(dllimport)
    #else
        #define LIGHTTOOLS_API
    #endif
#else
    // the library is being built
    // mark all APIs as being exported
    #ifdef _WIN32
        #define LIGHTTOOLS_API __declspec(dllexport)
    #else
        #define LIGHTTOOLS_API
    #endif
#endif
