#include "Algorithm/get_dissimilarities.h"
#include "TimeSeries/deserialize.h"
#include <math.h>
#include "Algorithm/compute_coefficients.h"
#include <string.h>
#include "Util/utils.h"

uint32_t* get_class(PowerCurve* data, uint32_t nbSeries, PowerCurve* medoids, 
	uint32_t nbClusters, uint32_t nbValues, uint32_t p_for_dissims, double* DISTOR)
{
	// nbReducedCoordinates = smallest power of 2 which is above nbValues
	uint32_t nbReducedCoordinates = (uint32_t)ceil(log2(nbValues));

	// Preprocessing to reduce dimension of both data and medoids
	Real* reducedCoordinates_data = (Real*) malloc(nbSeries * nbReducedCoordinates * sizeof(Real));
	compute_coefficients(data, nbSeries, nbValues, 
		reducedCoordinates_data, 0, nbReducedCoordinates);
	Real* reducedCoordinates_medoids = (Real*) malloc(nbClusters * nbReducedCoordinates * sizeof(Real));
	compute_coefficients(medoids, nbClusters, nbValues, 
		reducedCoordinates_medoids, 0, nbReducedCoordinates);
	
	Real* dissimilarities = get_dissimilarities_inter(reducedCoordinates_data, nbSeries, 
		reducedCoordinates_medoids, nbClusters, nbReducedCoordinates, p_for_dissims);
	free(reducedCoordinates_data);
	free(reducedCoordinates_medoids);
	
	// 3] Finally, assign each row to the least dissimilar center
	uint32_t* result = (uint32_t*) malloc(nbSeries*sizeof(uint32_t));
	for (uint32_t i=0; i<nbSeries; i++)
	{
		uint32_t minIndex = 0;
		Real minDissim = dissimilarities[i*nbClusters + 0];
		for (uint32_t j=1; j<nbClusters; j++)
		{
			if (dissimilarities[i*nbClusters + j] < minDissim)
			{
				minDissim = dissimilarities[i*nbClusters + j];
				minIndex = j;
			}
		}
		result[i] = minIndex + 1;
		(*DISTOR) += minDissim;
	}
	free(dissimilarities);
	return result;
}
