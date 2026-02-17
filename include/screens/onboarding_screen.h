#pragma once

class DisplayContext;
class Arduino_GFX;
class Arduino_Canvas;

void drawOnboardingScreen(DisplayContext& dc, Arduino_GFX* gfx, const char* code);

// Updates only the bottom status strip (uses buffered canvas)
void drawOnboardingStatus(Arduino_Canvas* canvas, const char* msg);
