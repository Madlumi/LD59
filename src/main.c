//main.c===============================================
#include <SDL2/SDL_rect.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <time.h>
#include "mnoise.h"
#include "msine.h"
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
//flags
#define printfps 1
#define printLog 1
//macros

typedef float  F;
typedef int    I;
typedef Uint32 UI;
typedef void   V;
typedef double D;
#define R return
#define ef else if
typedef struct {I x,y,w,h;} Box;
typedef struct _disp{Box B; SDL_Surface* sf; V (*fnc)(struct _disp* , Uint32* );} disp;
typedef struct {Box B; SDL_Texture* tx; SDL_Texture* txH;SDL_Texture* txP; V (*fnc)(V); I status;} Btn;
//TEXTURES=====================================================================================================================
//TEXTURES=====================================================================================================================
//TEXTURES=====================================================================================================================
enum txIdx {
    TXspace,
    TXbg,
    TXfg,
    TXmeter,
    TXhullmeter,
    TXquestion,
    TXcross,
    TXrocket,
    TXrocketshade,
    TXVol,TXVolpeg,TXVolpegL,
    TXbook0,
    TX_COUNT
};
typedef struct {
    char path[128];
    SDL_Texture *tx;
} txture;
txture txs[TX_COUNT] = {
    [TXspace] = { "res/screen/space180x120.png", NULL },
    [TXbg] = { "res/ld59bg.png", NULL },
    [TXfg] = { "res/ld59fg.png", NULL },
    [TXmeter] = { "res/meter.png", NULL },
    [TXhullmeter] = { "res/hullmeter.png", NULL },
    [TXquestion] = { "res/qestion.png", NULL },
    [TXcross] = { "res/crosshair.png", NULL },
    [TXrocket] = { "res/rocket.png", NULL },
    [TXrocketshade] = { "res/rocketshade.png", NULL },
    [TXVol] = { "res/volume.png", NULL },
    [TXVolpeg] = { "res/volumePeg.png", NULL },
    [TXVolpegL] = { "res/volumePegLift.png", NULL },
    [TXbook0] = { "res/book/ld59book_0000.png", NULL },
};
//Enums===========================================================================================================================================
//Enums===========================================================================================================================================
//Enums===========================================================================================================================================
typedef enum {
   BLACK =  0xFF000000,
   RED   = 0xFFFF94B2,
   GREEN = 0xFF3ADE19,
   BLUE = 0xFFFFFB2B,
   PINK  = 0xFFB048F9,
} sigCol;

typedef enum {
   Hi, Question, Ignore,
   Refuel,  Rearm, Figth,
   Happy,   Angry, Hostile,
   Back_off, Agree, Disagree,
   Msg_COUNT
} Msg;
//structs=============================================
typedef struct {
   sigCol in[4]; 
   Msg    m;
} MsgCode;
//declares============================================
V reset();
I inBox(I x, I y, Box B);
V printStats();
V addFuel();
V rmFuel();
V takeDmg();
I msgKnown(MsgCode *book, Msg m);
V alienLeave();
V newAlien();
V alienAttack();
V playSoundError( );
V quit();
V drwMainDisp(disp* d, Uint32* p);
V drwSignalDisp(disp* d, Uint32* p);
V drawDisp(disp* d, Uint32* p);
V setDrwCol(I c);
V setDrwCol0();
V setDrwCol1();
V setDrwCol2();
V setDrwCol3();
V setDrwCol4();
V generateNoiseSignal();
I sigMod(I i);
V readSignalbuffer();
V tick(F dt);
V events();
V drwBtn(Btn* b);
V render();
V mainLoop();
V OPTflyAway();
V OPTtrade();
V OPTfight();
V initBTN();
I init();
I main();

//arrays=============================================
MsgCode alienTalkStandard[Msg_COUNT] = {
   [Hi]        = { { BLACK, BLACK, BLACK, BLACK }, Hi       },
   [Question]  = { { RED,   BLUE,  GREEN, PINK  }, Question },
   [Ignore]    = { { BLACK, BLACK, BLACK, BLACK }, Ignore   },
   [Refuel]    = { { BLACK, BLACK, BLACK, BLACK }, Ignore   },
   [Rearm]     = { { BLACK, BLACK, BLACK, BLACK }, Ignore   },
   [Figth]     = { { BLACK, BLACK, BLACK, BLACK }, Ignore   },
   [Happy]     = { { BLACK, BLACK, BLACK, BLACK }, Ignore   },
   [Angry]     = { { BLACK, BLACK, BLACK, BLACK }, Ignore   },
   [Hostile]   = { { BLACK, BLACK, BLACK, BLACK }, Ignore   },
   [Back_off]  = { { BLACK, BLACK, BLACK, BLACK }, Ignore   },
   [Agree]     = { { BLACK, BLACK, BLACK, BLACK }, Ignore   },
   [Disagree]  = { { BLACK, BLACK, BLACK, BLACK }, Ignore   },
};

MsgCode alienTalk1[Msg_COUNT] = {
   [Hi]        = { { BLACK, BLACK, BLACK, BLACK }, Ignore    },
   [Question]  = { { BLACK, BLACK, BLACK, BLACK }, Ignore    },
   [Ignore]    = { { BLACK, BLACK, BLACK, BLACK }, Ignore    },
   [Refuel]    = { { RED,   PINK,  BLUE,  GREEN }, Refuel    },
   [Rearm]     = { { GREEN, RED,   PINK,  BLUE  }, Rearm     },
   [Figth]     = { { BLUE,  GREEN, RED,   PINK  }, Figth     },
   [Happy]     = { { PINK,  BLUE,  GREEN, RED   }, Happy     },
   [Angry]     = { { RED,   GREEN, PINK,  BLUE  }, Angry     },
   [Hostile]   = { { BLUE,  PINK,  RED,   GREEN }, Hostile   },
   [Back_off]  = { { BLACK, BLACK, BLACK, BLACK }, Ignore    },
   [Agree]     = { { GREEN, BLUE,  RED,   PINK  }, Agree     },
   [Disagree]  = { { PINK,  RED,   BLUE,  GREEN }, Disagree  },
};

MsgCode alienTalk2[Msg_COUNT] = {
   [Hi]        = { { RED,   BLUE,  PINK,  GREEN }, Hi        },
   [Question]  = { { BLACK, BLACK, BLACK, BLACK }, Ignore    },
   [Ignore]    = { { BLACK, BLACK, BLACK, BLACK }, Ignore    },
   [Refuel]    = { { GREEN, PINK,  RED,   BLUE  }, Refuel    },
   [Rearm]     = { { BLACK, BLACK, BLACK, BLACK }, Ignore    },
   [Figth]     = { { PINK,  GREEN, BLUE,  RED   }, Figth     },
   [Happy]     = { { BLACK, BLACK, BLACK, BLACK }, Ignore    },
   [Angry]     = { { BLACK, BLACK, BLACK, BLACK }, Ignore    },
   [Hostile]   = { { BLUE,  RED,   GREEN, PINK  }, Hostile   },
   [Back_off]  = { { RED,   GREEN, BLUE,  PINK  }, Back_off  },
   [Agree]     = { { GREEN, BLUE,  PINK,  RED   }, Agree     },
   [Disagree]  = { { PINK,  RED,   GREEN, BLUE  }, Disagree  },
};

MsgCode alienTalk3[Msg_COUNT] = {
   [Hi]        = { { GREEN, BLUE, GREEN, GREEN }, Hi        },
   [Question]  = { { BLACK, BLACK, BLACK, BLACK }, Ignore    },
   [Ignore]    = { { BLACK, BLACK, BLACK, BLACK }, Ignore    },
   [Refuel]    = { { RED,   PINK,  GREEN, BLUE  }, Refuel    },
   [Rearm]     = { { BLACK, BLACK, BLACK, BLACK }, Ignore    },
   [Figth]     = { { BLACK, BLACK, BLACK, BLACK }, Ignore    },
   [Happy]     = { { GREEN, RED,   BLUE,  PINK  }, Happy     },
   [Angry]     = { { PINK,  BLUE,  RED,   GREEN }, Angry     },
   [Hostile]   = { { BLACK, BLACK, BLACK, BLACK }, Ignore    },
   [Back_off]  = { { BLUE,  PINK,  GREEN, RED   }, Back_off  },
   [Agree]     = { { RED,   GREEN, PINK,  BLUE  }, Agree     },
   [Disagree]  = { { GREEN, BLUE,  RED,   PINK  }, Disagree  },
};

MsgCode alienTalk4[Msg_COUNT] = {
   [Hi]        = { { PINK,  RED,   GREEN, BLUE  }, Hi        },
   [Question]  = { { BLACK, BLACK, BLACK, BLACK }, Ignore    },
   [Ignore]    = { { BLACK, BLACK, BLACK, BLACK }, Ignore    },
   [Refuel]    = { { BLACK, BLACK, BLACK, BLACK }, Ignore    },
   [Rearm]     = { { BLACK, BLACK, BLACK, BLACK }, Ignore    },
   [Figth]     = { { GREEN, RED,   PINK,  BLUE  }, Figth     },
   [Happy]     = { { BLACK, BLACK, BLACK, BLACK }, Ignore    },
   [Angry]     = { { RED,   BLUE,  GREEN, PINK  }, Angry     },
   [Hostile]   = { { BLUE,  GREEN, RED,   PINK  }, Hostile   },
   [Back_off]  = { { PINK,  GREEN, BLUE,  RED   }, Back_off  },
   [Agree]     = { { GREEN, PINK,  RED,   BLUE  }, Agree     },
   [Disagree]  = { { BLUE,  RED,   PINK,  GREEN }, Disagree  },
};

//declares============================================
#if printfps
static F fpsTimer = 0;
static I fpsFrames = 0;
#endif
#define W 948
#define H 533
#define PS W*H
#define BGNoiseStr .1
I alienX, alienY;
I running;
I hp; I fuel; I fuelMax=10, hpMax=10, ammo, ammoMax=3;
F alienCd=3;
V reset(){
   hp=hpMax;
   
   ammo=ammoMax;
   fuel=fuelMax*.7;
   alienCd=3;
}




I mx; I my; //mouse
#define mkeyn 12 
I MKEYS[mkeyn];

SDL_Window     *wind;
SDL_Renderer   *rend;
SDL_Surface    *surf;
disp* disp1;
disp* disp2;
#define MX_BTNS 7
Btn* btns[MX_BTNS];
I AlresponseDelay=20;
I AlsilentCount;

F RwarpPowNoise=0;
F GwarpPowNoise=0;
F BwarpPowNoise=0;
#define  warpDecay .95;
#define  signalLogW 450
Uint32 signalLog[signalLogW];
F signalPwrLog[signalLogW];
Uint32 signalLogA[signalLogW];
F signalPwrLogA[signalLogW];
Uint32 snoiseLog[signalLogW];
F signalLogPoint=0;

//helpers
I inBox(I x, I y, Box B){
   if(x<B.x || y< B.y){return 0;}
   if(x-B.w > B.x || y-B.h> B.y){return 0;}

   return 1;
}

I actTaken[Msg_COUNT];





F mood= 0;
I alienType=2;
V printStats(){
   if(!printLog){return;}
   printf("fuel: %d\n", fuel);
   printf("hp:   %d\n", fuel);
   printf("Mood: %.2f\n", mood);
   printf("alien: %d\n",  alienType);
}
V addFuel(){
   fuel++;
   if(fuel>fuelMax){fuel=fuelMax;}
}
V rmFuel(){
   fuel--;
   if(fuel<0){fuel=0;printf("outta fuel");}
}
V takeDmg(){
   hp-=1;
   if(hp<0){hp=0;printf("outta hp");}
}
MsgCode *getAlienBook(void){
   if(alienType==0){ return alienTalk1; }
   if(alienType==1){ return alienTalk2; }
   if(alienType==2){ return alienTalk3; }
   return alienTalk4;
}
I msgKnown(MsgCode *book, Msg m){
   MsgCode c = book[m];
   return !(c.in[0]==BLACK && c.in[1]==BLACK && c.in[2]==BLACK && c.in[3]==BLACK);
}

Msg decodeAlienMsg(sigCol in[4]){
   MsgCode *book = getAlienBook();
   I i;

   for(i=0; i<Msg_COUNT; i++){
      if( book[i].in[0]==in[0] && book[i].in[1]==in[1] && book[i].in[2]==in[2] && book[i].in[3]==in[3]){ return (Msg)i; }
   }
   return Question;
}
I alienShow=0;
I alienRefueled=0;
V alienLeave(){
   F warpPowNoise=1.5;
   alienShow=0;
}
V newAlien(){
   alienType=rand()%4;
   RwarpPowNoise=((rand()%10)*.1+.7);
   GwarpPowNoise=((rand()%10)*.1+.7);
   BwarpPowNoise=((rand()%10)*.1+.7);
   mood=rand()%100*.01;
   for(I i= 0; i <Msg_COUNT;i++){ actTaken[i]=0; }
   alienShow=1;
   alienX=(rand()%140+20)*2;
   alienY=(rand()%60+30)*2;

}
V alienAttack(){
   alienLeave();
   takeDmg();
}
V playSoundError( ){}



MsgCode alienInterpret(sigCol in[4]){
   Msg m = decodeAlienMsg(in);
   MsgCode *book = getAlienBook();

   if(!msgKnown(book, m)){ return alienTalkStandard[Question]; }

   I taken = actTaken[m];
   actTaken[m] = 1;
   #define one if(!taken)
   #define two if(taken)
   if(alienType==0){
      if(m==Refuel      ) { return book[Agree]; }
      if(m==Hi          ) { return book[Agree]; } 
      if(m==Question    ) { return book[Agree]; }
      if(m==Ignore      ) { return book[Agree]; }
      if(m==Refuel      ) { return book[Agree]; }
      if(m==Rearm       ) { return book[Agree]; }
      if(m==Figth       ) { return book[Agree]; }
      if(m==Happy       ) { return book[Agree]; }
      if(m==Angry       ) { return book[Agree]; }
      if(m==Hostile     ) { return book[Agree]; }
      if(m==Back_off    ) { return book[Agree]; }
      if(m==Agree       ) { return book[Agree]; }
      if(m==Disagree    ) { return book[Agree]; }
   }
   if(alienType==1){
      if(m==Refuel      ) { return book[Agree]; }
      if(m==Hi          ) { return book[Agree]; } 
      if(m==Question    ) { return book[Agree]; }
      if(m==Ignore      ) { return book[Agree]; }
      if(m==Refuel      ) { return book[Agree]; }
      if(m==Rearm       ) { return book[Agree]; }
      if(m==Figth       ) { return book[Agree]; }
      if(m==Happy       ) { return book[Agree]; }
      if(m==Angry       ) { return book[Agree]; }
      if(m==Hostile     ) { return book[Agree]; }
      if(m==Back_off    ) { return book[Agree]; }
      if(m==Agree       ) { return book[Agree]; }
      if(m==Disagree    ) { return book[Agree]; }
   }

   if(alienType==2){
      if(m==Hi          ) {one{mood+=.2;} return book[Hi];  } 
      if(m==Refuel      ) {one{ }  return mood>.7 ? book[Agree] : book[Disagree]; }
      if(m==Happy       ) {one{if(mood>.5){mood += .2;}} return mood>.7 ? book[Happy] : book[Disagree]; }
      if(m==Angry       ) {one{if(mood<.5){mood -= .2;}} return mood<.4 ? book[Angry] : book[Disagree]; }
      if(m==Back_off    ) {one{printf("awo\n");} two{alienAttack();}mood=0; return book[Back_off]; }
      if(m==Agree       ) {one{} return book[Question]; }
      if(m==Disagree    ) {one{} return book[Question]; }
   }
   if(alienType==3){
      if(m==Refuel      ) { return book[Agree]; }
      if(m==Hi          ) { return book[Agree]; } 
      if(m==Question    ) { return book[Agree]; }
      if(m==Ignore      ) { return book[Agree]; }
      if(m==Refuel      ) { return book[Agree]; }
      if(m==Rearm       ) { return book[Agree]; }
      if(m==Figth       ) { return book[Agree]; }
      if(m==Happy       ) { return book[Agree]; }
      if(m==Angry       ) { return book[Agree]; }
      if(m==Hostile     ) { return book[Agree]; }
      if(m==Back_off    ) { return book[Agree]; }
      if(m==Agree       ) { return book[Agree]; }
      if(m==Disagree    ) { return book[Agree]; }
   }

   return book[m];
}



V quit(){ printf("quiting...\n");sinePlayerQuit(); running=0;SDL_Quit(); printf("quit\n"); }
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
   if(f!=NULL){ b->fnc=f; }
   b->status=0;
   return b;
}
typedef  enum {
   MT_MIX,
   MT_ADD,
} mixType;

UI mixCol(UI c1, UI c2, F pow, mixType t){
   if(pow < 0) pow = 0;
   if(pow > 1) pow = 1;

   #define CH(c, s) (((c) >> (s)) & 0xFFu)
   #define CL(x) ((x) > 0xFFu ? 0xFFu : (x))

   UI a = t == MT_MIX ? (UI)(CH(c2,24) + (CH(c1,24) - CH(c2,24)) * pow) : CL(CH(c2,24) + (UI)(CH(c1,24) * pow));
   UI r = t == MT_MIX ? (UI)(CH(c2,16) + (CH(c1,16) - CH(c2,16)) * pow) : CL(CH(c2,16) + (UI)(CH(c1,16) * pow));
   UI g = t == MT_MIX ? (UI)(CH(c2, 8) + (CH(c1, 8) - CH(c2, 8)) * pow) : CL(CH(c2, 8) + (UI)(CH(c1, 8) * pow));
   UI b = t == MT_MIX ? (UI)(CH(c2, 0) + (CH(c1, 0) - CH(c2, 0)) * pow) : CL(CH(c2, 0) + (UI)(CH(c1, 0) * pow));

   #undef CH
   #undef CL
   return (a << 24) | (r << 16) | (g << 8) | b;
}
//draw funcs===============================================

V drwMainDisp(disp* d, Uint32* p){
   SDL_Rect dst= {(*d).B.x,(*d).B.y,(*d).B.w,(*d).B.h};
   SDL_RenderCopy(rend, txs[TXspace].tx, NULL, &dst);
}
#define  h d->B.h
#define  w d->B.w
#define  xo d->B.x
#define  yo d->B.y
F t = 0;
#define SigDispCol mixCol(  mixCol(mixCol(signalLog[pnt], 0xFF000000, signalPwrLog[pnt], MT_MIX),mixCol(signalLogA[pnt], 0xFF000000, signalPwrLogA[pnt], MT_MIX),1,MT_ADD) , snoiseLog[pnt], 1,MT_ADD) 
V drwSignalDisp(disp* d, Uint32* p){
   for(I y= 0; y< h; y++) {
      for(I x= 0; x< w; x++) {

         I idx = (x+xo)+(y+yo)*W;
         if(idx>=PS || idx < 0){continue;}
         I noise=rand()%2 + (I)(2.0*rnd((F)(y+t*126)*.1)-1);
         I pnt=((x+noise+(I)signalLogPoint)%signalLogW);
         p[idx]= SigDispCol;
         if(x==w-5){ sinePlayerFreq(SigDispCol);}
      }
   }
}
V drawDisp(disp* d, Uint32* p){
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
}
   #undef w
   #undef h
I drwCol;
V setDrwCol(I c){ drwCol=c; }
V setDrwCol0(){ drwCol=BLACK; }
V setDrwCol1(){ drwCol=RED; }
V setDrwCol2(){ drwCol=GREEN; }
V setDrwCol3(){ drwCol=BLUE; }
V setDrwCol4(){ drwCol=PINK; }
//=============================================================================
F acc=0;

#define sigIdx ((I)signalLogPoint-1)% signalLogW
#define sigPrevIdx ((I)signalLogPoint-2)% signalLogW
V generateNoiseSignal(){
      signalLogPoint+=1;
      snoiseLog[((I)signalLogPoint-1)% signalLogW]=0xFF000000;
      I r =(I)(Perlin1D(t*2.5+13277)*0xFF)&0xFF;
      I g =(I)(Perlin1D(t*2+37731)*0xFF)&0xFF;
      I b =(I)(Perlin1D(t*2.3+12567)*0xFF)&0xFF;
      I noise = 0xFF000000 | (r << 16) | (g << 8) | b;
      F noiseStr=Perlin1D(t*115)*BGNoiseStr;
      snoiseLog[((I)signalLogPoint-1)% signalLogW]= mixCol(noise, 0xFF000000, noiseStr, MT_MIX);
      snoiseLog[((I)signalLogPoint-1)% signalLogW]= mixCol( 0xFFFF0000, snoiseLog[((I)signalLogPoint-1)% signalLogW], RwarpPowNoise, MT_ADD);
      snoiseLog[((I)signalLogPoint-1)% signalLogW]= mixCol( 0xFF00FF00, snoiseLog[((I)signalLogPoint-1)% signalLogW], GwarpPowNoise, MT_ADD);
      snoiseLog[((I)signalLogPoint-1)% signalLogW]= mixCol( 0xFF0000FF, snoiseLog[((I)signalLogPoint-1)% signalLogW], BwarpPowNoise, MT_ADD);
}

sigCol   resp[4];
I        respCnt=4;
I        respSpeed=0;
I        respGap=0;
F        respProg=0;
I sigMod(I i){
   i %= signalLogW;
   if(i < 0){ i += signalLogW; }
   return i;
}
V readSignalbuffer(){
   I last=0;
   I cnt=0;
    sigCol msg[4];
   for(I i = 0; i < signalLogW ; i++){
      if(cnt>=4){break;}
      I c = signalLog[sigMod(sigIdx - i)];
      if(c!=last){ 
         if(c==RED){   msg[3-cnt]=c; cnt++; }
         if(c==GREEN){ msg[3-cnt]=c;cnt++; }
         if(c==BLUE){  msg[3-cnt]=c; cnt++;}
         if(c==PINK){  msg[3-cnt]=c; cnt++;}
         last=c;
      }
   }
   MsgCode m = alienInterpret(msg);
   for(I i = 0; i < cnt ; i++){
      I c = msg[i];
      if(c==RED){    printf("R\n"); }
      if(c==GREEN){  printf("G\n"); }
      if(c==BLUE){   printf("B\n"); }
      if(c==PINK){   printf("P\n"); }
   }

   for (I i = 0; i < 4; i++) {
      resp[i] = m.in[i];
   }
   printStats();
   respCnt=0;
   respSpeed=5;
   respGap=3;
   respProg=0;
   for(I i = 0; i < cnt ; i++){
      I c = resp[i];
      if(c==RED){    printf("R\n"); }
      if(c==GREEN){  printf("G\n"); }
      if(c==BLUE){   printf("B\n"); }
      if(c==PINK){   printf("P\n"); }
   }
   
}
I new = 0;
I newReady=0;
F pressPwr=0;
I last;
V tick(F dt){
   if(alienCd>=0){alienCd-=dt;if(alienCd<=0.01){alienCd=-1 ; newAlien();}}
   acc+=dt*30;
   t+=dt;
   if(!alienShow){new=0;}
   if(MKEYS[1]==2){
      for(I i = 0; i < MX_BTNS; i++){
           if(inBox(mx, my, (*btns[i]).B)){btns[i]->fnc();}
      }
   }
   if(MKEYS[1]){
      if(newReady){ signalLog[sigPrevIdx] = 0;}
      if(inBox(mx, my, (*btns[1]).B)){btns[1]->fnc(); pressPwr+=dt*16;if(last!=1 || newReady){ new+=1;newReady=0;last=1;} }
      ef(inBox(mx, my, (*btns[2]).B)){btns[2]->fnc(); pressPwr+=dt*16;if(last!=2 || newReady){ new+=1;newReady=0;last=2;} }
      ef(inBox(mx, my, (*btns[3]).B)){btns[3]->fnc(); pressPwr+=dt*16;if(last!=3 || newReady){ new+=1;newReady=0;last=3;} }
      ef(inBox(mx, my, (*btns[4]).B)){btns[4]->fnc(); pressPwr+=dt*16;if(last!=4 || newReady){ new+=1;newReady=0;last=4;} }
      if(pressPwr>1){pressPwr=1;}
   }else{ pressPwr-=dt*7; if(pressPwr<.1){pressPwr=0;newReady=1;} }
   while(acc > 0){
      RwarpPowNoise*=warpDecay;
      GwarpPowNoise*=warpDecay;
      BwarpPowNoise*=warpDecay;
      generateNoiseSignal();
      signalLog[sigIdx]=0;
      signalLogA[sigIdx]=0;

      if(pressPwr>.01){signalLog[sigIdx]=drwCol; signalPwrLog[sigIdx]=pressPwr;}
      if(respCnt<4) { 
         if(respProg<respSpeed){signalLogA[sigIdx]=resp[respCnt]; signalPwrLogA[sigIdx]=1;}
         respProg+=1;
         if(respProg>respSpeed+respGap){respCnt+=1; respProg=0;}

      }
      if(signalLog[sigIdx]==0){AlsilentCount++;}else{AlsilentCount=0;}

      if(AlsilentCount>AlresponseDelay&& new>=4){ new=0; readSignalbuffer();AlsilentCount=0;}
      ef(AlsilentCount>AlresponseDelay*1.5){ new=0; AlsilentCount=0;}
      acc-=1;
   }

   if(!MKEYS[1]){ newReady=1; }
   /*VOLUME*///weee hardcode af volume bar!
   /*VOLUME*/if(MKEYS[1] && mx>210 && mx < (220+375+15) && abs(my-(280))<15 ){ sinePlayerVolume(((F)mx-220.0)/375.0); } 
   
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
   SDL_Rect dst = {b->B.x, b->B.y, b->B.w, b->B.h};
   if(inBox(mx, my, (*b).B)){ 
      if(MKEYS[1]){ SDL_RenderCopy(rend, b->txP, NULL, &dst);
      }else{ SDL_RenderCopy(rend, b->txH, NULL, &dst); }
   }else{ SDL_RenderCopy(rend, b->tx, NULL, &dst); }
}
V render(){

   sinePlayerTick();
   if (SDL_MUSTLOCK(surf)) SDL_LockSurface(surf);
   Uint32 * p = surf->pixels;

   for(I y= 0; y< H; y++) {
      for(I x= 0; x< W; x++) {
         p[x+y*W]=0x00000000;
      }
   }

   drawDisp(disp2, p);
   if (SDL_MUSTLOCK(surf)) SDL_UnlockSurface(surf);

   SDL_Texture *ScTx = SDL_CreateTextureFromSurface(rend, surf);//draw this on top alpha cut

   SDL_RenderClear(rend);
   //background
   SDL_Rect dst = {0, 0, W, H};
   SDL_RenderCopy(rend, txs[TXbg].tx, NULL, &dst);
   SDL_RenderCopy(rend, ScTx, NULL, NULL);
   drawDisp(disp1, p);


   /*AMMO*/
   I ammoX=695;
   I ammoY=210;
   I ammoD=110;
   for(I i = 0; i < ammo;i++){
      SDL_Rect rckdst= {ammoX,ammoY-ammoD*i,300,250};
   SDL_RenderCopy(rend, txs[TXrocketshade].tx, NULL, &rckdst);
   }
   for(I i = 0; i < ammo;i++){
      SDL_Rect rckdst= {ammoX,ammoY-ammoD*i,300,250};
      SDL_RenderCopy(rend, txs[TXrocket].tx, NULL, &rckdst);
   }
   /*AMMO*/
   for(I i = 0; i < MX_BTNS; i++){
      drwBtn(btns[i]);
   }
   SDL_Rect bookdst = {-30, 170, 300, 300};

   /*meter   */ float PER=1-((F)fuel/(F)fuelMax);
   /*meter   */ I meterH=190;
   /*meter   */ SDL_Rect meterdst= {600, 50+meterH*PER, 100, 190-meterH*PER};
   /*meter   */ SDL_Rect metersrc= {0, 0+meterH*PER, 100, 190-meterH*PER};
   /*meter   */ SDL_RenderCopy(rend, txs[TXmeter].tx, &metersrc, &meterdst);
   /*CROSSHAIR*/ if(alienShow){
  /*CROSSHAIR*/ SDL_Rect clip = {245, 30, 360, 240}, crossdst = {245 + alienX - 352, 30 + alienY - 59, 421, 283};
  /*CROSSHAIR*/ SDL_RenderSetClipRect(rend, &clip); 
  /*CROSSHAIR*/ SDL_RenderCopy(rend, txs[TXcross].tx, NULL, &crossdst); 
  /*CROSSHAIR*/ SDL_RenderSetClipRect(rend, NULL);
   /*CROSSHAIR*/}
   //

   SDL_RenderCopy(rend, txs[TXfg].tx, NULL, &dst);//FG=====================================
   /*VOLUME*/ SDL_Rect voldst= {200, 250, 450, 50};  SDL_RenderCopy(rend, txs[TXVol].tx, NULL, &voldst);
   /*VOLUME*/ I PEGDST=6;
   /*VOLUME*/ for(I i = 0; i < (375.0/PEGDST)*usrVOLUME;i++){ SDL_Rect volPdst= {220+i*PEGDST, 275, 11, 15}; 
      if(abs(mx-(220+i*PEGDST))<15&&abs(my-(280))<15){SDL_RenderCopy(rend, txs[TXVolpegL].tx, NULL, &volPdst);}else{ SDL_RenderCopy(rend, txs[TXVolpeg].tx, NULL, &volPdst); }}

  /*Question */ SDL_Rect quedst = {430, 220, 200, 50};
  /*Question */ if(alienShow){SDL_RenderCopy(rend, txs[TXquestion].tx, NULL, &quedst);}
   //hull meter
   float hPER=((F)hp/(F)hpMax);
   I hullmeterw=135;
   SDL_Rect hmeterdst= {248, 230, hullmeterw*hPER, 40};
   SDL_Rect hmetersrc= {0,   0, hullmeterw*hPER, 40};
   SDL_RenderCopy(rend, txs[TXhullmeter].tx, &hmetersrc, &hmeterdst);
   //

   SDL_RenderCopy(rend, txs[TXbook0].tx, NULL, &bookdst);
   drwBtn(btns[1]);
   //****************TODO DELETE SHITTY SCREENSHOT CODE******************************''
   //****************TODO DELETE SHITTY SCREENSHOT CODE******************************''
   //****************TODO DELETE SHITTY SCREENSHOT CODE******************************''
   if(MKEYS[2]==2){
      time_t t = time(NULL);
      struct tm *tmv = localtime(&t);
      if(!tmv){ printf("localtime fail\n"); return; }

      char path[64];
      strftime(path, sizeof(path), "shot_%H%M%S.png", tmv);

      SDL_Surface *shot = SDL_CreateRGBSurfaceWithFormat(0, W, H, 32, SDL_PIXELFORMAT_ARGB8888);
      if(!shot){ printf("screenshot surface fail: %s\n", SDL_GetError()); return; }

      if(SDL_RenderReadPixels(rend, NULL, SDL_PIXELFORMAT_ARGB8888, shot->pixels, shot->pitch) != 0){
         printf("SDL_RenderReadPixels fail: %s\n", SDL_GetError());
         SDL_FreeSurface(shot);
         return;
      }

      if(IMG_SavePNG(shot, path) != 0){
         printf("IMG_SavePNG fail: %s\n", IMG_GetError());
      }else{
         printf("saved %s\n", path);
      }

      SDL_FreeSurface(shot);
   }
   //****************TODO DELETE SHITTY SCREENSHOT CODE******************************''
   //****************TODO DELETE SHITTY SCREENSHOT CODE******************************''
   //****************TODO DELETE SHITTY SCREENSHOT CODE******************************''
   SDL_RenderPresent(rend);
   SDL_DestroyTexture( ScTx);
}


V mainLoop(){
   static Uint64 last = 0;
   Uint64 now = SDL_GetPerformanceCounter();
   if(last == 0){ last = now; }

   F dt = (F)(now - last) / (F)SDL_GetPerformanceFrequency();
   last = now;

   events();
   tick(dt);
   render();

#if printfps
   fpsTimer += dt;
   fpsFrames += 1;
   if(fpsTimer >= 1.0f){
      printf("fps: %d\n", fpsFrames);
      fpsFrames = 0;
      fpsTimer -= 1.0f;
   }
#endif
}


V OPTflyAway(){
   //if (mood<.5){takeDmg();}
   rmFuel();
   alienLeave();
   RwarpPowNoise=.7;
   GwarpPowNoise=2;
   BwarpPowNoise=1.5;
   alienCd=rand()%10;
   printStats();

}
V OPTtrade(){
   if(alienRefueled){return; playSoundError();}
   alienRefueled=1;
   if (mood>.65){addFuel();}
   else {takeDmg();}
   if (mood<.5){takeDmg();}
   RwarpPowNoise=.3;
   GwarpPowNoise=.6;
   BwarpPowNoise=.4;
   printStats();
}

V OPTfight(){
   if(!alienShow){return;}
   alienLeave();
   ammo--;
   RwarpPowNoise=3;
   GwarpPowNoise=.4;
   BwarpPowNoise=8;
   printStats();
}
V initBTN(){
   int btnH = 75;
   int btnW = 85;

   int signalBtnsX = 210;
   int signalBtnY = 370;
   int signalBtnXOff = 120;

   int diagBtnsX = 660;
   int diagBtnsY = 285;
   int diagBtnXOff = 70;
   int diagBtnYOff = 77;

   btns[1] = newBtn((Box){signalBtnsX + signalBtnXOff * 0, signalBtnY, btnW, btnH}, "res/btns/btnRN.png", "res/btns/btnRH.png", "res/btns/btnRP.png", setDrwCol1);
   btns[2] = newBtn((Box){signalBtnsX + signalBtnXOff * 1, signalBtnY, btnW, btnH}, "res/btns/btnGN.png", "res/btns/btnGH.png", "res/btns/btnGP.png", setDrwCol2);
   btns[3] = newBtn((Box){signalBtnsX + signalBtnXOff * 2, signalBtnY, btnW, btnH}, "res/btns/btnBN.png", "res/btns/btnBH.png", "res/btns/btnBP.png", setDrwCol3);
   btns[4] = newBtn((Box){signalBtnsX + signalBtnXOff * 3, signalBtnY, btnW, btnH}, "res/btns/btnPN.png", "res/btns/btnPH.png", "res/btns/btnPP.png", setDrwCol4);

   btns[0] = newBtn((Box){diagBtnsX + diagBtnXOff * 0, diagBtnsY + diagBtnYOff * 0, btnW, btnH}, "res/btns/btnN.png", "res/btns/btnH.png", "res/btns/btnP.png", OPTtrade);
   btns[5] = newBtn((Box){diagBtnsX + diagBtnXOff * 1, diagBtnsY + diagBtnYOff * 1, btnW, btnH}, "res/btns/btnN.png", "res/btns/btnH.png", "res/btns/btnP.png", OPTflyAway);
   btns[6] = newBtn((Box){diagBtnsX + diagBtnXOff * 2, diagBtnsY + diagBtnYOff * 2, btnW, btnH}, "res/btns/btnN.png", "res/btns/btnH.png", "res/btns/btnP.png", OPTfight);

}
I init(){
   running=1;
   reset();
   //display
   disp1 = newDisp((Box){245, 30, 360 , 240}, drwMainDisp);
   //signal scope
   disp2 = newDisp((Box){200, 290, signalLogW, 60}, drwSignalDisp);

   //init empty signal
   for(I i = 0; i < signalLogW; i++){ t++; generateNoiseSignal();}

   initBTN();


   for (int i = 0; i < TX_COUNT; i++) {
      txs[i].tx = IMG_LoadTexture(rend, txs[i].path);
      if (!txs[i].tx) { printf("IMG_LoadTexture failed: %s\n", IMG_GetError()); return 0;}
   }
   printf("init'd\n");

   SDL_Init(SDL_INIT_AUDIO);
   sinePlayerInit();

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
   const F target = 1.0f / 60.0f;

   while(running){
      Uint64 frameStart = SDL_GetPerformanceCounter();

      mainLoop();

      Uint64 frameEnd = SDL_GetPerformanceCounter();
      F frameTime = (F)(frameEnd - frameStart) / (F)SDL_GetPerformanceFrequency();

      if(frameTime < target){
         SDL_Delay((Uint32)((target - frameTime) * 1000.0f));
      }

   }
#endif 
return 0;

}
//eof main.c============================================
