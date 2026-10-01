#pragma once

#include <SDL2/SDL.h>

struct Theme
{
    SDL_Color windowBackground;//1

    SDL_Color panelBackground;//2
    SDL_Color sidebarBackground;//3

    SDL_Color headerBackground;//4
    SDL_Color toolbarBackground;//5
    SDL_Color statusBarBackground;//6

    SDL_Color buttonNormal;//7
    SDL_Color buttonHover;//8
    SDL_Color buttonPressed;//9

    SDL_Color border;//10

    SDL_Color text;//11
    SDL_Color lightText;//12
    SDL_Color darkText;//13

    SDL_Color lightSecondaryText;//14
    SDL_Color darkSecondaryText;//15

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