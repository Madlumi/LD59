//main.c===============================================
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL.h>
#include <stdlib.h>
//macros
typedef float  F;
typedef int    I;
typedef void   V;
typedef double D;
#define R return
typedef struct {I x,y,w,h;} Box;
typedef struct {Box B; SDL_Surface* sf;} disp;
disp* disp1;
//declares============================================
#define W 948
#define H 533
#define PS W*H
I running;


SDL_Window     *wind;
SDL_Renderer   *rend;
SDL_Surface    *surf;

//funcs===============================================

V quit(){ SDL_Quit();printf("quiting...\n"); running=0; }
V tick(F dt){

}
V events(){
   SDL_Event e;
   while(SDL_PollEvent(&e)){
      if(e.type==SDL_QUIT){
         quit();
      }
   }
}

disp* newDisp(Box b){
   disp *d= malloc(sizeof(Box));
   d->B=b;
   d->sf= SDL_CreateRGBSurface(0, b.w , b.h, 32, 0, 0, 0, 0);
   return d;
}
V drawDisp(disp* d, Uint32* p){
   #define  h d->B.h
   #define  w d->B.w
   #define  xo d->B.x
   #define  yo d->B.y
   for(I y= 0; y< h; y++) {
      for(I x= 0; x< w; x++) {
         I idx = (x+xo)+(y+yo)*W;
         if(idx>=PS || idx < 0){continue;}
         p[idx]=0xFF111111;
      }
   }
   #undef w
   #undef h
}
V render(){
   if (SDL_MUSTLOCK(surf)) SDL_LockSurface(surf);
   Uint32 * p = surf->pixels;

   for(I y= 0; y< H; y++) {
      for(I x= 0; x< W; x++) {
         p[x+y*W]=0xFFFF00FF;
      }
   }

   drawDisp(disp1, p);
   if (SDL_MUSTLOCK(surf)) SDL_UnlockSurface(surf);

   SDL_Texture *ScTx = SDL_CreateTextureFromSurface(rend, surf);

   SDL_RenderClear(rend);
   SDL_RenderCopy(rend, ScTx, NULL, NULL);
   SDL_RenderPresent(rend);
   SDL_DestroyTexture( ScTx);
}




V mainLoop(){
   events();
   tick(1.0/60);//dirty pliz remember to fix
   render();
}

I main(){
   SDL_Init(SDL_INIT_VIDEO);
   SDL_CreateWindowAndRenderer(W, H, 0, &wind, &rend);
   surf= SDL_CreateRGBSurface(0, W , H, 32, 0, 0, 0, 0);

   disp1 = newDisp((Box){40, 40, 500, 1200});
   running=1;

   #ifdef __EMSCRIPTEN__
   emscripten_set_main_loop(mainLoop, 0, 1);
#else
   while(running) {        
      mainLoop();
      SDL_Delay(8);
   }
#endif 

}
//eof main.c============================================
