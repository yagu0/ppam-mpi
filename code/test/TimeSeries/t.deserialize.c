#include "TimeSeries/deserialize.h"
#include "lut.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "Util/utils.h"

void t_deserialize1()
{
	// decode sample_byCols.bin into the toy dataset (4 customers, 3 sample points)
	const char* ifName = "../data/test/sample_byCols.bin";
	
	// check 'header'
	uint32_t nbSeries = get_nbSeries(ifName);
	LUT_ASSERT(nbSeries == 4);
	uint32_t tsLength = get_tsLength(ifName);
	LUT_ASSERT(tsLength == 13); //3*3+4
	
	uint32_t ranks[] =
	{
		0, 2, 1, 3
	};
	PowerCurve* powerCurves = deserialize(ifName, NULL, ranks, nbSeries);

	Real epsilon = 0.1; //epsilon = 0.1 because raw powers are truncated
	
	LUT_ASSERT(powerCurves[0].ID == 12301);
	LUT_ASSERT(fabs(powerCurves[0].values[0] - 23.051) < epsilon);
	LUT_ASSERT(fabs(powerCurves[0].values[1] - 33.052) < epsilon);
	LUT_ASSERT(fabs(powerCurves[0].values[2] - 43.053) < epsilon);
	
	LUT_ASSERT(powerCurves[1].ID == 1313);
	LUT_ASSERT(fabs(powerCurves[1].values[0] - 50.05) < epsilon);
	LUT_ASSERT(fabs(powerCurves[1].values[1] - 51.05) < epsilon);
	LUT_ASSERT(fabs(powerCurves[1].values[2] - 52.05) < epsilon);
	
	LUT_ASSERT(powerCurves[2].ID == 50000);
	LUT_ASSERT(fabs(powerCurves[2].values[0] - 150.321) < epsilon);
	LUT_ASSERT(fabs(powerCurves[2].values[1] - 160.322) < epsilon);
	LUT_ASSERT(fabs(powerCurves[2].values[2] - 140.323) < epsilon);

	LUT_ASSERT(powerCurves[3].ID == 6300);
	LUT_ASSERT(fabs(powerCurves[3].values[0] - 500.30) < epsilon);
	LUT_ASSERT(fabs(powerCurves[3].values[1] - 501.31) < epsilon);
	LUT_ASSERT(fabs(powerCurves[3].values[2] - 502.32) < epsilon);

	for (int i = 0; i < nbSeries; i++)
		free(powerCurves[i].values);
	free(powerCurves);
}

void t_deserialize2()
{
	// decode sample_byRows.bin into the toy dataset (4 customers, 3 sample points)
	const char* ifName = "../data/test/sample_byRows.bin";
	
	// check 'header'
	uint32_t nbSeries = get_nbSeries(ifName);
	LUT_ASSERT(nbSeries == 4);
	uint32_t tsLength = get_tsLength(ifName);
	LUT_ASSERT(tsLength == 13); //3*3+4
	
	uint32_t ranks[] =
	{
		0, 2, 1, 3
	};
	PowerCurve* powerCurves = deserialize(ifName, NULL, ranks, nbSeries);

	Real epsilon = 0.1; //epsilon = 0.1 because raw powers are truncated
	
	LUT_ASSERT(powerCurves[0].ID == 12301);
	LUT_ASSERT(fabs(powerCurves[0].values[0]/100.0-3 - 23.051) < epsilon);
	LUT_ASSERT(fabs(powerCurves[0].values[1]/100.0-3 - 33.052) < epsilon);
	LUT_ASSERT(fabs(powerCurves[0].values[2]/100.0-3 - 43.053) < epsilon);
	
	LUT_ASSERT(powerCurves[1].ID == 1313);
	LUT_ASSERT(fabs(powerCurves[1].values[0]/100.0-3 - 50.05) < epsilon);
	LUT_ASSERT(fabs(powerCurves[1].values[1]/100.0-3 - 51.05) < epsilon);
	LUT_ASSERT(fabs(powerCurves[1].values[2]/100.0-3 - 52.05) < epsilon);
	
	LUT_ASSERT(powerCurves[2].ID == 50000);
	LUT_ASSERT(fabs(powerCurves[2].values[0]/100.0-3 - 150.321) < epsilon);
	LUT_ASSERT(fabs(powerCurves[2].values[1]/100.0-3 - 160.322) < epsilon);
	LUT_ASSERT(fabs(powerCurves[2].values[2]/100.0-3 - 140.323) < epsilon);

	LUT_ASSERT(powerCurves[3].ID == 6300);
	LUT_ASSERT(fabs(powerCurves[3].values[0]/100.0-3 - 500.30) < epsilon);
	LUT_ASSERT(fabs(powerCurves[3].values[1]/100.0-3 - 501.31) < epsilon);
	LUT_ASSERT(fabs(powerCurves[3].values[2]/100.0-3 - 502.32) < epsilon);

	for (int i = 0; i < nbSeries; i++)
		free(powerCurves[i].values);
	free(powerCurves);
}
