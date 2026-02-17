#pragma once

#include <Arduino.h>

enum class AppScreen {
    BOOT,
    HOME,
    CONNECT_NETWORK,
    ONBOARDING,
    TAPPED
};

class AppState {
public:
    AppState();
    
    void setScreen(AppScreen screen);
    AppScreen getScreen() const { return current_screen; }
    
    bool shouldRevertToHome() const;
    void markTapped();
    
private:
    AppScreen current_screen;
    unsigned long tapped_time;
    static const unsigned long TAPPED_DISPLAY_DURATION = 3000;
};