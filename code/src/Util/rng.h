#ifndef PPAM_RNG_H
#define PPAM_RNG_H

#include <stdint.h>
#include "Util/types.h"

#define INV_RANDMAX 2.3283064370807974E-10 // 1/0xffffffff

// If (flag!=0), then use a random array to initialize mm[]
#define mix(a,b,c,d,e,f,g,h)  \
{                             \
	a^=b<<11; d+=a; b+=c;     \
	b^=c>>2;  e+=b; c+=d;     \
	c^=d<<8;  f+=c; d+=e;     \
	d^=e>>16; g+=d; e+=f;     \
	e^=f<<10; h+=e; f+=g;     \
	f^=g>>4;  a+=f; g+=h;     \
	g^=h<<8;  b+=g; h+=a;     \
	h^=a>>9;  c+=h; a+=b;     \
}

// initialize the (pseudo-)random generator
// NOTE: flag has only two states, active or not.
//       flag != 0 --> "maximal randomness"
void init_rng(int flag);
 
// return a (pseudo-)random integer
uint32_t get_rand_int();

// return a (pseudo-)random real number in [0,1]
Real get_rand_real();

#endif
