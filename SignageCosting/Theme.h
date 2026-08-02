#pragma once

#include <SDL2/SDL.h>

struct Theme
{
    SDL_Color windowBackground;

    SDL_Color panelBackground;
    SDL_Color sidebarBackground;

    SDL_Color headerBackground;
    SDL_Color toolbarBackground;
    SDL_Color statusBarBackground;

    SDL_Color buttonNormal;
    SDL_Color buttonHover;
    SDL_Color buttonPressed;

    SDL_Color border;

    SDL_Color text;
    SDL_Color lightText;
    SDL_Color darkText;

    SDL_Color lightSecondaryText;
    SDL_Color darkSecondaryText;

    // New
    SDL_Color successText;
    SDL_Color warningText;
    SDL_Color errorText;
    SDL_Color titleText;

    SDL_Color accent;
    SDL_Color sidebarSelected;
    SDL_Color navigationHover;

    int borderThickness;
    int padding;
    int margin;
};

extern Theme DefaultTheme;