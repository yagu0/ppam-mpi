#include "lut.h"
#include "Util/utils.h"
#include <stdlib.h>
#include <stdio.h>
#include "Util/types.h"
#include <math.h>

//integers
void t_utils1()
{
	FILE* file = fopen("../data/test/integers.txt", "rb");
	int64_t x;
	readInt(file, &x);
	LUT_ASSERT(x == 1234);
	readInt(file, &x);
	LUT_ASSERT(x == -987);
	readInt(file, &x);
	LUT_ASSERT(x == 654321);
	readInt(file, &x);
	LUT_ASSERT(x == 12);
	readInt(file, &x);
	LUT_ASSERT(x == 345678);
	readInt(file, &x);
	LUT_ASSERT(x == -1234);
	fclose(file);
}

//reals
void t_utils2()
{
	FILE* file = fopen("../data/test/reals.txt", "rb");
	Real x, tol = 1e-10;
	readReal(file, &x);
	LUT_ASSERT(fabs(x - 1234.056) < tol);
	readReal(file, &x);
	LUT_ASSERT(fabs(x - 987e-3) < tol);
	readReal(file, &x);
	LUT_ASSERT(fabs(x - -654321e-2) < tol);
	readReal(file, &x);
	LUT_ASSERT(fabs(x - 12.34567) < tol);
	readReal(file, &x);
	LUT_ASSERT(fabs(x - 345678.0) < tol);
	readReal(file, &x);
	LUT_ASSERT(fabs(x - -1234.05) < tol);
	readReal(file, &x);
	LUT_ASSERT(fabs(x - 3e-1) < tol);
	readReal(file, &x);
	LUT_ASSERT(fabs(x - -1.0188) < tol);
	fclose(file);
}
