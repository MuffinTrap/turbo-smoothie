#pragma once

#include <mgdl.h>

static bool PointInsideRect(float x, float y, float rx, float ry, float w, float h)
{
    return (x >= rx && x < rx + w && y >= ry && y < ry + h);
}

// TODO Move to Sprite
static bool Sprite_CheckCollisionPoint(Sprite* sprite, float x, float y, V2f point, float scale, AlignmentModes horizontal, AlignmentModes vertical)
{
   float h = scale;
   //float w = scale;
   // TODO ! Sprite_GetAspect;

    float w = sprite->_font->_aspect * scale;
   float tlx = x;
   float tly = y;
   if (horizontal == RJustify) { tlx -= w;}
   else if (horizontal == Centered) { tlx -= w/2.0f;}
   if (vertical == RJustify) { tly -= h;}
   else if (vertical == Centered) { tly -= h/2.0f;}

    bool is = PointInsideRect(V2f_X(point), V2f_Y(point), tlx, tly, w, h);
   /*

    Draw2D_RectLines(tlx, tly, tlx + w, tly+h, Color_GetDefaultColor(Color_Red));
    if (is)
    {
        Draw2D_RectLines(tlx, tly, tlx + w, tly+h, Color_GetDefaultColor(Color_Green));
    }
    else
    {
        Draw2D_RectLines(tlx, tly, tlx + w, tly+h, Color_GetDefaultColor(Color_Red));
    }
    */
    return is;
}
