#include "lut.h"
#include "Util/rng.h"
#include <stdlib.h>
#include <cds/Vector.h>
#include <math.h>

// Auxiliary to perform K-S test for a given flag, sample size and bins count
void aux_ks_test(int flag, uint32_t N, uint32_t nBins)
{
	init_rng(flag);
		
	// Fill the bins
	uint32_t bins[nBins];
	for (uint32_t i=0; i<nBins; i++)
		bins[i] = 0;
	for (uint32_t i=0; i<N; i++)
	{
		Real rf = get_rand_real();
		uint32_t index = floor(rf*nBins);
		if (index >= nBins) index = nBins - 1; //in case of...
		bins[index]++;
	}
	
	// Test the bins
	double ksThreshold = 1.358 / sqrt((double)N);
	double countPerBin = (double)N / nBins;
	uint32_t cumulativeSum = 0;
	for (uint32_t i=0; i<nBins; i++)
	{
		cumulativeSum += bins[i];
		LUT_ASSERT((double)cumulativeSum / N - (i+1)*countPerBin/N < ksThreshold);
	}
}

// Kolmogorov-Smirnov test on random real numbers (flag==0)
void t_rng1()
{
	aux_ks_test(0, 1000000, 1000);
	aux_ks_test(0, 100000, 1000);
	aux_ks_test(0, 10000, 100);
}

// Kolmogorov-Smirnov test on random real numbers (flag==1)
void t_rng2()
{
	aux_ks_test(1, 1000000, 1000);
	aux_ks_test(1, 100000, 1000);
	aux_ks_test(1, 10000, 100);
}
