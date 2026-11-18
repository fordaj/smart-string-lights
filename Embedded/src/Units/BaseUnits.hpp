#pragma once
#include <stdint.h>

namespace Units
{
  template<typename Derived, typename T = double>
  struct UnitBase
  {
    T value;

    constexpr UnitBase() = default;
    constexpr explicit UnitBase(T v):
        value(v)
    {
    }

    constexpr Derived operator+(Derived other) const
    {
      return Derived(value + other.value);
    }
    constexpr Derived operator-(Derived other) const
    {
      return Derived(value - other.value);
    }
    constexpr Derived operator*(T v) const
    {
      return Derived(value * v);
    }
    constexpr Derived operator/(T v) const
    {
      return Derived(value / v);
    }

    constexpr bool operator==(Derived other) const
    {
      return value == other.value;
    }
    constexpr bool operator!=(Derived other) const
    {
      return value != other.value;
    }
    constexpr bool operator<(Derived other) const
    {
      return value < other.value;
    }
    constexpr bool operator>(Derived other) const
    {
      return value > other.value;
    }
    constexpr bool operator<=(Derived other) const
    {
      return value <= other.value;
    }
    constexpr bool operator>=(Derived other) const
    {
      return value >= other.value;
    }
  };
}; // namespace Units