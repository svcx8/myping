#ifndef LOGGER_HEADER
#define LOGGER_HEADER

#include <cstdio>
#include <iostream>
using std::cout;
using std::endl;

#define logger(str, ...) \
    std::printf(str "\n", __VA_ARGS__);

#endif