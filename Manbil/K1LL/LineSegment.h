#pragma once

#include "../Math/Shapes/Circle.h"


struct LineSegment
{
    Vector2f Start, End;


    LineSegment(Vector2f start, Vector2f end) : Start(start), End(end) { }


    inline bool TouchesCircle(const Circle& circ)
    {
        return circ.GetIntersections(Start, End, dummyIntersections) > 0;
    }

    Vector2f GetNormal(void) const { return Vector2f(End.y - Start.y, End.x - Start.x).Normalized(); }
    inline Vector2f Reflect(Vector2f inDir) const
    {
        Vector2f wallN = GetNormal();
        return inDir - (wallN * 2.0f * wallN.Dot(inDir));
    }


private:

    static Vector2f dummyIntersections[2];
};