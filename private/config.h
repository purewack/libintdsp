#pragma once

typedef int16_t spl_t;

#ifndef DEBUG
    #define LOGL(X) 
    #define LOGNL(X)
#else
    #ifndef LOGL
        #define LOGL(X) 
    #endif
    #ifndef LOGNL
        #define LOGNL(X) 
    #endif
#endif

#ifndef SPL_BITS
    #define SPL_BITS 16
#endif

#ifndef SPL_BITS_PROC
    #define SPL_BITS_PROC 32
#endif

#ifndef LUT_COUNT 
    #define LUT_COUNT 256
#endif

#define SIN_FUNC(X) 