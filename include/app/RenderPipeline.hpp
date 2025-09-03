#pragma once
struct App;

namespace renderpipe 
{
    void draw3D(App& app);         // opaque + debug (hit lines, AABB)
    void draw2D(App& app);         // calls scene->render2D(...)
}
