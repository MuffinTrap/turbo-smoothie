#include "fruitformation.h"

#include "sprite_collision.h"

static const short spriteHeight = 64;

static Sprite* fruitSprites = NULL;
FruitFormation::FruitFormation()
{
    fruitSprites = NULL;
}


void FruitFormation::Load()
{
    fruitSprites = mgdl_LoadSprite("assets/fruits.png", 16, 16);
    for(int i = 0; i < 16; i++)
    {
        collectedFruits[i] = 0;

    }
}

int FruitFormation::GetCollectedAmount()
{

    int collected = 0;
    for (int i = 0; i < FRUIT_COUNT; i++)
    {
        if (collectedFruits[i] == 1)
        {
            collected++;
        }
    }
    return collected;
}


void FruitFormation::Draw(V2f cursorPos, bool mouseClick)
{
    mgdl_glSetAlphaTest(true);
    mgdl_glSetTransparency(true);
    int mx = mgdl_GetScreenWidth()/2;
    int my = mgdl_GetScreenHeight()/2;
        float time = mgdl_GetElapsedSeconds();
        int size = spriteHeight;
    for (int i = 0; i < FRUIT_COUNT; i++)
    {
        float angle = (M_PI * 2/(float)FRUIT_COUNT) * i;
        float radius = size * 1.5 + sin(time) * size*0.5f;
        float x = mx + cos(time + angle) * radius;
        float y = my + sin(time + angle) * radius;
        if (collectedFruits[i] == 0)
        {
            if (Sprite_CheckCollisionPoint(fruitSprites, x, y, cursorPos , size, Centered, Centered))
            {
                if (mouseClick)
                {
                    collectedFruits[i] = 1;
                }
            }
            Sprite_Draw2D(fruitSprites, i, x, y, size, Centered, Centered, Color_GetDefaultColor(Color_White));
        }
    }
    mgdl_glSetAlphaTest(false);
    mgdl_glSetTransparency(false);
}
