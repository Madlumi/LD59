#include <SDL2/SDL_audio.h>
#include <math.h>
#include <stdint.h>
typedef  void V ;
typedef  float_t F;
typedef int I ;
typedef uint32_t UI;

static SDL_AudioDeviceID asDev;
static SDL_AudioSpec as;
F VOLUME;
F usrVOLUME;
F midVol=1;
F maxVol=3;

/* cool notes
   C2 =  65.41f
   E2 =  82.41f
   A2 = 110.00f
   C3 = 130.81f
   E3 = 164.81f
   A3 = 220.00f
   C4 = 261.63f
   */
//c octaves
F Rfreq = 130.81f;
F Gfreq = 261.63f;
F Bfreq = 523.25f;
//5thOctave
//F Rfreq = 130.81f;
//F Gfreq = 392.00f;
//F Bfreq = 587.33f;
//F Rfreq = 65.41f;
//F Gfreq = 196.00f;
//F Bfreq = 293.66f;

F Rpow=0, Gpow=0, Bpow=0;
F RpowT=0, GpowT=0, BpowT=0;

F Roff=0;
F Goff=0;
F Boff=0;

#define AChan 2
#define ABUFF 256
#define AMAXBUFF  sizeof(F) * ABUFF * 8 * AChan
#define L i*2+0
#define R i*2+1
F smooth = 0.0015f;
static inline F softclip(F x){
    return x / (1.0f + fabsf(x));
}
V sinePlayerTick(){
   if(!asDev){ return; }
   while( SDL_GetQueuedAudioSize(asDev) < AMAXBUFF){
      F buf[ABUFF*AChan];
      F Rstep =(/*full step*/ 2.0f * (F)M_PI )* (Rfreq / (F)as.freq);
      F Gstep =(/*full step*/ 2.0f * (F)M_PI )* (Gfreq / (F)as.freq);
      F Bstep =(/*full step*/ 2.0f * (F)M_PI )* (Bfreq / (F)as.freq);
      for(I i = 0; i < ABUFF; i++){
         Rpow += (RpowT - Rpow) * smooth;
         Gpow += (GpowT - Gpow) * smooth;
         Bpow += (BpowT - Bpow) * smooth;
         F r=sinf(Roff)*Rpow*(VOLUME*.333);
         F g=sinf(Goff)*Gpow*(VOLUME*.333);
         F b=sinf(Boff)*Bpow*(VOLUME*.333);
         buf[L]=r*.33+(g*.5)+b*.66;
         buf[R]=r*.66+(g*.5)+b*.33;
         Roff+=Rstep; Goff+=Gstep; Boff+=Bstep;
         if(Roff > 2.0f * (F)M_PI){ Roff -= 2.0f * (F)M_PI; }
         if(Goff > 2.0f * (F)M_PI){ Goff -= 2.0f * (F)M_PI; }
         if(Boff > 2.0f * (F)M_PI){ Boff -= 2.0f * (F)M_PI; }
         buf[L] = softclip(buf[L]);
         buf[R] = softclip(buf[R]);
      }

      //send buffer to sdl and it plays
      SDL_QueueAudio(asDev, buf, sizeof(buf));
   }
}
#undef L
#undef R
V sinePlayerFreq(UI col){
   RpowT = (F)((col >> 16) & 255) / 255.0f * 2;
   GpowT = (F)((col >>  8) & 255) / 255.0f * 1.5;
   BpowT = (F)((col >>  0) & 255) / 255.0f * .75 ;
}
V sinePlayerVolume(F v){
   if(v>1){v=1;} if(v<0){v=0;}
   usrVOLUME=v;
   VOLUME=usrVOLUME*maxVol;
   VOLUME = maxVol * pow(usrVOLUME, log(midVol/maxVol)/log(0.5));
   //uv=0 =>   v=0
   //uv=.5 =>  v=midVol
   //uv=1.0 => v=maxVol
}
V sinePlayerInit(){
   sinePlayerVolume(.4);
   sinePlayerFreq(0xFF000000);
   SDL_AudioSpec asTarg= {0};
   asTarg.freq=48000;
   asTarg.format=AUDIO_F32SYS;
   asTarg.samples=512;
   asTarg.channels=AChan;
   asDev = SDL_OpenAudioDevice(NULL, 0, &asTarg, &as, 0);
   if(!asDev){ return; }
   SDL_PauseAudioDevice(asDev, 0);
}
V sinePlayerQuit(){
   if(asDev){ SDL_CloseAudioDevice(asDev); asDev = 0; }
}
