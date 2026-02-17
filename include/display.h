#pragma once

#include <Arduino_GFX_Library.h>
#include "display_context.h"

class Display {
public:
    Display();
    ~Display();
    
    bool begin();
    void showBootScreen(const char* status = nullptr);
    void showHomeScreen();
    void showConnectToNetworkScreen(const char* apSsid);
    void showOnboardingScreen(const char* code);
    void updateOnboardingStatus(const char* msg);
    void showTappedMessage();
    void showSwipedMessage();
    
    Arduino_GFX* getGfx() { return gfx; }
    DisplayContext& getDc() { return dc; }
    
private:
    Arduino_DataBus *bus;
    Arduino_GFX *gfx;
    DisplayContext dc;
    
    void applyColorFix();

    Arduino_Canvas* status_canvas_ = nullptr;
};
