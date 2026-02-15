#include "app_state.h"

AppState::AppState() 
    : current_screen(AppScreen::BOOT), tapped_time(0) {
}

void AppState::setScreen(AppScreen screen) {
    current_screen = screen;
}

void AppState::markTapped() {
    tapped_time = millis();
    current_screen = AppScreen::TAPPED;
}

bool AppState::shouldRevertToHome() const {
    if (current_screen != AppScreen::TAPPED) {
        return false;
    }
    return (millis() - tapped_time >= TAPPED_DISPLAY_DURATION);
}