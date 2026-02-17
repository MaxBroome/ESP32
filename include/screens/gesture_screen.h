#pragma once

class DisplayContext;
class Arduino_GFX;

void drawGestureMessage(DisplayContext& dc, Arduino_GFX* gfx, const char* message);
