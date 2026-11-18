#pragma once

#include "BaseUnits.hpp"

namespace Units
{

  struct Frequency : UnitBase<Frequency>
  {
    enum class Unit
    {
      Hertz,
      Kilohertz,
      Millihertz
    };
    using UnitBase::UnitBase; // value stored as Hz

    constexpr double Hertz() const
    {
      return value;
    }
    constexpr double Kilohertz() const
    {
      return value / 1000.0;
    }
    constexpr double Millihertz() const
    {
      return value * 1000.0;
    }

    constexpr double PeriodSeconds() const
    {
      return 1.0 / value;
    }

    static constexpr const char* Units(Unit u = Unit::Hertz)
    {
      return (u == Unit::Kilohertz) ? "kHz" : (u == Unit::Millihertz) ? "mHz" : "Hz";
    }
  };

  // User-defined literals

  constexpr Frequency operator""_hertz(long double v)
  {
    return Frequency(static_cast<double>(v));
  }

  constexpr Frequency operator""_hertz(unsigned long long v)
  {
    return Frequency(static_cast<double>(v));
  }

  constexpr Frequency operator""_k_hertz(long double v)
  {
    return Frequency(static_cast<double>(v) * 1000.0);
  }

  constexpr Frequency operator""_k_hertz(unsigned long long v)
  {
    return Frequency(static_cast<double>(v) * 1000.0);
  }

  constexpr Frequency operator""_m_hertz(long double v)
  {
    return Frequency(static_cast<double>(v) / 1000.0);
  }

  constexpr Frequency operator""_m_hertz(unsigned long long v)
  {
    return Frequency(static_cast<double>(v) / 1000.0);
  }

} // namespace Units
