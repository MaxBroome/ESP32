#ifndef FONT_MANAGER_H
#define FONT_MANAGER_H

#include <Arduino_GFX_Library.h>

// Forward declarations of generated fonts
extern const GFXfont Inter_Regular12pt7b;
extern const GFXfont Inter_Regular16pt7b;
extern const GFXfont Inter_SemiBold20pt7b;
extern const GFXfont Inter_SemiBold24pt7b;
extern const GFXfont Inter_Bold32pt7b;
extern const GFXfont JetBrainsMono_Regular12pt7b;
extern const GFXfont JetBrainsMono_Regular16pt7b;

/**
 * FontManager - Centralized font management for the display
 * 
 * Provides easy access to the typography system with semantic naming.
 * All fonts use Inter (sans-serif) for UI and JetBrains Mono for data.
 */
class FontManager {
public:
    // Font size categories
    enum class Size {
        SMALL,      // 12pt - Captions, metadata
        BODY,       // 16pt - Body text, labels
        HEADING,    // 20pt - Secondary headlines
        TITLE,      // 24pt - Primary headlines
        HERO        // 32pt - Hero text, splash screens
    };

    // Font weight
    enum class Weight {
        REGULAR,
        SEMIBOLD,
        BOLD
    };

    // Font family
    enum class Family {
        SANS,       // Inter
        MONO        // JetBrains Mono
    };

    /**
     * Get font by semantic size
     * Uses Inter Semibold for headings, Regular for body
     */
    static const GFXfont* get(Size size);

    // Convenience methods for common use cases
    static const GFXfont* body()    { return &Inter_Regular16pt7b; }
    static const GFXfont* heading() { return &Inter_SemiBold24pt7b; }
    static const GFXfont* title()   { return &Inter_SemiBold20pt7b; }
    static const GFXfont* display() { return &Inter_Bold32pt7b; }
    static const GFXfont* mono()    { return &JetBrainsMono_Regular16pt7b; }
    static const GFXfont* monoSmall() { return &JetBrainsMono_Regular12pt7b; }
    static const GFXfont* small()   { return &Inter_Regular12pt7b; }
};

#endif // FONT_MANAGER_H