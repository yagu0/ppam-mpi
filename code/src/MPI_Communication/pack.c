#include <stdlib.h>
#include "Util/types.h"
#include "Util/utils.h"

// serialize a Work_t object into a bytes string
void pack_work(Work_t* work, uint32_t nbSeriesInChunk, Byte* packedWork)
{
	uint32_t index = 0;

	while (work->inputFileName[index] != 0)
	{
		packedWork[index] = work->inputFileName[index];
		index++;
	}
	// complete with zeros
	while (index < NCHAR_FNAME)
		packedWork[index++] = 0;

	write_int(work->nbSeries, 4, packedWork + index);
	index += 4;

	for (uint32_t i = 0; i < work->nbSeries; i++)
	{
		write_int(work->ranks[i], 4, packedWork + index);
		index += 4;
	}
	// complete with zeros
	for (uint32_t i = 0; i < nbSeriesInChunk - work->nbSeries; i++)
	{
		write_int(0, 4, packedWork + index);
		index += 4;
	}

	write_int(work->nbClusters, 4, packedWork + index);
	index += 4;
	write_int(work->clustOnMedoids, 4, packedWork + index);
	index += 4;
	write_int(work->p_for_dissims, 4, packedWork + index);
}

// serialize a Result_t object into a bytes string
void pack_result(Result_t* result, Byte* packedResult)
{
	uint32_t index = 0;

	write_int(result->nbClusters, 4, packedResult);
	index += 4;

	for (uint32_t i = 0; i < result->nbClusters; i++)
	{
		write_int(result->medoids_ID[i], 4, packedResult + index);
		index += 4;
	}

	for (uint32_t i = 0; i < result->nbClusters; i++)
	{
		write_int(result->medoids_ranks[i], 4, packedResult + index);
		index += 4;
	}
}
