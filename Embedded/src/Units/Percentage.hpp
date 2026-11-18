#pragma once

#include "BaseUnits.hpp"

namespace Units
{

  struct Percentage : UnitBase<Percentage>
  {
    using UnitBase::UnitBase;

    static constexpr const char* Units()
    {
      return "%";
    }
  };

  constexpr Percentage operator""_percent(long double v)
  {
    return Percentage(static_cast<double>(v));
  }
  constexpr Percentage operator""_percent(unsigned long long v)
  {
    return Percentage(static_cast<double>(v));
  }

} // namespace Units
