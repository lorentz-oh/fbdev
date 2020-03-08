#include "window.h"
#include "const.h"
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <iostream>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <cmath>
#include <errno.h>

Color::Color(u_ch R, u_ch G, u_ch B, u_ch A)
{
    Data[0] = B;
    Data[1] = G;
    Data[2] = R;
    Data[3] = A;
}

Color::Color(){}

Window::Window(int Height, int Width):
	m_Height {Height},
	m_Width  {Width}
{
    int fd = open("/dev/fb0", O_RDWR);
	if(fd == -1) {
		m_State = false;
        return;}
	
	if(-1 == fstat(fd, &m_sb)){
		m_State = false;
		return;}

	if(-1 == ioctl(fd, FBIOGET_VSCREENINFO, (char*)&m_fb_info)) {
		m_State = false;
		return;}
	//too beaucoup
    if(Height > m_fb_info.yres || Width > m_fb_info.xres) {
		m_Height = g_DefH;
		m_Width = g_DefW;}

	//g_BtPx is bytes, not bits per pixel
	if(m_fb_info.bits_per_pixel != g_BtPx * 8){
		m_State = false;
		return;}

    if(m_fb_info.xres != 1024){
        m_State = false;
        return;
    }

    m_TempFramebuf = new Color*[m_Height];
	for(int i = 0; i < m_Height; i++){
		m_TempFramebuf[i] = nullptr;
        m_TempFramebuf[i] = new Color [m_Width];
	}

    m_State = MMapFB(fd);
    if(m_State == false){
        return;
    }

	close(fd);

    m_min_fX = -1.0;
    m_min_fY = -( (float)m_Height / m_Width );
    m_norm_dX = std::abs(m_min_fX / m_Width * 2);
    m_norm_dY = m_norm_dX;
}

Window::~Window()
{	
	for(int i = 0; i < m_Height; i++){
		delete [] m_TempFramebuf[i];
	}
	delete [] m_TempFramebuf;

    UnMMapFB();
}

bool Window::MMapFB(int FD)
{
    m_RealFramebuf = new u_ch*[m_fb_info.yres];
    size_t RowSize = m_fb_info.xres * g_BtPx;
    for(size_t i = 0; i < m_fb_info.yres ; i++){
        m_RealFramebuf[i] = (u_ch*)mmap(nullptr,
                                        RowSize,
                                        PROT_WRITE,
                                        MAP_SHARED,
                                        FD,
                                        RowSize * i);
        if(m_RealFramebuf[i] == MAP_FAILED){
            return false;
        }
    }
    return true;
}

int Window::UnMMapFB()
{
    size_t RowSize = m_fb_info.xres * g_BtPx;
    for(size_t i = 0; i < m_fb_info.yres; i++){
        munmap(m_RealFramebuf[i], RowSize);
    }
    delete [] m_RealFramebuf;
    return 1;
}

inline void Window::FlushPixel(int Y, int X)
{
    static int Gap = (m_fb_info.yres - m_Height) / 2;
    static int Margin = (m_fb_info.xres - m_Width) / 2;
    for(int i = 0; i < g_BtPx; i++){
        m_RealFramebuf[Y+Gap][(X+Margin)*g_BtPx+i] = m_TempFramebuf[Y][X].Data[i];
    }
}

void Window::Dump()
{
    #pragma omp parallel for
    for(int Y = 0; Y < m_Height; Y++){
        for(int X = 0; X < m_Width; X++){
            FlushPixel(Y,X);
        }
    }
}

void Window::ForEachPixelNorm(Color (*Func)(float Y, float X))
{
    float fY = m_min_fY;
    float fX = m_min_fX;
    Color C{};
    #pragma omp parallel for schedule(dynamic) shared(fY) firstprivate(fX)
    for(int Y = 0; Y < m_Height; Y++){
        float ThisFY = fY;
        fY += m_norm_dY;
        for(int X = 0; X < m_Width; X++){
            C = Func(ThisFY, fX);
            m_TempFramebuf[Y][X] = C;
            fX += m_norm_dX;
        }
        fX = m_min_fX;
    }
}
