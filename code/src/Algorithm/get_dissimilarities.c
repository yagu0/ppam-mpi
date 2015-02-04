#include "Util/types.h"
#include <stdlib.h>
#include <math.h>

// compute L^p dissimilarities for a nxm matrix
Real* get_dissimilarities_intra(Real* samples, uint32_t nbSamples, uint32_t nbValues, uint32_t p)
{
	Real* dissimilarities = (Real*) malloc(nbSamples*nbSamples*sizeof(Real));
	for (uint32_t i=0; i<nbSamples; i++)
	{
		dissimilarities[i*nbSamples+i] = 0.0;
		for (uint32_t j=0; j<i; j++)
		{
			// dissimilarities[i*nbSamples+j] = L^p distance between reduced rows i and j
			double dissim = 0.0;
			for (uint32_t m=0; m<nbValues; m++)
			{
				double delta = fabs(samples[i*nbValues+m] - samples[j*nbValues+m]);
				dissim += pow(delta, p);
			}
			dissimilarities[i*nbSamples+j] = pow(dissim, 1.0/p);
			dissimilarities[j*nbSamples+i] = dissimilarities[i*nbSamples+j];
		}
	}
	return dissimilarities;
}

// compute L^p dissimilarities between rows of 2 matrices
Real* get_dissimilarities_inter(Real* mat1, uint32_t n1, Real* mat2, uint32_t n2, 
	uint32_t nbValues, uint32_t p)
{
	Real* dissimilarities = (Real*) malloc(n1*n2*sizeof(Real));
	for (uint32_t i=0; i<n1; i++)
	{
		for (uint32_t j=0; j<n2; j++)
		{
			double dissim = 0.0;
			for (uint32_t m=0; m<nbValues; m++)
			{
				double delta = fabs(mat1[i*nbValues+m] - mat2[j*nbValues+m]);
				dissim += pow(delta, p);
			}
			dissimilarities[i*n2+j] = pow(dissim, 1.0/p);
		}
	}
	return dissimilarities;
}
