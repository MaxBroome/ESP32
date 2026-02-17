#pragma once

class DisplayContext;
class Arduino_GFX;

void drawBootScreen(DisplayContext& dc, Arduino_GFX* gfx, const char* status);
