#include <math.h>
#include <stdint.h>
typedef  void V ;
typedef  float_t F;
typedef int I ;
typedef uint32_t UI;
F VOLUME;
F FREQ;
V sinePlayerTick(){

}
V sinePlayerFreq(UI col){
   //r=nhz
   //g=(n+1oct)hz
   //b=(n+2oct)hz

}
V sinePlayerVolume(F v){
   if(v>1){v=1;} if(v<0){v=0;}
   VOLUME=v;
}
V sinePlayerInit(){
  sinePlayerVolume(.5);
  sinePlayerFreq(0);
}
