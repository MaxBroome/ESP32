#pragma once

class DisplayContext;
class Arduino_GFX;

void drawConnectToNetworkScreen(DisplayContext& dc, Arduino_GFX* gfx, const char* apSsid);
