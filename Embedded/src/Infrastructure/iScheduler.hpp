#pragma once
#include <stddef.h>
#include <stdint.h>

using TaskFunc = void (*)();

struct Task
{
  TaskFunc run;
  uint32_t period_ms;
  uint32_t next_run;
};

namespace Platform
{

  using Tick = uint32_t;

  Tick Now();  // current time in ms
  void Idle(); // yield / sleep hint

} // namespace Platform

class Scheduler
{
public:
  Scheduler(Task* tasks, size_t count):
      tasks_(tasks),
      count_(count)
  {
  }

  void Init()
  {
    uint32_t now = Platform::Now();
    for (size_t i = 0; i < count_; ++i)
    {
      tasks_[i].next_run = now + tasks_[i].period_ms;
    }
  }

  void RunOnce()
  {
    uint32_t now = Platform::Now();

    for (size_t i = 0; i < count_; ++i)
    {
      if ((int32_t)(now - tasks_[i].next_run) >= 0)
      {
        tasks_[i].run();
        tasks_[i].next_run += tasks_[i].period_ms;
      }
    }

    Platform::Idle();
  }

private:
  Task* tasks_;
  size_t count_;
};