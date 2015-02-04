#include "Util/types.h"
#include "Util/utils.h"
#include "TimeSeries/deserialize.h"
#include "Algorithm/compute_coefficients.h"
#include "Algorithm/get_dissimilarities.h"
#include "Algorithm/pam.h"
#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include "MPI_Communication/unpack.h"
#include "MPI_Communication/pack.h"

// main job done by a slave: 
static Result_t* do_work(Work_t* work)
{
	// *** Step 1 *** 
	// Obtain reduced coordinates matrix from time-series.
	
	uint32_t nbSeries = work->nbSeries;
	uint32_t tsLength = get_tsLength(work->inputFileName);
	uint32_t nbValues = (tsLength - 4) / 3;
	
	// nbReducedCoordinates = smallest power of 2 which is above nbValues
	uint32_t nbReducedCoordinates = (uint32_t)ceil(log2(nbValues));
	Real* reducedCoordinates = (Real*) malloc(nbSeries * nbReducedCoordinates * sizeof(Real));

	// call preprocessing with the rows of raw power values matrix.
	// Keep the IDs in memory for further processing.
	uint32_t* IDs = (uint32_t*)malloc(nbSeries*sizeof(uint32_t));
	for (uint32_t i = 0; i < nbSeries; i+=CURVES_PER_REQUEST)
	{
		uint32_t nbCurvesRetrieved = CURVES_PER_REQUEST;
		if (i + nbCurvesRetrieved > nbSeries) 
			nbCurvesRetrieved -= (i + nbCurvesRetrieved - nbSeries);
		PowerCurve* powerCurves = 
			deserialize(work->inputFileName, NULL, work->ranks + i, nbCurvesRetrieved);
		compute_coefficients(powerCurves, nbCurvesRetrieved, nbValues, 
			reducedCoordinates, i, nbReducedCoordinates);
		for (uint32_t ii=i; ii<i+nbCurvesRetrieved; ii++)
		{
			IDs[ii] = powerCurves[ii-i].ID;
			free(powerCurves[ii-i].values);
		}
		free(powerCurves);
	}

	// *** Step 2 ***
	// Run PAM algorithm on the dissimilarity matrix computed from 'reducedCoordinates'.
	
	Real* dissimilarities = get_dissimilarities_intra(
		reducedCoordinates, nbSeries, nbReducedCoordinates, work->p_for_dissims);
	free(reducedCoordinates);

	uint32_t nbClusters = work->nbClusters;
	Result_t* result = (Result_t*) malloc(sizeof(Result_t));
	result->medoids_ID = (uint32_t*) malloc(nbClusters * sizeof(uint32_t));
	result->medoids_ranks = (uint32_t*) malloc(nbClusters * sizeof(uint32_t));
	result->nbClusters = nbClusters;

	// Run PAM algorithm to fill result->medoids_ranks
	pam(dissimilarities, nbSeries, nbClusters, work->clustOnMedoids, NSTART, MAXITER, result);
	free(dissimilarities);
	
	// Deduce medoids_IDs from indices
	for (uint32_t i = 0; i < nbClusters; i++)
		result->medoids_ID[i] = IDs[result->medoids_ranks[i]];
	free(IDs);

	// return medoids IDs, indices and items labels (to be post-processed)
	return result;
}

// code executed by slave process
void slave_run(int myrank, uint32_t nbSeriesInChunk, uint32_t nbClusters)
{
	Work_t* work;
	Result_t* result;
	MPI_Status status;

	// Expected size of a Work message in bytes:
	uint32_t work_message_length = get_packedWork_length(nbSeriesInChunk);
	Byte packedWork[work_message_length];

	// Expected size of a Result message in bytes: (uint32_t is on 4 bytes)
	uint32_t result_message_length = get_packedResult_length(nbClusters);
	Byte packedResult[result_message_length];

	while (1)
	{
		// Receive a message from the master
		MPI_Recv(packedWork, work_message_length, MPI_BYTE, 0,
			MPI_ANY_TAG, MPI_COMM_WORLD, &status);

		// Check the tag of the received message.
		if (status.MPI_TAG == DIETAG)
			return;

		// Do the work
		work = unpack_work(packedWork, nbSeriesInChunk);
		fprintf(stdout, "%u / Slave pid=%u work on %s\n",myrank,getpid(),work->inputFileName);
		result = do_work(work);
		free_work(work);

		// Send the result back
		pack_result(result, packedResult);
		free_result(result);
		MPI_Send(packedResult, result_message_length, MPI_BYTE, 0, WORKTAG, MPI_COMM_WORLD);
	}
}
