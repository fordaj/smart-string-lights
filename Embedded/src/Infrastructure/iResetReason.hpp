#pragma once

class IResetReason
{
public:
  virtual ~IResetReason()    = default;
  virtual void print() const = 0;
};
