#include "font_manager.h"

// Include generated font files
#include "../assets/fonts/Inter_Regular_12pt.h"
#include "../assets/fonts/Inter_Regular_16pt.h"
#include "../assets/fonts/Inter_Semibold_20pt.h"
#include "../assets/fonts/Inter_Semibold_24pt.h"
#include "../assets/fonts/Inter_Bold_32pt.h"
#include "../assets/fonts/JetBrainsMono_Regular_12pt.h"
#include "../assets/fonts/JetBrainsMono_Regular_16pt.h"

const GFXfont* FontManager::get(FontManager::Size size) {
    switch (size) {
        case FontManager::Size::SMALL:   return &Inter_Regular12pt7b;
        case FontManager::Size::BODY:    return &Inter_Regular16pt7b;
        case FontManager::Size::HEADING: return &Inter_SemiBold20pt7b;
        case FontManager::Size::TITLE:   return &Inter_SemiBold24pt7b;
        case FontManager::Size::HERO:    return &Inter_Bold32pt7b;
        default:                         return &Inter_Regular16pt7b;
    }
}