#include "fruitformation.h"

#include "sprite_collision.h"

static const short spriteHeight = 32;

static Sprite* fruitSprites = NULL;
FruitFormation::FruitFormation()
{
    fruitSprites = NULL;
}


void FruitFormation::Load()
{
    fruitSprites = mgdl_LoadSprite("assets/fruits.png", 16, 16);
    for(int i = 0; i < FRUIT_COUNT; i++)
    {
        collectedFruits[i] = 0;

    }
}

void FruitFormation::Reset()
{
    for (int i = 0; i < FRUIT_COUNT; i++)
    {
        collectedFruits[i] = 0;
    }
}

int FruitFormation::GetCollectedAmount()
{

    int collected = 0;
    for (int i = 0; i < FRUIT_COUNT; i++)
    {
        collected +=  collectedFruits[i];
    }
    return collected;
}

void FruitFormation::Draw_Intro()
{
    mgdl_glSetAlphaTest(true);
    mgdl_glSetTransparency(true);
    int mx = mgdl_GetScreenWidth()/2;
    int my = mgdl_GetScreenHeight()/2;
    float time = mgdl_GetElapsedSeconds();
    int size = spriteHeight * 2;

    Sprite_BeginDrawBatch(fruitSprites);
    for (int i = 0; i < FRUIT_COUNT; i++)
    {
        float angle = (M_PI * 2/(float)FRUIT_COUNT) * i;
        float radius = size * 3.5 + sin(time) * size*0.5f;
        float x = mx + cos(time + angle) * radius;
        float y = my + sin(time + angle) * radius;
        Sprite_Draw2D(fruitSprites, i, x, y, size, Centered, Centered, Color_GetDefaultColor(Color_White));
    }

    Sprite_EndDrawBatch();
    // Draw collected fruits
    mgdl_glSetAlphaTest(false);
    mgdl_glSetTransparency(false);

}

bool FruitFormation::Draw(V2f cursorPos, bool mouseClick)
{
    mgdl_glSetAlphaTest(true);
    mgdl_glSetTransparency(true);
    int mx = mgdl_GetScreenWidth()/2;
    int my = mgdl_GetScreenHeight()/2 + mgdl_GetScreenHeight()/4;
    float time = mgdl_GetElapsedSeconds();
    int size = spriteHeight;

    bool gotFruit = false;

    Sprite_BeginDrawBatch(fruitSprites);
    // Fancier movements!
    for (int i = 0; i < FRUIT_COUNT; i++)
    {
        float angle = (M_PI * 2/(float)FRUIT_COUNT) * i * movedir;
        float radius = size * 1.5 + sin(time) * size*0.5f;
        // modify center of circle
        float ccx = mx;
        float ccy = my;

        ccx += sin(time + angle/2.0f) * sin(time/4.0f) * mx * 0.75;
        ccy += cos((time-angle)/2.0f) * cos(time + angle/8.0f) * my * 0.5f;

        float x = ccx + cos(time + angle) * radius;
        float y = ccy + sin(time + angle) * radius;
        if (collectedFruits[i] == 0)
        {
            if (Sprite_CheckCollisionPoint(fruitSprites, x, y, cursorPos , size, Centered, Centered))
            {
                if (mouseClick)
                {
                    collectedFruits[i] = 1;
                    gotFruit = true;
                    movedir *= -1.0f;

                }
            }
            Sprite_Draw2D(fruitSprites, i, x, y, size, Centered, Centered, Color_GetDefaultColor(Color_White));
        }
        else
        {
            /*Draw collected to the side
            Sprite_Draw2D(fruitSprites, i, size/2, mgdl_GetScreenHeight() - size - size/2 *i, size/2, LJustify, LJustify, Color_GetDefaultColor(Color_White));
            */
        }

    }

    Sprite_EndDrawBatch();
    // Draw collected fruits
    mgdl_glSetAlphaTest(false);
    mgdl_glSetTransparency(false);
    return gotFruit;
}
