#pragma once

#include <mgdl.h>

#define FRUIT_COUNT 12
class FruitFormation
{
public:

    int collectedFruits[FRUIT_COUNT];
    FruitFormation();
    void Load();

    bool Draw( V2f cursorPos, bool mouseClick);
    int GetCollectedAmount();

};
