#include <stdlib.h>
#include "Util/types.h"
#include <math.h>
#include <gsl/gsl_wavelet.h>
#include <gsl/gsl_spline.h>

// compute rows of the matrix of reduced coordinates
void compute_coefficients(PowerCurve* powerCurves, uint32_t nbSeries, uint32_t nbValues,
	Real* reducedCoordinates, uint32_t index, uint32_t nbReducedCoordinates)
{
	uint32_t D = (1 << nbReducedCoordinates);
	Real* x = (Real*) malloc(nbValues*sizeof(Real));
	for (uint32_t i=0; i<nbValues; i++)
		x[i] = i;
	Real* y = (Real*) malloc(nbValues*sizeof(Real));
	Real* interpolatedTranformedCurve = (Real*) malloc(D*sizeof(Real));
	gsl_interp* linearInterpolation = gsl_interp_alloc(gsl_interp_linear, nbValues);
	gsl_interp_accel* acc = gsl_interp_accel_alloc();
	gsl_wavelet_workspace* work = gsl_wavelet_workspace_alloc(D);
	//gsl_wavelet* w = gsl_wavelet_alloc(gsl_wavelet_bspline, 206); //used for starlight
	gsl_wavelet* w = gsl_wavelet_alloc(gsl_wavelet_daubechies, 6); //used for power curves
	//gsl_wavelet* w = gsl_wavelet_alloc(gsl_wavelet_haar, 2);
	for (uint32_t i = 0; i < nbSeries; i++)
	{
		//Spline interpolation to have D = 2^u sample points
		for (uint32_t j=0; j<nbValues; j++)
			y[j] = powerCurves[i].values[j];
		gsl_interp_init(linearInterpolation, x, y, nbValues);
		for (uint32_t j=0; j<D; j++)
		{
			interpolatedTranformedCurve[j] = 
				gsl_interp_eval(linearInterpolation, x, y, j*((Real)(nbValues-1)/(D-1)), acc);
		}
		//DWT transform (in place) on interpolated curve [TODO: clarify stride parameter]
		gsl_wavelet_transform_forward(w, interpolatedTranformedCurve, 1, D, work);
		//Fill reducedCoordinates with energy contributions
		uint32_t t0 = 1;
		uint32_t t1 = 1;
		for (uint32_t j=0; j<nbReducedCoordinates; j++) 
		{
			t1 += (1 << j);
			//reducedCoordinates[(index+i)*nbReducedCoordinates+j] = sqrt( sum( x[t0:t1]^2 ) )
			Real sumOnSegment = 0.0;
			for (uint32_t u=t0; u<t1; u++)
				sumOnSegment += interpolatedTranformedCurve[u]*interpolatedTranformedCurve[u];
			reducedCoordinates[(index+i)*nbReducedCoordinates+j] = sqrt(sumOnSegment);
			t0 = t1;
		}
	}
	gsl_interp_free(linearInterpolation);
    gsl_interp_accel_free(acc);
	free(x);
	free(y);
	free(interpolatedTranformedCurve);
	gsl_wavelet_free(w);
	gsl_wavelet_workspace_free(work);
}
