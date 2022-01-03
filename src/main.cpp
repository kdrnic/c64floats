#include <allegro.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <math.h>
#include <locale.h>
#include <errno.h>

#include "C64Float.h"

double freqR, freqG, freqB;
double phaseR, phaseG, phaseB;

template<class floatType>
void draw_mandelbrot(BITMAP *bmp, double centerX, double centerY, double radius, const int IterationMax)
{
	/* screen ( integer) coordinate */
	int iX,iY;
	const int iXmax = bmp->w; 
	const int iYmax = bmp->h;
	
	floatType f0 = 0.0;
	floatType f2 = 2.0;
	
	/* world ( double) coordinate = parameter plane*/
	floatType Cx,Cy;
	const floatType CxMin= centerX - radius;
	const floatType CxMax= centerX + radius;
	const floatType CyMin= centerY - radius * (floatType) bmp->h / (floatType) bmp->w;
	const floatType CyMax= centerY + radius * (floatType) bmp->h / (floatType) bmp->w;;
	/* */
	const floatType PixelWidth=(CxMax-CxMin)/ floatType(iXmax);
	const floatType PixelHeight = PixelWidth;
	/* color component ( R or G or B) is coded from 0 to 255 */
	/* it is 24 bit color RGB file */
	const int MaxColorComponentValue=255; 
	unsigned char color[3];
	/* Z=Zx+Zy*i  ;   Z0 = 0 */
	floatType Zx, Zy;
	floatType Zx2, Zy2; /* Zx2=Zx*Zx;  Zy2=Zy*Zy  */
	/*  */
	int Iteration;
	/* bail-out value , radius of circle ;  */
	const floatType EscapeRadius=2;
	const floatType ER2=EscapeRadius*EscapeRadius;
	/* compute and write image data bytes to the file*/
	for(iY=0;iY<iYmax;iY++){
		//printf("%d%%\n", (100*(iY * iXmax))/(iXmax*iYmax));
		Cy=CyMin + floatType(iY) *PixelHeight;
		if(abs(Cy)< PixelHeight/floatType(2)) Cy=f0; /* Main antenna */
		for(iX=0,Cx=CxMin;iX<iXmax;iX++,Cx+=PixelWidth){
			/* initial value of orbit = critical point Z= 0 */
			Zx=f0;
			Zy=f0;
			Zx2=Zx*Zx;
			Zy2=Zy*Zy;
			/* */
			for(Iteration=0;Iteration<IterationMax && ((Zx2+Zy2)<ER2);Iteration++){
				Zy=f2*Zx*Zy + Cy;
				Zx=Zx2-Zy2 +Cx;
				Zx2=Zx*Zx;
				Zy2=Zy*Zy;
			};
			/* compute  pixel color (24 bit = 3 bytes) */
			if(Iteration==IterationMax){
				/*  interior of Mandelbrot set = black */
				color[0]=0;
				color[1]=0;
				color[2]=0;
			}
			else{ /* exterior of Mandelbrot set = white */
				double mandelColor = (double) Iteration / (double) IterationMax;
				color[0]=255 * (sin((mandelColor + phaseR) * freqR) + 1.0) * 0.5;
				color[1]=255 * (sin((mandelColor + phaseG) * freqG) + 1.0) * 0.5;
				color[2]=255 * (sin((mandelColor + phaseB) * freqB) + 1.0) * 0.5;
			}
			putpixel(bmp, iX, iY, makecol(color[0], color[1], color[2]));
		}
	}
}

static double linterp(double s, double e, double i)
{
	return s + (e - s) * i;
}

int main(int argc, char **argv)
{
	allegro_init();
	set_color_depth(32);
	set_gfx_mode(GFX_AUTODETECT_WINDOWED, 640, 480, 0, 0);
	install_keyboard();
	
	srand(time(0));
	
	BITMAP *gif_bmp = create_bitmap(120, 90);
	
	int itMax = 25;
	double x = -0.7;
	double y = 0.0;
	double r = 1.5;
	
	randomise:
	freqR = ((double) rand() / (double) RAND_MAX) * 30.0;
	freqG = ((double) rand() / (double) RAND_MAX) * 30.0;
	freqB = ((double) rand() / (double) RAND_MAX) * 30.0;
	phaseR = ((double) rand() / (double) RAND_MAX) * 2.0 * 3.141592653;
	phaseG = ((double) rand() / (double) RAND_MAX) * 2.0 * 3.141592653;
	phaseB = ((double) rand() / (double) RAND_MAX) * 2.0 * 3.141592653;
	
	while(!key[KEY_ESC]){
		clear(gif_bmp);
		
		draw_mandelbrot<double>(gif_bmp, x, y, r, itMax);
		stretch_blit(gif_bmp, screen, 0, 0, gif_bmp->w, gif_bmp->h, 0, 0, SCREEN_W, SCREEN_H);
		
		if(key[KEY_LEFT]) itMax--;
		if(key[KEY_RIGHT]) itMax++;
		if(itMax <= 0) itMax = 1;
		if(key[KEY_SPACE]) goto randomise;
	}
	
	char fn[256];
	sprintf(fn, "mandelbrot_x86_%dx%d.bmp", gif_bmp->w, gif_bmp->h);
	save_bitmap(fn, gif_bmp, 0);
	
	long long unsigned c0 = C64Float::GetCycles();
	time_t t0 = time(0);
	clear(gif_bmp);
	draw_mandelbrot<C64Float>(gif_bmp, x, y, r, itMax);
	time_t t1 = time(0);
	int deltat = difftime(t1, t0);
	long long unsigned c1 = C64Float::GetCycles();
	printf("took %d seconds, %llu 6502 cycles (%llu C64 seconds)\n", deltat, c1 - c0, (c1 - c0) / 1022727);
	
	sprintf(fn, "mandelbrot_c64_%dx%d.bmp", gif_bmp->w, gif_bmp->h);
	save_bitmap(fn, gif_bmp, 0);
	
	allegro_exit();
	
	return 0;
}
END_OF_MAIN()
