#pragma once

#include <Arduino_GFX_Library.h>
#include "display_context.h"

class Display {
public:
    Display();
    ~Display();
    
    bool begin();
    void showBootScreen();
    void showHomeScreen();
    void showTappedMessage();
    void showSwipedMessage();
    
    Arduino_GFX* getGfx() { return gfx; }
    DisplayContext& getDc() { return dc; }
    
private:
    Arduino_DataBus *bus;
    Arduino_GFX *gfx;
    DisplayContext dc;
    
    void applyColorFix();
};