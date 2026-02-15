#!/bin/bash
set -e

echo "=== Font Generation Script ==="
echo ""

# Save project directory
PROJECT_DIR="$(pwd)"

# Create directories
mkdir -p assets/fonts
mkdir -p /tmp/font_generation
cd /tmp/font_generation

# Download and build fontconvert
echo "1. Cloning Adafruit-GFX-Library..."
if [ ! -d "Adafruit-GFX-Library" ]; then
    git clone --depth 1 https://github.com/adafruit/Adafruit-GFX-Library.git
fi

echo "2. Building fontconvert..."
cd Adafruit-GFX-Library/fontconvert

# Detect freetype path on macOS
if [ -d "/opt/homebrew/include/freetype2" ]; then
    FT_INCLUDE="/opt/homebrew/include/freetype2"
    FT_LIB="/opt/homebrew/lib"
elif [ -d "/usr/local/include/freetype2" ]; then
    FT_INCLUDE="/usr/local/include/freetype2"
    FT_LIB="/usr/local/lib"
else
    FT_INCLUDE="/usr/include/freetype2"
    FT_LIB="/usr/lib"
fi

echo "   Using freetype from: $FT_INCLUDE"
gcc -Wall -I"$FT_INCLUDE" -L"$FT_LIB" fontconvert.c -lfreetype -o fontconvert

# Download fonts
echo "3. Downloading Inter font..."
cd /tmp/font_generation
if [ ! -f "Inter.zip" ]; then
    curl -L "https://github.com/rsms/inter/releases/download/v4.0/Inter-4.0.zip" -o Inter.zip
    unzip -q Inter.zip -d Inter
fi

echo "4. Downloading JetBrains Mono font..."
if [ ! -f "JetBrainsMono.zip" ]; then
    curl -L "https://github.com/JetBrains/JetBrainsMono/releases/download/v2.304/JetBrainsMono-2.304.zip" -o JetBrainsMono.zip
    unzip -q JetBrainsMono.zip -d JetBrainsMono
fi

# Convert fonts
echo "5. Converting fonts to GFX format..."
FONTCONVERT="./Adafruit-GFX-Library/fontconvert/fontconvert"

# Inter fonts (from extras/ttf)
$FONTCONVERT Inter/extras/ttf/Inter-Regular.ttf 12 32 126 > Inter_Regular_12pt.h
$FONTCONVERT Inter/extras/ttf/Inter-Regular.ttf 16 32 126 > Inter_Regular_16pt.h
$FONTCONVERT Inter/extras/ttf/Inter-SemiBold.ttf 20 32 126 > Inter_Semibold_20pt.h
$FONTCONVERT Inter/extras/ttf/Inter-SemiBold.ttf 24 32 126 > Inter_Semibold_24pt.h
$FONTCONVERT Inter/extras/ttf/Inter-Bold.ttf 32 32 126 > Inter_Bold_32pt.h

# JetBrains Mono fonts
$FONTCONVERT JetBrainsMono/fonts/ttf/JetBrainsMono-Regular.ttf 12 32 126 > JetBrainsMono_Regular_12pt.h
$FONTCONVERT JetBrainsMono/fonts/ttf/JetBrainsMono-Regular.ttf 16 32 126 > JetBrainsMono_Regular_16pt.h

echo "6. Moving font files to project..."
mv *.h "$PROJECT_DIR/assets/fonts/"

echo ""
echo "✓ Font generation complete!"
echo "✓ Font files saved to: assets/fonts/"
echo ""
echo "Generated fonts:"
ls -1 "$PROJECT_DIR/assets/fonts/"*.h

# Cleanup
cd /tmp
rm -rf /tmp/font_generation

echo ""
echo "Fonts are ready to use via FontManager class"
