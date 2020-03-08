#pragma once

#include "window.h"
#include <ncurses.h>

class Viewer
{
public:
    Viewer();
    ~Viewer();
    void Run();
    void GetInput();
    inline static Color Mandelbrot(float Im, float Re);
    bool operator!()
    {
        return !m_State;
    }
private:
    Window Win;
    bool IsRunning = true;
    bool m_State = true;
};
