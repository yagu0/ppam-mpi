#ifndef PPAM_COMPUTE_COEFFICIENTS_H
#define PPAM_COMPUTE_COEFFICIENTS_H

#include "Util/types.h"

// compute rows of the matrix of reduced coordinates (see computeCoefficients.R)
void compute_coefficients(PowerCurve* powerCurves, uint32_t nbSeries, uint32_t nbValues,
	Real* reducedCoordinates, uint32_t index, uint32_t nbReducedCoordinates);

#endif
