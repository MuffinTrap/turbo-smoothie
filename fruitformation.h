#pragma once

#include <mgdl.h>

#define FRUIT_COUNT 12
class FruitFormation
{
public:

    int collectedFruits[16];
    FruitFormation();
    void Load();

    void Draw( V2f cursorPos, bool mouseClick);
    int GetCollectedAmount();

};
