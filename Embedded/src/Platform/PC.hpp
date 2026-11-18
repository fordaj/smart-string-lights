#pragma once
#include "Logger.hpp"
#include <iostream>
#include <cstdarg>
#include <cstdio>

class PCLogger : public Logger
{
public:
  void logf(const char* fmt, ...) override
  {
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    std::vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    std::cout << buffer;
  }
};

PCLogger logger;