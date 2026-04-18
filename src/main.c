//main.c===============================================
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
//macros
typedef float  F;
typedef int    I;
typedef void   V;
typedef double D;
#define R return
typedef struct {I x,y,w,h;} Box;
typedef struct _disp{Box B; SDL_Surface* sf; V (*fnc)(struct _disp* , Uint32* );} disp;
typedef struct {Box B; SDL_Texture* tx; SDL_Texture* txH;SDL_Texture* txP; V (*fnc)(V);} Btn;
disp* disp1;
disp* disp2;
//declares============================================
#define W 948
#define H 533
#define PS W*H
I running;


SDL_Window     *wind;
SDL_Renderer   *rend;
SDL_Surface    *surf;
SDL_Texture *tx;

#define  signalLogW 300
Uint32 signalLog[signalLogW];
F signalLogPoint=0;

//initializers===============================================================
disp* newDisp(Box b, V (* f)(disp* , Uint32* )){
   disp *d= malloc(sizeof(Box));
   d->B=b;
   d->fnc=NULL;
   if(f!=NULL){ d->fnc=f; }
   d->sf= SDL_CreateRGBSurface(0, b.w , b.h, 32, 0, 0, 0, 0);
   return d;
}
Btn* newBtn(Box B, char* pth, char* pthHov, char* pthPress, V (* f)){
   return NULL;
}
//draw funcs===============================================

V drwSignalDisp(disp* d, Uint32* p){
   #define  h d->B.h
   #define  w d->B.w
   #define  xo d->B.x
   #define  yo d->B.y
      for(I y= 0; y< h; y++) {
         for(I x= 0; x< w; x++) {

            I idx = (x+xo)+(y+yo)*W;
            if(idx>=PS || idx < 0){continue;}
            p[idx]=signalLog[(x+(I)signalLogPoint)%signalLogW];
         }
      }
   #undef w
   #undef h
}
V drawDisp(disp* d, Uint32* p){
   #define  h d->B.h
   #define  w d->B.w
   #define  xo d->B.x
   #define  yo d->B.y
   if(d->fnc!=NULL){
      d->fnc(d,p);
   }else{
      for(I y= 0; y< h; y++) {
         for(I x= 0; x< w; x++) {
            I idx = (x+xo)+(y+yo)*W;
            if(idx>=PS || idx < 0){continue;}
            p[idx]=0xFF111111;
         }
      }
   }
   #undef w
   #undef h
}
//=============================================================================
V quit(){ SDL_Quit();printf("quiting...\n"); running=0; }
F acc=0;

V tick(F dt){
   acc+=dt*10;
   while(acc > 0){
      signalLogPoint+=1;
      signalLog[((I)signalLogPoint-1)% signalLogW]=0xFF000000;
      if(rand()%10==1){
         signalLog[((I)signalLogPoint-1)% signalLogW]=rand()%0xFFFFFFFF|0xFF000000;
      }
      acc-=1;
   }
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
         p[x+y*W]=0x00000000;
      }
   }

   drawDisp(disp1, p);
   drawDisp(disp2, p);
   if (SDL_MUSTLOCK(surf)) SDL_UnlockSurface(surf);

   SDL_Texture *ScTx = SDL_CreateTextureFromSurface(rend, surf);//draw this on top alpha cut

   SDL_Rect dst = {0, 0, W, H};
   SDL_RenderClear(rend);
   SDL_RenderCopy(rend, tx, NULL, &dst);
   SDL_RenderCopy(rend, ScTx, NULL, NULL);
   SDL_RenderPresent(rend);
   SDL_DestroyTexture( ScTx);
}




V mainLoop(){
   events();
   tick(1.0/60);//dirty pliz remember to fix
   render();
}

I init(){
   disp1 = newDisp((Box){40, 40, 300, 200}, NULL);
   disp2 = newDisp((Box){40, 250, 300, 30}, drwSignalDisp);
   running=1;

   tx=IMG_LoadTexture(rend, "res/ld59.png");
   if (!tx) { printf("IMG_LoadTexture failed: %s\n", IMG_GetError()); return 0;}
   for(I i = 0; i < signalLogW; i++){
   signalLog[i]=rand()%0xFFFFFFFF;
   }
   printf("init'd\n");
   return 1;
}
I main(){
   SDL_Init(SDL_INIT_VIDEO);
   SDL_CreateWindowAndRenderer(W, H, 0, &wind, &rend);
   surf = SDL_CreateRGBSurfaceWithFormat(0, W, H, 32, SDL_PIXELFORMAT_ARGB8888);

   if(!init()){return 1;}

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
