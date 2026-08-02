#pragma once

class ShadowRenderer
{
public:
    void initialize(SDL_Renderer* renderer);

    void draw(
        Renderer& renderer,
        const SDL_Rect& rect,
        int radius);

private:

    SDL_Texture* shadowTexture = nullptr;
};