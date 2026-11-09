#pragma once

#include "Renderer.h"
#include "Metrics.h"
#include "Theme.h"
#include "FontManager.h"
#include "Label.h"
#include <array>
#include <sstream>
#include <iomanip>

namespace NestingStyle
{
    inline constexpr SDL_Color ink{ 24, 32, 46, 255 };
    inline constexpr SDL_Color muted{ 112, 123, 142, 255 };
    inline const SDL_Color& border = DefaultTheme.border;
    inline constexpr SDL_Color white{ 255, 255, 255, 255 };
    inline constexpr SDL_Color green{ 35, 111, 64, 255 };
    inline constexpr std::array<SDL_Color, 8> colours{{
        { 76, 171, 118, 255 }, { 72, 137, 231, 255 },
        { 177, 139, 213, 255 }, { 220, 168, 81, 255 },
        { 73, 169, 180, 255 }, { 211, 122, 142, 255 },
        { 136, 158, 83, 255 }, { 129, 148, 178, 255 }
    }};

    inline std::string number(double value)
    {
        std::ostringstream stream;
        stream << std::setprecision(7) << value;
        return stream.str();
    }

    inline void text(Renderer& renderer, const std::string& value, int x, int y,
        LabelStyle style = LabelStyle::Small, SDL_Color colour = ink)
    {
        renderer.drawText(value, x, y, style, colour);
    }

    inline void card(Renderer& renderer, const SDL_Rect& rect)
    {
        renderer.fillRoundedRect(rect, white, Metrics::CardRadius);
        renderer.drawRoundedRect(rect, border, Metrics::CardRadius);
    }
}

