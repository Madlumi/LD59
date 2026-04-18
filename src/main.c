//main.c===============================================
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "mnoise.h"
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
#define ef else if
typedef struct {I x,y,w,h;} Box;
typedef struct _disp{Box B; SDL_Surface* sf; V (*fnc)(struct _disp* , Uint32* );} disp;
typedef struct {Box B; SDL_Texture* tx; SDL_Texture* txH;SDL_Texture* txP; V (*fnc)(V); I status;} Btn;
//declares============================================
#define W 948
#define H 533
#define PS W*H
I running;

I mx; I my; //mouse
#define mkeyn 12 
I MKEYS[mkeyn];

SDL_Window     *wind;
SDL_Renderer   *rend;
SDL_Surface    *surf;
SDL_Texture    *tx;
disp* disp1;
disp* disp2;
Btn*  btn1;
Btn*  btn2;
Btn*  btn3;
Btn*  btn4;
I AlresponseDelay=20;
I AlsilentCount;

#define  signalLogW 300
Uint32 signalLog[signalLogW];
Uint32 snoiseLog[signalLogW];
F signalLogPoint=0;

//helpers
I inBox(I x, I y, Box B){
   if(x<B.x || y< B.y){return 0;}
   if(x-B.w > B.x || y-B.h> B.y){return 0;}

   return 1;
}

typedef enum {
   BLACK =  0xFF000000,
   RED =    0xFFFF0000,
   BLUE =   0xFF0000FF,
   GREEN=   0xFF00FF00,
   PINK =   0xFFFF00FF,
} sigCol;







V quit(){ SDL_Quit();printf("quiting...\n"); running=0; }
//initializers===============================================================
disp* newDisp(Box b, V (* f)(disp* , Uint32* )){
   disp *d= malloc(sizeof(disp));
   d->B=b;
   d->fnc=NULL;
   if(f!=NULL){ d->fnc=f; }
   d->sf= SDL_CreateRGBSurface(0, b.w , b.h, 32, 0, 0, 0, 0);
   return d;
}
Btn* newBtn(Box B, char* pth, char* pthHov, char* pthPress, V (* f)){
   Btn *b = malloc(sizeof(Btn));
   b->B=B;
   b->tx=IMG_LoadTexture(rend, pth);
   b->txH=IMG_LoadTexture(rend, pthHov);
   b->txP=IMG_LoadTexture(rend, pthPress);
   if (!tx) { printf("IMG_LoadTexture failed: %s\n", IMG_GetError()); quit();}
   if(f!=NULL){ b->fnc=f; }
   b->status=0;
   return b;
}
typedef  enum {
   MT_MIX,
   MT_ADD,
} mixType;
I mixCol(I c1, I c2, F pow, mixType t){
   if(pow < 0) pow = 0; if(pow > 1) pow = 1;
   I a1 = (c1 >> 24) & 0xFF;
   I r1 = (c1 >> 16) & 0xFF;
   I g1 = (c1 >> 8)  & 0xFF;
   I b1 =  c1        & 0xFF;

   I a2 = (c2 >> 24) & 0xFF;
   I r2 = (c2 >> 16) & 0xFF;
   I g2 = (c2 >> 8)  & 0xFF;
   I b2 =  c2        & 0xFF;

   I a = 0; I r = 0; I g = 0; I b = 0;
   if (t==MT_MIX){
      a = (I)(a2 + ((a1-a2)*pow));
      r = (I)(r2 + ((r1-r2)*pow));
      g = (I)(g2 + ((g1-g2)*pow));
      b = (I)(b2 + ((b1-b2)*pow));
   }ef(t==MT_ADD){
      a = a2 + (I)(a1 * pow); if (a > 0xFF) a = 0xFF;
      r = r2 + (I)(r1 * pow); if (r > 0xFF) r = 0xFF;
      g = g2 + (I)(g1 * pow); if (g > 0xFF) g = 0xFF;
      b = b2 + (I)(b1 * pow); if (b > 0xFF) b = 0xFF;
   }
   return (a << 24) | (r << 16) | (g << 8) | b;
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
         p[idx]=mixCol(signalLog[(x+(I)signalLogPoint)%signalLogW], snoiseLog[(x+(I)signalLogPoint)%signalLogW],1,MT_ADD);
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
I drwCol;
V setDrwCol(I c){ drwCol=c; }
V setDrwCol0(){ drwCol=BLACK; }
V setDrwCol1(){ drwCol=RED; }
V setDrwCol2(){ drwCol=GREEN; }
V setDrwCol3(){ drwCol=BLUE; }
V setDrwCol4(){ drwCol=PINK; }
//=============================================================================
F acc=0;

F t = 0;
#define sigIdx ((I)signalLogPoint-1)% signalLogW
V generateNoiseSignal(){
      signalLogPoint+=1;
      snoiseLog[((I)signalLogPoint-1)% signalLogW]=0xFF000000;
      I r =(I)(Perlin1D(t*2.5+13277)*0xFF)&0xFF;
      I g =(I)(Perlin1D(t*2+37731)*0xFF)&0xFF;
      I b =(I)(Perlin1D(t*2.3+12567)*0xFF)&0xFF;
      I noise = 0xFF000000 | (r << 16) | (g << 8) | b;
      F noiseStr=Perlin1D(t*5)*.2;
      snoiseLog[((I)signalLogPoint-1)% signalLogW]= mixCol(noise, 0xFF000000, noiseStr, MT_MIX);
}
V readSignalbuffer(){
   I last=0;
   for(I i = 0; i < signalLogW ; i++){
      I c = signalLog[(sigIdx+i)%signalLogW];
      if(c!=last){ 
         if(c==RED){printf("RED\n");}
         if(c==GREEN){printf("GREEN\n");}
         if(c==BLUE){printf("BLUE\n");}
         if(c==PINK){printf("PIN\n");}
         last=c;
      }
   }
}
I new = 0;
F pressPwr=0;
V tick(F dt){
   acc+=dt*10;
   t+=dt;
   if(MKEYS[1]){
      if(inBox(mx, my, (*btn1).B)){btn1->fnc(); pressPwr+=dt*3; new+=1; };
      if(inBox(mx, my, (*btn2).B)){btn2->fnc(); pressPwr+=dt*3; new+=1; };
      if(inBox(mx, my, (*btn3).B)){btn3->fnc(); pressPwr+=dt*3; new+=1; };
      if(inBox(mx, my, (*btn4).B)){btn4->fnc(); pressPwr+=dt*3; new+=1; };
      if(pressPwr>1){pressPwr=1;}
   }else{ pressPwr-=dt*3; if(pressPwr<0){pressPwr=0;} }
   while(acc > 0){
      generateNoiseSignal();
      signalLog[sigIdx]=0;
      if(pressPwr>.01){signalLog[sigIdx]= mixCol(drwCol,0xFF000000, pressPwr, MT_MIX);}
      if(signalLog[sigIdx]==0&&new>3){AlsilentCount++;}else{AlsilentCount=0;}
      if(AlsilentCount>AlresponseDelay){new=0; AlsilentCount=0 ; readSignalbuffer();  printf("WEWOO\n"); }
      acc-=1;
   }
}
V events(){
   SDL_Event e;
   SDL_GetMouseState(&mx, &my);
   for(int i = 0; i < mkeyn; i++){if(MKEYS[i]>1){MKEYS[i]--;}}
   while(SDL_PollEvent(&e)){
      if (e.type == SDL_QUIT){ quit(); }
      ef (e.type == SDL_MOUSEBUTTONDOWN){ if(e.button.button>= mkeyn){continue;} ;MKEYS[e.button.button]=2;}
      ef (e.type == SDL_MOUSEBUTTONUP){   if(e.button.button>= mkeyn){continue;} ;MKEYS[e.button.button]=0;}
   }
}


V drwBtn(Btn* b){
   SDL_Rect dst = {b->B.x, b->B.y, b->B.w, 30};
   if(inBox(mx, my, (*b).B)){ 
      if(MKEYS[1]){ SDL_RenderCopy(rend, b->txP, NULL, &dst);
      }else{ SDL_RenderCopy(rend, b->txH, NULL, &dst); }
   }else{ SDL_RenderCopy(rend, b->tx, NULL, &dst); }
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
   drwBtn(btn1);
   drwBtn(btn2);
   drwBtn(btn3);
   drwBtn(btn4);
   SDL_RenderPresent(rend);
   SDL_DestroyTexture( ScTx);
}



///alien ai///////////////////////////



//////////////////////////////////////

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
      t++;
      generateNoiseSignal();}

   btn1 = newBtn((Box){40, 300, 30, 30}, "res/btn.png", "res/btnH.png","res/btnP.png", setDrwCol1);
   btn2 = newBtn((Box){80, 300, 30, 30}, "res/btn.png", "res/btnH.png","res/btnP.png", setDrwCol2);
   btn3 = newBtn((Box){120, 300, 30, 30}, "res/btn.png", "res/btnH.png","res/btnP.png", setDrwCol3);
   btn4 = newBtn((Box){160, 300, 30, 30}, "res/btn.png", "res/btnH.png","res/btnP.png", setDrwCol4);
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
