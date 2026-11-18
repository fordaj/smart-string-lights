#pragma once
#ifdef PLATFORM_ARDUINO
  #include "ESP32Services.hpp"
using PlatformServices = ESP32Services;
#elif defined(PLATFORM_PC)
  #include "PCServices.hpp"
using PlatformServices = PCServices;
#endif

class ServicesSingleton
{
public:
  static ServicesSingleton& GetServicesSingleton(void)
  {
    static ServicesSingleton singleton;
    return singleton;
  }

private:
  ServicesSingleton(void)                                = default;
  ~ServicesSingleton(void)                               = default;
  ServicesSingleton(const ServicesSingleton&)            = delete;
  ServicesSingleton& operator=(const ServicesSingleton&) = delete;

public:
  void Init(void)
  {
    platformServices.Init();
  }

  void Poll(void)
  {
    platformServices.Poll();
  }

  iServices& GetPlatformServices(void)
  {
    return platformServices;
  }

private:
  PlatformServices platformServices = PlatformServices();
};

static ServicesSingleton& Services = ServicesSingleton::GetServicesSingleton();
