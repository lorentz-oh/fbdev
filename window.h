#pragma once

#include <sys/stat.h>
#include <linux/fb.h> 

typedef unsigned char u_ch;

struct Color
{
    Color();
    Color(u_ch R, u_ch G, u_ch B, u_ch A);
    u_ch Data[4];
};

//State is set to false if constructor failed
class Window
{
public:
	Window();

	Window(int Height, int Width);	
	
	~Window();

    bool MMapFB(int FD);

    int UnMMapFB();

    void FlushPixel(int Y, int X);

    void ForEachPixelNorm(Color(*Func)(float Y, float X));
	//Dump contents of temporary framebuffer onto the screen
    void Dump();

	bool operator!()
	{
		return !m_State;
	}

private:
	//Pixel size in normalized device coordinates
	float m_norm_dY = 0.0f;
	float m_norm_dX = 0.0f;
    //Minimal X and Y values in normalized device coordinates
    float m_min_fY;
    float m_min_fX;
	//Dimensions of the created window
	int m_Height = 480; 
	int m_Width = 640;  
	//Is the window was created without errors
	bool m_State = true;
	//Frame buffer for drawing on
    Color** m_TempFramebuf = nullptr;
	//Memory map of the real framebuffer, data is copied from 
	//temporary buffer to there
    u_ch** m_RealFramebuf = nullptr;
	//Information about screen obtained via ioctl()
	fb_var_screeninfo m_fb_info;
	//Framebuffer file statistics
	struct stat m_sb;
};
