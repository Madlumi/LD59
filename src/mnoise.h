
#include <math.h>
#include <stdint.h>

typedef float F;
typedef int I;



F rnd(I x) {
    uint32_t z = (uint32_t)x;
    z ^= z >> 16;
    z *= 0x7feb352dU;
    z ^= z >> 15;
    z *= 0x846ca68bU;
    z ^= z >> 16;
    return (F)z / 4294967296.0f;
}

F Perlin1D(F x) {
   I x0 = (I)floorf(x);
    I x1 = x0 + 1;
    F prog = x - (F)x0;
    return rnd(x0) * (1.0f - prog) + rnd(x1) * prog;
}
