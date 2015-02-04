// ISAAC PRNG
// Initial code by Bob Jenkins, March 1996 - modified in 2008
// Reference: http://burtleburtle.net/bob/rand/isaacafa.html
// Further slightly modified by Benjamin Auder, 2013

#include "Util/rng.h"
#include <stdlib.h>
#include <time.h>

// Internal state
static uint32_t mm[256];
static uint32_t aa, bb, cc;

// initialize the (pseudo-)random generator
void init_rng(int flag)
{
	// 'bootstrap' isaac with basic bad PRNG
	srand(time(NULL));

	aa = rand() & 0xffffffff;
	bb = rand() & 0xffffffff;
	cc = rand() & 0xffffffff;

	uint32_t a, b, c, d, e, f, g, h, i;
	// The golden ratio:
	a = b = c = d = e = f = g = h = 0x9e3779b9;
	// Scramble it
	for (i = 0; i < 4; ++i)
		mix(a, b, c, d, e, f, g, h);

	// Fill in mm[] with messy stuff (the seed)
	uint32_t seedArray[256];
	for (i = 0; i < 256; ++i) mm[i] = seedArray[i] = rand() & 0xffffffff;
	for (i = 0; i < 256; i += 8)
	{
		if (flag)
		{
			// Use all the information in the seed
			a += seedArray[i  ];
			b += seedArray[i + 1];
			c += seedArray[i + 2];
			d += seedArray[i + 3];
			e += seedArray[i + 4];
			f += seedArray[i + 5];
			g += seedArray[i + 6];
			h += seedArray[i + 7];
		}
		mix(a, b, c, d, e, f, g, h);
		mm[i  ] = a;
		mm[i + 1] = b;
		mm[i + 2] = c;
		mm[i + 3] = d;
		mm[i + 4] = e;
		mm[i + 5] = f;
		mm[i + 6] = g;
		mm[i + 7] = h;
	}

	if (flag)
	{
		// Do a second pass to make all of the seed affect all of mm
		for (i = 0; i < 256; i += 8)
		{
			a += mm[i  ];
			b += mm[i + 1];
			c += mm[i + 2];
			d += mm[i + 3];
			e += mm[i + 4];
			f += mm[i + 5];
			g += mm[i + 6];
			h += mm[i + 7];
			mix(a, b, c, d, e, f, g, h);
			mm[i  ] = a;
			mm[i + 1] = b;
			mm[i + 2] = c;
			mm[i + 3] = d;
			mm[i + 4] = e;
			mm[i + 5] = f;
			mm[i + 6] = g;
			mm[i + 7] = h;
		}
	}
}

// return a (pseudo-)random integer
uint32_t get_rand_int()
{
	// TODO: register variables ? (x,y,i)
	uint32_t x, y;
	static uint32_t i = 0;

	if (i == 0)
	{
		cc = cc + 1;  // cc just gets incremented once per 256 results
		bb = bb + cc; // then combined with bb
	}

	x = mm[i];
	switch (i % 4)
	{
	case 0:
		aa = aa^(aa << 13);
		break;
	case 1:
		aa = aa^(aa >> 6);
		break;
	case 2:
		aa = aa^(aa << 2);
		break;
	case 3:
		aa = aa^(aa >> 16);
		break;
	}

	// NOTE: bits 2..9 are chosen from x but 10..17 are chosen
	//       from y.  The only important thing here is that 2..9 and 10..17
	//       don't overlap.  2..9 and 10..17 were then chosen for speed in
	//       the optimized version (rand.c) */
	// See http://burtleburtle.net/bob/rand/isaac.html
	// for further explanations and analysis.

	aa        = mm[(i + 128) % 256] + aa;
	mm[i] = y = mm[(x >> 2) % 256]  + aa + bb;
	bb        = mm[(y >> 10) % 256] + x;

	i = (i + 1) % 256;

	return bb;
}

// return a (pseudo-)random real number in [0,1]
Real get_rand_real()
{
	return (Real) (INV_RANDMAX * (double) get_rand_int());
}
