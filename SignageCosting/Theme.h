#pragma once

#include <SDL2/SDL.h>

struct Theme
{
    SDL_Color windowBackground;

    SDL_Color panelBackground;

    SDL_Color headerBackground;
    SDL_Color toolbarBackground;
    SDL_Color statusBarBackground;

    SDL_Color buttonNormal;
    SDL_Color buttonHover;
    SDL_Color buttonPressed;

    SDL_Color border;

    SDL_Color text;
    SDL_Color textSecondary;

    SDL_Color accent;

    int borderThickness;
    int padding;
    int margin;
};

extern Theme DefaultTheme;