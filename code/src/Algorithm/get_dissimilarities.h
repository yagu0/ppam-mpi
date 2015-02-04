#ifndef PPAM_GET_DISSIMILARITIES_H
#define PPAM_GET_DISSIMILARITIES_H

#include "Util/types.h"

// compute L^p dissimilarities for a nxm matrix
Real* get_dissimilarities_intra(Real* samples, uint32_t nbSamples, uint32_t nbValues, uint32_t p);

// compute L^p dissimilarities between rows of 2 matrices
Real* get_dissimilarities_inter(Real* mat1, uint32_t n1, Real* mat2, uint32_t n2, 
	uint32_t nbValues, uint32_t p);

#endif
