#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <cstring>
#include <list>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <queue>

const double SK = 0.23873241463784;
const double G = 0.1;
const double T = 0.5;
const int SCREEN_WIDTH = 960;
const int SCREEN_HEIGHT = 640;
const int BODYNUMS = 200;
const int MAXLENGTH = 100;

SDL_Window *wnd = NULL;
SDL_Renderer *ren = NULL;
TTF_Font *font;

struct point{
    int x,y;
    point(int xx, int yy){
        x=xx,y=yy;
    }
};

double cbrt(double x)
{
	double x0 = 1, x1 = x0 - (x0 * x0 * x0 - x) / (3 * x0 * x0);
	while (x0 - x1 > 0.00000000000001 || x0 - x1 < -0.00000000000001)
	{
		x0 = x1 - (x1 * x1 * x1 - x) / (3 * x1 * x1);
		x1 = x0 - (x0 * x0 * x0 - x) / (3 * x0 * x0);
	}
	return x1;
}

void DrawCircle(SDL_Color col, int x, int y, int r)
{
    SDL_SetRenderDrawColor(ren, col.r, col.g, col.b, 255);
	int xc = 0, yc = r, i, d;
	d = 3 - (r << 1);
	while (xc <= yc)
	{
		for (i = xc; i <= yc; i++)
		{
			SDL_RenderDrawPoint(ren, x + xc, y + i);
			SDL_RenderDrawPoint(ren, x - xc, y + i);
			SDL_RenderDrawPoint(ren, x + xc, y - i);
			SDL_RenderDrawPoint(ren, x - xc, y - i);
			SDL_RenderDrawPoint(ren, x + i, y + xc);
			SDL_RenderDrawPoint(ren, x - i, y + xc);
			SDL_RenderDrawPoint(ren, x + i, y - xc);
			SDL_RenderDrawPoint(ren, x - i, y - xc);
		}
		if (d < 0) d += (xc << 2) + 6;
		else
		{
			d += ((xc - yc) << 2) + 10;
			yc--;
		}
		xc++;
	}
}

void DrawText(std::string text, int x, int y, SDL_Color col){
    SDL_Surface *text_surface = TTF_RenderText_Solid(font, text.c_str(), col);
    SDL_Texture *text_texture = SDL_CreateTextureFromSurface(ren, text_surface);
    SDL_Rect rec = {x, y, text_surface->w, text_surface->h};
    SDL_RenderCopy(ren, text_texture, NULL, &rec);
    SDL_DestroyTexture(text_texture);
    SDL_FreeSurface(text_surface);
}

class Body
{
	public:
		Body(int Bodynum)
		{
			x = rand() % SCREEN_WIDTH;
			y = rand() % SCREEN_HEIGHT;
			m = rand() % 500 + 10.;
			r = cbrt(m * SK);
			vx = (rand() & 1 ? 1 : -1) * (rand() % 401 / 200.);
			vy = (rand() & 1 ? 1 : -1) * (rand() % 401 / 200.);
			num = Bodynum;
			col = {rand() % 256, rand() % 256, rand() % 256};
			for (int i = 0; i < MAXLENGTH; i++)
			{
				path.push_back(point(x, y));
			}
		}
		void move()
		{
            x += vx * T;
            y += vy * T;
            path.push_back(point(x, y));
            path.pop_front();
		}
		void show()
		{
            DrawCircle(col, x, y, r);
            std::list<point>::iterator it1 = path.begin(), it2 = it1;
            ++it2;
            while (it2 != path.end())
            {
                SDL_RenderDrawLine(ren, (*it1).x, (*it1).y, (*it2).x, (*it2).y);
                ++it1;
                ++it2;
            }
            DrawText("Body:" + std::to_string(num), x, y, {255,255,255});
		}
		static void gravitation(Body &a, Body &b)
		{
			double tmp1, tmp2;
            if ((a.r + b.r) * (a.r + b.r) < (b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y))
            {
                tmp1 = G * b.m / ((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y));
                tmp2 = sqrt((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y));
                a.vx += tmp1 * ((b.x- a.x) / tmp2);
                a.vy += tmp1 * ((b.y - a.y) / tmp2);
            }
            else
            {
                if (a.m >= b.m)
                {
                    a.vx = (a.m * a.vx + b.m * b.vx) / (a.m + b.m);
                    a.vy = (a.m * a.vy + b.m * b.vy) / (a.m + b.m);
                    a.m += b.m;
                    a.r = cbrt(a.m * SK);
                    b.m = 0;
                    b.r = 0;
                    b.vx = 0;
                    b.vy = 0;
                }
                else
                {
                    b.vx = (a.m * a.vx + b.m * b.vx) / (a.m + b.m);
                    b.vy = (a.m * a.vy + b.m * b.vy) / (a.m + b.m);
                    b.m += a.m;
                    b.r = cbrt(b.m * SK);
                    a.m = 0;
                    a.r = 0;
                    a.vx = 0;
                    a.vy = 0;
                }
            }
        }
		double x;
		double y;
		double m;
		double r;
		double vx;
		double vy;
		int num;
		SDL_Color col;
		std::list<point> path;
};

int main(int argc, char *argv[])
{
	SDL_Event e;
	bool run = true;
	srand(time(NULL));
	std::list<Body> bodies;
	if (SDL_Init(SDL_INIT_VIDEO) == -1)
	{
		std::cerr << "Error: " << SDL_GetError();
		return -1;
	}
	if(TTF_Init()==-1) {
        std::cerr << "Error: " << TTF_GetError();
        return 1;
    }
    font=TTF_OpenFont("font.ttf", 12);
    if(!font) {
        std::cerr << "Error: " << TTF_GetError();
        return -1;
    }
	wnd = SDL_CreateWindow("Planet", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (!wnd)
	{
		std::cerr << "Error: " << SDL_GetError();
		SDL_Quit();
		return -1;
	}
	ren = SDL_CreateRenderer(wnd, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!ren)
	{
		std::cerr << "Error: " << SDL_GetError();
		SDL_DestroyWindow(wnd);
		SDL_Quit();
		return -1;
	}
	for(int i = 1; i <= BODYNUMS; i++)bodies.push_back(Body(i));
	std::list<Body>::iterator i, j;
	int listnum = 0;
	while (run)
	{
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT) run = false;
		}
		SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
		SDL_RenderClear(ren);
		std::queue<std::list<Body>::iterator> earse;
		for (i = bodies.begin(); i != bodies.end(); ++i)
		{
			(*i).move();
			for (j = bodies.begin(); j != bodies.end(); ++j)
			{
				if (i != j) Body::gravitation(*i, *j);
			}
			if((*i).m == 0){
                earse.push(i);
                continue;
            }
			(*i).show();
		}
		while(!earse.empty()){
            std::list<Body>::iterator t = earse.front();
            earse.pop();
            bodies.erase(t);
		}
		for (i = bodies.begin(),listnum = 0; i != bodies.end(); ++i, ++listnum){
            if((listnum + 1) * TTF_FontHeight(font) <= SCREEN_HEIGHT){
                DrawText("Body:" + std::to_string((*i).num) + " m:" + std::to_string((*i).m) + " r:" + std::to_string((*i).r), 0, listnum * TTF_FontHeight(font), (*i).col);
			}
		}
		SDL_RenderPresent(ren);
	}
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(wnd);
	SDL_Quit();
	return 0;
}
