#include <complex>
#include "viewer.h"
#include "const.h"

static float Zoom = 1.0;
static float ViewX = 0.0;
static float ViewY = 0.0;
static int Iters = 20;

Viewer::Viewer():
    Win{480, 640}
{
    if(!Win){
        m_State = false;
        return;
    }
    initscr();
    raw();
    noecho();
    refresh();
    curs_set(0);
    printw("Press C to quit, move with W/A/S/D, resize with Q/E, adjust iterations with R/F");
    Zoom = 1.0f;
    ViewX = 0.0f;
    ViewY = 0.0f;

}

Viewer::~Viewer()
{
    endwin();
}

void Viewer::Run()
{
    while(IsRunning){
        Win.ForEachPixelNorm(Mandelbrot);
        Win.Dump();
        GetInput();
    }
}

inline Color Viewer::Mandelbrot(float Im, float Re)
{
    Im *= Zoom;
    Re *= Zoom;
    Im += ViewY;
    Re += ViewX;
    std::complex<float> C{Re, Im};
    std::complex<float> Z{0.0f, 0.0f};

    for(int i = 0; i < Iters; i++){
        Z = Z * Z + C;
        if(std::abs(Z) > 2.0f){
            return Color{255,255,255,255};}
    }
    return Color{0,0,0,0};
}

void Viewer::GetInput()
{
    int Key;
    Key = getch();
    switch(Key){
    case 'a':
        ViewX -= Zoom / 3.0f;
        break;
    case 'd':
        ViewX += Zoom / 3.0f;
        break;
    case 'w':
        ViewY -= Zoom / 3.0f;
        break;
    case 's':
        ViewY += Zoom / 3.0f;
        break;
    case 'q':
        Zoom *= 1.5f;
        break;
    case 'e':
        Zoom /= 1.5f;
        break;
    case 'c':
        IsRunning = false;
        break;
    case 'r':
        Iters++;
        break;
    case 'f':
        Iters--;
        break;
    }
}
