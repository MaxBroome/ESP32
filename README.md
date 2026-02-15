# ScoreScrape ESP32-P4

IoT device with 1.47" touch display, WiFi, and Ethernet connectivity.

## Quick Start

### Build & Flash
```bash
pio run --target upload
```

### Upload Web Files
```bash
pio run --target uploadfs
```

### Factory Reset
Unplug device, hold BOOT button while plugging in power. Release after 5+ seconds.

## Drawing on the Display

MonkeyC-style API for easy text and graphics:

```cpp
#include "display_context.h"
#include "colors.h"

DisplayContext& dc = display.getDc();
Arduino_GFX* gfx = dc.getGfx();

// Batch drawing for instant, flicker-free updates
gfx->startWrite();

dc.setColor(COLOR_WHITE, COLOR_BLACK);
dc.clear();

dc.drawText(
    dc.getWidth() / 2,
    dc.getHeight() / 2,
    DisplayContext::FONT_LARGE,
    "Hello World",
    DisplayContext::TEXT_JUSTIFY_CENTER | DisplayContext::TEXT_JUSTIFY_VCENTER
);

gfx->endWrite();  // Everything appears instantly!
```

See [Quick Reference](docs/QUICK_REFERENCE.md) for common patterns.

## Performance

Eliminate flicker and speed up screen transitions by batching drawing operations:

```cpp
gfx->startWrite();   // Start batching
// ... all drawing operations ...
gfx->endWrite();     // Display everything at once
```

See [Performance Guide](docs/PERFORMANCE.md) for details.

## Documentation

- [Quick Reference](docs/QUICK_REFERENCE.md) - Common patterns and examples
- [Performance Guide](docs/PERFORMANCE.md) - Eliminate flicker and speed up updates
- [MonkeyC-Style API](docs/MONKEYC_STYLE_API.md) - Complete API reference
- [Font Usage](docs/FONT_USAGE.md) - Typography details
- [Typography System](docs/TYPOGRAPHY.md) - Design principles

### Regenerating Fonts
```bash
bash scripts/generate_fonts.sh
```

Requires `freetype`: `brew install freetype`

## Project Structure

- `src/` - Main application code
- `include/` - Header files
- `data/` - Web interface files (captive portal)
- `assets/` - Images, fonts, and resources
- `docs/` - Documentation
- `scripts/` - Build and utility scripts