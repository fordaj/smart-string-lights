#pragma once
#include <cstdint>

class Logger
{
public:
  virtual ~Logger()                       = default;
  virtual void logf(const char* fmt, ...) = 0;
};