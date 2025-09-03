#pragma once
struct App;

namespace picking 
{
    void handle3D(App& app, double sx, double sy, bool useBVH, bool* outHit = nullptr);
}
