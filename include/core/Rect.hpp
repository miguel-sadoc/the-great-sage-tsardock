#pragma once

// Simple geometric structer for AABB collision
// (Axis-Aligned Bounding Box)
struct Rect {

    float left;
    float right;
    float top;
    float bottom;

    // Verifies intersections between two rectangles
    bool intersects(const Rect& other) const {
        if (left   > other.right)  return false;
        if (right  < other.left)   return false;
        if (top    > other.bottom) return false;
        if (bottom < other.top)    return false;
        return true;
    }
};
