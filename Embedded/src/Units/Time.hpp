#pragma once
#include "BaseUnits.hpp"
#include "Frequency.hpp"

namespace Units
{

  struct Time : UnitBase<Time>
  {
    enum class Unit
    {
      Seconds,
      Milliseconds,
      Microseconds,
      Minutes,
      Hours,
      // Days
    };

    using UnitBase::UnitBase;

    constexpr double Seconds() const
    {
      return value;
    }
    constexpr double Milliseconds() const
    {
      return value * 1000.0;
    }
    constexpr double Microseconds() const
    {
      return value * 1000000.0;
    }
    constexpr double Minutes() const
    {
      return value / 60.0;
    }
    constexpr double Hours() const
    {
      return value / 3600.0;
    }

    static Time Microseconds(const long double _value)
    {
      return Time(_value / 1e6);
    }

    // constexpr double Hertz() const {
    //     return (value > 0.0) ? 1.0 / value : 0.0;
    // }

    static const char* Units(Unit u = Unit::Seconds)
    {
      switch (u)
      {
        case Unit::Milliseconds:
          return "ms";
        case Unit::Microseconds:
          return "μs";
        case Unit::Minutes:
          return "min";
        case Unit::Hours:
          return "h";
        // case Unit::Days:      return "d";
        default:
          return "s";
      }
    }
  };

  constexpr Time operator/(double scalar, const Frequency& freq)
  {
    return Time(scalar / freq.value);
  }
  constexpr Time operator/(float scalar, const Frequency& freq)
  {
    return Time(static_cast<double>(scalar) / freq.value);
  }
  constexpr Time operator/(int scalar, const Frequency& freq)
  {
    return static_cast<double>(scalar) / freq;
  }

  constexpr Time operator""_seconds(long double v)
  {
    return Time(static_cast<double>(v));
  }
  constexpr Time operator""_seconds(unsigned long long v)
  {
    return Time(static_cast<double>(v));
  }
  constexpr Time operator""_m_seconds(long double v)
  {
    return Time(static_cast<double>(v) * 0.001);
  }
  constexpr Time operator""_m_seconds(unsigned long long v)
  {
    return Time(static_cast<double>(v) * 0.001);
  }
  constexpr Time operator""_u_seconds(long double v)
  {
    return Time(static_cast<double>(v) * 1e-6);
  }
  constexpr Time operator""_u_seconds(unsigned long long v)
  {
    return Time(static_cast<double>(v) * 1e-6);
  }
  constexpr Time operator""_minutes(long double v)
  {
    return Time(static_cast<double>(v) * 60.0);
  }
  constexpr Time operator""_minutes(unsigned long long v)
  {
    return Time(static_cast<double>(v) * 60.0);
  }
  constexpr Time operator""_hours(long double v)
  {
    return Time(static_cast<double>(v) * 3600.0);
  }
  constexpr Time operator""_hours(unsigned long long v)
  {
    return Time(static_cast<double>(v) * 3600.0);
  }

} // namespace Units