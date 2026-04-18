//main.c
#include <SDL2/SDL_surface.h>
#include <stdio.h>
#include <SDL2/SDL.h>
//macros
typedef float  F;
typedef int    I;
typedef void   V;
typedef double D;
#define R return
//declares
#define W 948
#define H 533


SDL_Window     *wind;
SDL_Renderer   *rend;
SDL_Surface    *surf;

//
I main(){
   SDL_Init(SDL_INIT_VIDEO);
   SDL_CreateWindowAndRenderer(W, H, 0, &wind, &rend);

   printf("hewwo!\n");
   R 0;
}
