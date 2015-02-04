#include <stdlib.h>
#include "Util/types.h"
#include "Util/utils.h"

// deserialize a Work_t object from a bytes string
Work_t* unpack_work(Byte* packedWork, uint32_t nbSeriesInChunk)
{
	Work_t* work = (Work_t*) malloc(sizeof(Work_t));

	uint32_t index = 0;

	size_t inputFileNameSize = 0;
	while (packedWork[inputFileNameSize++] != 0) { }
	work->inputFileName = (char*) malloc(inputFileNameSize);
	while (index < inputFileNameSize)
	{
		work->inputFileName[index] = packedWork[index];
		index++;
	}

	index = NCHAR_FNAME;

	uint32_t nbSeries = work->nbSeries = bInt_to_uint(packedWork + index, 4);
	index += 4;

	work->ranks = (uint32_t*) malloc(nbSeries * sizeof(uint32_t));
	for (uint32_t i = 0; i < nbSeries; i++)
	{
		work->ranks[i] = bInt_to_uint(packedWork + index, 4);
		index += 4;
	}
	// shift over the zeros
	index += 4 * (nbSeriesInChunk - nbSeries);

	work->nbClusters = bInt_to_uint(packedWork + index, 4);
	index += 4;
	work->clustOnMedoids = bInt_to_uint(packedWork + index, 4);
	index += 4;
	work->p_for_dissims = bInt_to_uint(packedWork + index, 4);

	return work;
}

// deserialize a Result_t object from a bytes string
Result_t* unpack_result(Byte* packedResult)
{
	Result_t* result = (Result_t*) malloc(sizeof(Result_t));
	uint32_t index = 0;

	uint32_t nbClusters = result->nbClusters = bInt_to_uint(packedResult, 4);
	index += 4;

	result->medoids_ID = (uint32_t*) malloc(nbClusters * sizeof(uint32_t));
	for (uint32_t i = 0; i < nbClusters; i++)
	{
		result->medoids_ID[i] = bInt_to_uint(packedResult + index, 4);
		index += 4;
	}

	result->medoids_ranks = (uint32_t*) malloc(nbClusters * sizeof(uint32_t));
	for (uint32_t i = 0; i < nbClusters; i++)
	{
		result->medoids_ranks[i] = bInt_to_uint(packedResult + index, 4);
		index += 4;
	}

	return result;
}
