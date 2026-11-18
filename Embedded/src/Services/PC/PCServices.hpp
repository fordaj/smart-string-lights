#pragma once
#include "iServices.hpp"
#include "Configuration.hpp"
#include "PCWebServer.hpp"

class PCServices : public iServices
{
public:
  void Init(void) override
  {
    if (Configuration.isWebServerEnabled)
    {
      webServer.Init();
    }
    isInitialized = true;
  }

  void Poll(void) override
  {
    if (Configuration.isWebServerEnabled)
    {
      if (!webServer.IsRunning())
      {
        webServer.Start();
      }
    }
  }

private:
  PCWebServer webServer = PCWebServer();
};
