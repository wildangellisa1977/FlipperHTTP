#pragma once
#include "boards.h"
class WiFiDeauth
{
public:
    bool start(const char *ssid);
    void stop();
    void update();
};
