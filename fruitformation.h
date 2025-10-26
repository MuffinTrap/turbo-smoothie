#pragma once

#include <mgdl.h>

#define FRUIT_COUNT 12
class FruitFormation
{
public:

    int collectedFruits[FRUIT_COUNT];
    FruitFormation();
    void Load();
    void Reset();

    bool Draw( V2f cursorPos, bool mouseClick);
    void Draw_Intro( );
    int GetCollectedAmount();

    float movedir = 1.0f;

};
