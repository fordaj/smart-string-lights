#pragma once

class iServices
{
public:
  virtual void Init(void) = 0;
  virtual void Poll(void) = 0;

  void Disable(void)
  {
    isEnabled = false;
  }

  void Enable(void)
  {
    isEnabled = true;
  }

  bool IsEnabled(void)
  {
    return isEnabled;
  }

  bool IsInitialized(void)
  {
    return isInitialized;
  }

protected:
  bool isInitialized = false;
  bool isEnabled     = true;
};