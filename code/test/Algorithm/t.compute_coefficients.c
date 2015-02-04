#include "Algorithm/compute_coefficients.h"
#include "Util/types.h"
#include "lut.h"
#include <math.h>
#include <stdlib.h>

void t_compute_coefficients1()
{	
	uint32_t nbSeries = 3;
	uint32_t nbValues = 10;
	Real curves[] =
	{
		1.0,2.0,3.0,4.0,1.0,2.0,3.0,4.0,1.0,2.0,
		3.0,4.0,1.0,2.0,3.0,4.0,1.0,1.0,2.0,3.0,
		3.0,1.0,0.0,8.0,2.0,4.0,2.0,0.0,3.0,3.0
	};

	PowerCurve* powerCurves = (PowerCurve*)malloc(nbSeries*sizeof(PowerCurve));
	for (int i=0; i<nbSeries; i++)
	{
		powerCurves[i].ID = i;
		powerCurves[i].values = curves + i * nbValues;
	}
	
	// Expected energy matrix: (to change...)
	// 1.2829311 2.8197073 2.7653092 0.5390868
	// 1.433019 3.319643 2.078569 1.248209
	// 1.4532363 6.1433038 5.2052217 0.3447528
	
	uint32_t nbReducedCoordinates = 4; //2^4 > 10, 2^3 < 10
	Real* reducedCoordinates = (Real*)malloc(nbSeries * nbReducedCoordinates * sizeof(Real));
	Real epsilon = 1e-4;
	compute_coefficients(powerCurves, nbSeries, nbValues, reducedCoordinates, 0, nbReducedCoordinates);
	free(powerCurves);

	//~ LUT_ASSERT(fabs(reducedCoordinates[0] - 1.2829311) < epsilon);
	//~ LUT_ASSERT(fabs(reducedCoordinates[1] - 2.8197073) < epsilon);
	//~ LUT_ASSERT(fabs(reducedCoordinates[2] - 2.7653092) < epsilon);
	//~ LUT_ASSERT(fabs(reducedCoordinates[3] - 0.5390868) < epsilon);
//~ 
	//~ LUT_ASSERT(fabs(reducedCoordinates[4] - 1.433019) < epsilon);
	//~ LUT_ASSERT(fabs(reducedCoordinates[5] - 3.319643) < epsilon);
	//~ LUT_ASSERT(fabs(reducedCoordinates[6] - 2.078569) < epsilon);
	//~ LUT_ASSERT(fabs(reducedCoordinates[7] - 1.248209) < epsilon);
//~ 
	//~ LUT_ASSERT(fabs(reducedCoordinates[8] - 1.4532363) < epsilon);
	//~ LUT_ASSERT(fabs(reducedCoordinates[9] - 6.1433038) < epsilon);
	//~ LUT_ASSERT(fabs(reducedCoordinates[10] - 5.2052217) < epsilon);
	//~ LUT_ASSERT(fabs(reducedCoordinates[11] - 0.3447528) < epsilon);
	
	free(reducedCoordinates);
}

void t_compute_coefficients2()
{	
	uint32_t nbSeries = 3;
	uint32_t nbValues = 20;
	Real curves[] =
	{
		0.07291223,0.3468863,0.8648239,0.2348877,0.4315312,0.7036144,0.2431289,0.01040031,0.1178515,0.03080914,0.3673594,0.3738354,0.6695186,0.9140619,0.3102869,0.6374112,0.843919,0.2661967,0.1571974,0.7748992,
		0.8818654,0.6703627,0.8847847,0.03184918,0.997876,0.7612137,0.6387965,0.296034,0.5018912,0.7942868,0.1095461,0.3727642,0.2351644,0.5057783,0.7287164,0.340264,0.08904832,0.5050351,0.1371493,0.8821361,
		0.1844872,0.5861217,0.7114864,0.8779052,0.5999996,0.2707162,0.9586406,0.6902478,0.2514943,0.8113025,0.2820882,0.5661663,0.9571422,0.6838519,0.998652,0.6126693,0.9178886,0.7810725,0.7895782,0.181061
	};

	PowerCurve* powerCurves = (PowerCurve*)malloc(nbSeries*sizeof(PowerCurve));
	for (int i=0; i<nbSeries; i++)
	{
		powerCurves[i].ID = i;
		powerCurves[i].values = curves + i * nbValues;
	}
	
	//~ // Expected energy matrix:
	//~ // ...TODO
	
	uint32_t nbReducedCoordinates = 5; //2^5 > 20, 2^4 < 20
	Real* reducedCoordinates = (Real*)malloc(nbSeries * nbReducedCoordinates * sizeof(Real));
	Real epsilon = 1e-4;
	compute_coefficients(powerCurves, nbSeries, nbValues, reducedCoordinates, 0, nbReducedCoordinates);
	free(powerCurves);

	//~ LUT_ASSERT(fabs(reducedCoordinates[0] - 1.2829311) < epsilon);
	//~ LUT_ASSERT(fabs(reducedCoordinates[1] - 2.8197073) < epsilon);
	//~ LUT_ASSERT(fabs(reducedCoordinates[2] - 2.7653092) < epsilon);
	//~ LUT_ASSERT(fabs(reducedCoordinates[3] - 0.5390868) < epsilon);
	//~ LUT_ASSERT(fabs(reducedCoordinates[4] - 0.5390868) < epsilon);
//~ 
	//~ LUT_ASSERT(fabs(reducedCoordinates[5] - 3.319643) < epsilon);
	//~ LUT_ASSERT(fabs(reducedCoordinates[6] - 2.078569) < epsilon);
	//~ LUT_ASSERT(fabs(reducedCoordinates[7] - 1.248209) < epsilon);
	//~ LUT_ASSERT(fabs(reducedCoordinates[8] - 1.4532363) < epsilon);
	//~ LUT_ASSERT(fabs(reducedCoordinates[9] - 6.1433038) < epsilon);
//~ 
	//~ LUT_ASSERT(fabs(reducedCoordinates[10] - 5.2052217) < epsilon);
	//~ LUT_ASSERT(fabs(reducedCoordinates[11] - 0.3447528) < epsilon);
	//~ LUT_ASSERT(fabs(reducedCoordinates[12] - 0.3447528) < epsilon);
	//~ LUT_ASSERT(fabs(reducedCoordinates[13] - 1.248209) < epsilon);
	//~ LUT_ASSERT(fabs(reducedCoordinates[14] - 1.248209) < epsilon);

	free(reducedCoordinates);
}
