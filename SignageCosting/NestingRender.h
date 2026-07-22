#pragma once

#include "NestingCore.h"

class Renderer;

class NestingRenderer
{
public:

    static void drawSheet(
        Renderer& renderer,
        const Sheet& sheet,
        int screenX,
        int screenY,
        float scale
    );

};
