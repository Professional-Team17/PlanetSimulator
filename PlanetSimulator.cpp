#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <cstring>
#include <list>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <queue>

const double SK = 0.2;
const double G = 0.1;
const double T = 0.5;
const int SCREEN_WIDTH = 960;
const int SCREEN_HEIGHT = 640;
const int BODYNUMS = 200;
const int MAXLENGTH = 100;

SDL_Window *wnd = NULL;
SDL_Renderer *ren = NULL;
TTF_Font *font = NULL;

struct point{
    double x,y;
	point(){}
    point(double xx, double yy){
        x=xx,y=yy;
    }
};

point mid;
double ratio;

double ratiox(double x){
	return SCREEN_WIDTH/2-ratio*(mid.x-x);
}
double ratioy(double y){
	return SCREEN_HEIGHT/2-ratio*(mid.y-y);
}
double deratiox(double x){
	return mid.x-(SCREEN_WIDTH/2-x)/ratio;
}
double deratioy(double y){
	return mid.y-(SCREEN_HEIGHT/2-y)/ratio;
}

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
			col = {Uint8(rand() % 256), Uint8(rand() % 256), Uint8(rand() % 256)};
			for (int i = 0; i < MAXLENGTH; i++)
			{
				path.push_back(point(x, y));
			}
		}
		Body(int Bodynum, point pos, point vpos)
		{
			x = pos.x;
			y = pos.y;
			m = rand() % 500 + 10.;
			r = cbrt(m * SK);
			vx = vpos.x-pos.x;
			vy = vpos.y-pos.y;
			num = Bodynum;
			col = {Uint8(rand() % 256), Uint8(rand() % 256), Uint8(rand() % 256)};
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
            DrawCircle(col, ratiox(x), ratioy(y), ratio*r);
            std::list<point>::iterator it1 = path.begin(), it2 = it1;
            ++it2;
            while (it2 != path.end())
            {
                SDL_RenderDrawLine(ren, ratiox((*it1).x), ratioy((*it1).y), ratiox((*it2).x), ratioy((*it2).y));
                ++it1;
                ++it2;
            }
            DrawText("Body:" + std::to_string(num), ratiox(x), ratioy(y), {255,255,255});
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
	wnd = SDL_CreateWindow("PlanetSimulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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
	int listnum;
	int bodynum;
	bool is_botton=0;
	point lay;
	bool is_lay;
	bool is_pause;
	bool is_find;
start:
	for(int i = 1; i <= BODYNUMS; i++)bodies.push_back(Body(i));
	std::list<Body>::iterator i, j;
	listnum = 0;
	bodynum=BODYNUMS;
	ratio=1;
	is_lay=0;
	lay=point(0,0);
	mid=point(SCREEN_WIDTH/2,SCREEN_HEIGHT/2);
	is_pause=0;
	is_find=0;
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
			if(!is_pause){
				(*i).move();
				for (j = bodies.begin(); j != bodies.end(); ++j)
				{
					if (i != j) Body::gravitation(*i, *j);
				}
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
		if(is_lay){
			SDL_SetRenderDrawColor(ren,255,255,255,255);
			SDL_RenderDrawLine(ren,ratiox(lay.x),ratioy(lay.y),double(e.button.x),double(e.button.y));
			DrawCircle({255,255,255},ratiox(lay.x),ratioy(lay.y),5);
			DrawText("laypoint",ratiox(lay.x),ratioy(lay.y),{255,255,255});
		}
		SDL_RenderPresent(ren);
		if(e.button.button!=SDL_BUTTON_LEFT&&e.button.button!=SDL_BUTTON_RIGHT&&e.key.keysym.sym!=SDLK_p&&e.key.keysym.sym!=SDLK_f){
			is_botton=0;
		}
		if(e.type == SDL_MOUSEWHEEL){
			if(e.wheel.y>0&&ratio<=5){
				ratio+=0.01;
			}
			if(e.wheel.y<0&&ratio>=0.2){
				ratio-=0.01;
			}
		}
		switch (e.key.keysym.sym)
		{
		case SDLK_UP:
			mid.y--;
			break;
		case SDLK_LEFT:
			mid.x--;
			break;
		case SDLK_DOWN:
			mid.y++;
			break;
		case SDLK_RIGHT:
			mid.x++;
			break;
		default:
			break;
		}
		if(is_find){
			std::list<Body>::iterator maxn;
			double maxm=0;
			for(i = bodies.begin(); i != bodies.end(); ++i){
				if((*i).m>maxm){
					maxn=i;
					maxm=(*i).m;
				}
			}
			mid=point((*maxn).x,(*maxn).y);
		}
		if(is_botton){
			continue;
		}
		if(e.key.keysym.sym==SDLK_p){
			is_pause = !is_pause;
			is_botton=1;
		}
		if(e.key.keysym.sym==SDLK_f){
			is_find = !is_find;
			is_botton=1;
		}
		if(!is_lay&&e.button.button==SDL_BUTTON_LEFT){
			lay=point(deratiox(double(e.button.x)),deratioy(double(e.button.y)));
			is_lay=1;
			is_botton=1;
		}
		else if(is_lay&&e.button.button==SDL_BUTTON_LEFT){
			bodies.push_back(Body(bodynum,lay,point(deratiox(double(e.button.x)),deratioy(double(e.button.y)))));
			bodynum++;
			is_botton=1;
		}
		else if(is_lay&&e.button.button==SDL_BUTTON_RIGHT){
			is_lay=0;
			lay=point(0,0);
			is_botton=1;
		}
		else if(!is_lay&&e.button.button==SDL_BUTTON_RIGHT){
			bodies.clear();
			is_botton=1;
			goto start;
		}
	}
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(wnd);
	SDL_Quit();
	return 0;
}
