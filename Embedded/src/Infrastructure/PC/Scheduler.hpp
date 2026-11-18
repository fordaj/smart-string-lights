#pragma once
#include <chrono>
#include <thread>
#include "iScheduler.hpp"

namespace
{

  using clock      = std::chrono::steady_clock;
  const auto start = clock::now();

} // namespace

namespace Platform
{

  Tick Now()
  {
    auto now = clock::now();
    auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
    return static_cast<Tick>(ms.count());
  }

  void Idle()
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

} // namespace Platform