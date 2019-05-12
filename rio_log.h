#ifndef __RIO_LOG_H__
#define __RIO_LOG_H__

#ifdef DEBUG
    #define LOG(...) printf (__VA_ARGS__)
#else
    #define LOG(...) 
#endif

#endif