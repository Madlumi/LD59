//main.c===============================================
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL.h>
//macros
typedef float  F;
typedef int    I;
typedef void   V;
typedef double D;
#define R return
//declares============================================
#define W 948
#define H 533
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
V render(){
   if (SDL_MUSTLOCK(surf)) SDL_LockSurface(surf);
   Uint32 * p = surf->pixels;

   for(I y= 0; y< H; y++) {
   for(I x= 0; x< W; x++) {
         p[x+y*W]=0xFFFF00FF;
   }
   }
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
