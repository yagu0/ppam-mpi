#include "lut.h"
#include <stdlib.h>
#include "MPI_Communication/pack.h"
#include "Util/types.h"
#include <stdint.h>
#include "Util/utils.h"

// Work_t
void t_pack1()
{
	Work_t* work = (Work_t*)malloc(sizeof(Work_t));
	work->inputFileName = "../data/inputTest.bin";
	work->nbSeries = 5;
	uint32_t ranks[] =
	{
		0, 
		12*65536 + 3*256+11,
		32*16777216 + 2*65536 + 17*256 + 42, 
		123,
		51*16777216 + 77*65536 + 0*256 + 10
	};
	work->ranks = ranks;
	work->nbClusters = 15;
	work->clustOnMedoids = 0;
	work->p_for_dissims = 1;
	
	// Hard-coded expected packed work
	unsigned char expectedPackedWork[] =
	{
		// --> ../data/inputTest.bin
		46,46,47,100,97,116,97,47,105,110,112,117,116,84,101,115,116,46,98,105,110,
		// continue with 256 - strlen("../data/inputTest.bin") zeros...
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		// --> 5, {ranks}+0,0, 15, 0 on 4-bytes integers little-endian
		5,0,0,0, 0,0,0,0, 11,3,12,0, 42,17,2,32, 123,0,0,0, 10,0,77,51, 0,0,0,0, 0,0,0,0, 
		15,0,0,0, 0,0,0,0, 1,0,0,0
	};
	
	uint32_t nbSeriesInChunk = 7;
	uint32_t work_message_length = get_packedWork_length(nbSeriesInChunk);
	unsigned char packedWork[work_message_length];
	pack_work(work, nbSeriesInChunk, packedWork);
	
	for (uint32_t i=0; i<work_message_length; i++)
	{
		LUT_ASSERT(packedWork[i] == expectedPackedWork[i]);
	}
	
	free(work);
}

// Result_t
void t_pack2() {
	
	Result_t* result = (Result_t*)malloc(sizeof(Result_t));
	uint32_t nbClusters = result->nbClusters = 5;
	uint32_t medoids_ID[] =
	{
		15*65536 + 13*256+11,
		11, 
		17*16777216 + 0*65536 + 14*256 + 42, 
		123*65536 + 0*256 + 7,
		51*16777216 + 0*65536 + 0*256 + 10
	};
	result->medoids_ID = medoids_ID;
	uint32_t medoids_ranks[] =
	{
		5*65536 + 32,
		11*256 + 4, 
		23*16777216 + 42, 
		35*65536 + 5*256 + 77,
		1*16777216 + 1*65536 + 1*256 + 10
	};
	result->medoids_ranks = medoids_ranks;
	
	// Hard-coded expected result
	unsigned char expectedPackedResult[] =
	{
		// 5, {medoids_ID}, {medoids_ranks}
		5,0,0,0,
		11,13,15,0, 11,0,0,0, 42,14,0,17, 7,0,123,0, 10,0,0,51,
		32,0,5,0, 4,11,0,0, 42,0,0,23, 77,5,35,0, 10,1,1,1
	};
	
	uint32_t result_message_length = get_packedResult_length(nbClusters);
	unsigned char packedResult[result_message_length];
	pack_result(result, packedResult);
	
	for (uint32_t i=0; i<result_message_length; i++)
	{
		LUT_ASSERT(packedResult[i] == expectedPackedResult[i]);
	}
	
	free(result);
}
