#include "Util/types.h"
#include "Util/utils.h"
#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include "MPI_Communication/pack.h"
#include "MPI_Communication/unpack.h"
#include "Util/rng.h"

// save the final result in XML format
static void result_to_XML(Result_t* result, const char* inputFileName, 
	const char* lastBinaryFileName, uint32_t p_for_dissims)
{
	uint32_t nbClusters = result->nbClusters;
	FILE* ofile = fopen("ppamResult.xml", "w");
	
	fprintf(ofile, "<medoids>\n\n");

	fprintf(ofile, "  <file>");
	fprintf(ofile, "%s", lastBinaryFileName);
	fprintf(ofile, "</file>\n\n");

	fprintf(ofile, "  <p_for_dissims>");
	fprintf(ofile, "%u", p_for_dissims);
	fprintf(ofile, "</p_for_dissims>\n\n");
	
	fprintf(ofile, "  <IDs>\n");
	for (uint32_t i=0; i<nbClusters; i++)
		fprintf(ofile, "    <ID>%u</ID>\n", result->medoids_ID[i]);
	fprintf(ofile, "  </IDs>\n\n");

	// medoids ranks in last binary file (human printing: 0 --> 1 ...etc)
	fprintf(ofile, "  <ranks>\n");
	for (uint32_t i=0; i<nbClusters; i++)
		fprintf(ofile, "    <rank>%u</rank>\n", result->medoids_ranks[i] + 1);
	fprintf(ofile, "  </ranks>\n\n");

	fprintf(ofile, "</medoids>\n");
	fclose(ofile);
}

static void binaryResult_to_file(Result_t* result, const char* inputFileName,
	const char* outputFileName)
{
	// Determine tsLength from inputFile
	uint32_t tsLength = get_tsLength(inputFileName);

	FILE* ifile = fopen(inputFileName, "rb");
	FILE* ofile = fopen(outputFileName, "ab"); //'append binary'

	Byte tsBuffer[tsLength];
	for (uint32_t i = 0; i < result->nbClusters; i++)
	{
		// Get time-series in (binary) inputFile
		fseek(ifile, 8 + result->medoids_ranks[i] * tsLength, SEEK_SET);
		// Copy current serie onto ofile
		
		size_t lengthRead = fread(tsBuffer, 1, tsLength, ifile);
		if (lengthRead != tsLength)
			fprintf(stderr, "problem while copying binary series to new file.\n");
		fwrite(tsBuffer, 1, tsLength, ofile);
	}

	fclose(ifile);
	fclose(ofile);
}

// fill a new unit of work suitable to be given to a slave
static Work_t* get_next_work(char* inputFileName, uint32_t nbSeries, uint32_t nbSeriesInChunk,
	double idealNbSeriesInChunk, uint32_t jobsSentCount, uint32_t lastEndIndex,
	uint32_t nbClusters, uint32_t clustOnMedoids, 
	int randomize, uint32_t p_for_dissims)
{
	Work_t* work = (Work_t*) malloc(sizeof(Work_t));

	work->inputFileName = (char*) malloc(strlen(inputFileName) + 1);
	strcpy(work->inputFileName, inputFileName);

	if (randomize)
		work->nbSeries = nbSeriesInChunk;
	else
	{
		double adjustedNbSeriesInNextChunk =
			idealNbSeriesInChunk * (jobsSentCount + 1) - lastEndIndex;
		// round to closest integer
		work->nbSeries = (uint32_t)adjustedNbSeriesInNextChunk;
		// stay below the upper bound (TODO: is this check required ?)
		if (work->nbSeries > nbSeriesInChunk)
			work->nbSeries = nbSeriesInChunk;
		// TODO: what about this one ? (probably useless)
		if (lastEndIndex + work->nbSeries > nbSeries)
			work->nbSeries = nbSeries - lastEndIndex;
	}

	//TODO: ranks on uint64_t if more than 4.3 billion series at the same place (unlikely...)
	work->ranks = (uint32_t*) malloc(work->nbSeries * sizeof(uint32_t));
	for (uint32_t i = 0; i < work->nbSeries; i++)
		work->ranks[i] = (randomize ? get_rand_int() % nbSeries : lastEndIndex + i);

	work->nbClusters = nbClusters;
	work->clustOnMedoids = clustOnMedoids;
	work->p_for_dissims = p_for_dissims;

	return work;
}

// process all subtasks and save binary results into a new file
// NOTE: this file will be smaller than initial file (or DB...)
static void clusters_reduce(char* inputFileName, char* outputFileName, uint32_t ntasks,
	uint32_t totalNbSeries, uint32_t nbSeriesInChunk, double idealNbSeriesInChunk,
	uint32_t tsLength, uint32_t nbClusters, uint32_t clustOnMedoids, 
	int randomize, uint32_t p_for_dissims)
{
	FILE* ofile = fopen(outputFileName, "wb"); //'write binary'
	// Leave a blank for series' count and tsLength
	for (uint32_t i = 0; i < 8; i++)
		fputc(0, ofile);
	fclose(ofile);

	uint32_t jobsSentCount = 0; //used if randomize==FALSE
	uint32_t lastEndIndex = 0; //used if randomize==FALSE

	uint32_t sentSeriesCount = 0; //used if randomize==TRUE

	// Count series sent to binary file on output
	uint32_t newSeriesCount = 0;

	// Expected size of a Work message in bytes:
	uint32_t work_message_length = get_packedWork_length(nbSeriesInChunk);
	Byte packedWork[work_message_length];

	// Expected size of a Result message in bytes: (uint32_t is on 4 bytes)
	uint32_t result_message_length = 4 + 4 * nbClusters + 4 * nbClusters;
	Byte packedResult[result_message_length];

	// Seed the slaves; send one unit of work to each slave.
	Work_t* work;
	int* busy_slave = (int*) malloc(ntasks * sizeof(int));
	for (int rank = 1; rank < ntasks; rank++)
		busy_slave[rank] = 0;
	for (int rank = 1; rank < ntasks; rank++)
	{
		// Find the next item of work to do
		work = get_next_work(inputFileName, totalNbSeries, nbSeriesInChunk, idealNbSeriesInChunk,
			jobsSentCount, lastEndIndex, nbClusters, clustOnMedoids, 
			randomize, p_for_dissims);

		if (randomize)
			sentSeriesCount += nbSeriesInChunk;
		else
		{
			lastEndIndex = lastEndIndex + work->nbSeries;
			jobsSentCount++;
		}

		// Send it to current rank
		pack_work(work, nbSeriesInChunk, packedWork);
		free_work(work);
		fprintf(stdout, "0 / Send work %s to rank=%i / %u\n",inputFileName, rank, 
			(randomize ? sentSeriesCount : lastEndIndex));
		MPI_Send(packedWork, work_message_length, MPI_BYTE, rank, WORKTAG, MPI_COMM_WORLD);

		busy_slave[rank] = 1;

		if ((randomize && sentSeriesCount >= 1.5*totalNbSeries) //TODO: 1.5 = heuristic, magic number...
			|| (!randomize && lastEndIndex >= totalNbSeries))
		{
			// Nothing more to read
			break;
		}
	}

	// Loop over getting new work requests until there is no more work to be done
	Result_t* result;
	MPI_Status status;
	while (1)
	{
		// If no slave is active, job is over
		int atLeastOneSlaveActive = 0;
		for (int rank = 1; rank < ntasks; rank++)
		{
			if (busy_slave[rank])
			{
				atLeastOneSlaveActive = 1;
				break;
			}
		}
		if (!atLeastOneSlaveActive)
			break;

		// Receive results from a slave
		MPI_Recv(packedResult, result_message_length, MPI_BYTE, MPI_ANY_SOURCE,
			WORKTAG, MPI_COMM_WORLD, &status);
		result = unpack_result(packedResult);
		fprintf(stdout, "0 / Receive result from rank=%i on %s\n",status.MPI_SOURCE,inputFileName);

		// 'binarize' the result (only series' values) returned by the slave
		binaryResult_to_file(result, inputFileName, outputFileName);
		free_result(result);
		newSeriesCount += nbClusters;

		if ((randomize && sentSeriesCount < totalNbSeries)
			|| (!randomize && lastEndIndex < totalNbSeries))
		{
			// Get the next unit of work to be done
			work = get_next_work(inputFileName, totalNbSeries, nbSeriesInChunk,
				idealNbSeriesInChunk, jobsSentCount, lastEndIndex, nbClusters, 
				clustOnMedoids, randomize, p_for_dissims);

			if (randomize)
				sentSeriesCount += nbSeriesInChunk;
			else
			{
				lastEndIndex = lastEndIndex + work->nbSeries;
				jobsSentCount++;
			}

			// Send the slave a new work unit
			pack_work(work, nbSeriesInChunk, packedWork);
			free_work(work);
			fprintf(stdout, "0 / Send work %s to rank=%i / %u\n",inputFileName, status.MPI_SOURCE, 
				(randomize ? sentSeriesCount : lastEndIndex));
			MPI_Send(packedWork, work_message_length, MPI_BYTE,
				status.MPI_SOURCE, WORKTAG, MPI_COMM_WORLD);
		}

		else
			// No more work to do
			busy_slave[status.MPI_SOURCE] = 0;
	}

	// There's no more work to be done, so receive all results from the slaves.
	for (int rank = 1; rank < ntasks; rank++)
	{
		if (busy_slave[rank])
		{
			MPI_Recv(packedResult, result_message_length, MPI_BYTE,
				rank, WORKTAG, MPI_COMM_WORLD, &status);
			result = unpack_result(packedResult);

			// 'binarize' the result (only series' values) returned by the slave
			binaryResult_to_file(result, inputFileName, outputFileName);
			free_result(result);
			newSeriesCount += nbClusters;
		}
	}
	free(busy_slave);

	// Finalize output file: write total number of series inside it, and tsLength
	ofile = fopen(outputFileName, "r+b"); //read and write, binary
	fseek(ofile, 0, SEEK_SET);
	Byte intBuffer[4];
	write_int(newSeriesCount, 4, intBuffer);
	fwrite(intBuffer, 1, 4, ofile);
	write_int(tsLength, 4, intBuffer);
	fwrite(intBuffer, 1, 4, ofile);
	fclose(ofile);
}

// generate random temporary file names
static char* get_unique_name()
{
	size_t nbDigits = 7; //rather arbitrary
	size_t stringLength = 5 + nbDigits; //5 for '.tmp/'
	char* s = (char*) malloc(stringLength + 1);
	s[0] = '.';
	s[1] = 't';
	s[2] = 'm';
	s[3] = 'p';
	s[4] = '/';
	for (int i=0; i<nbDigits; i++)
		s[5+i] = '0' + get_rand_int() % 10;
	s[stringLength] = 0;
	return s;
}

// code executed by master process
void master_run(char* mainInputFileName, uint32_t totalNbSeries, uint32_t nbSeriesInChunk,
	double idealNbSeriesInChunk, uint32_t tsLength, uint32_t nbClusters, 
	int randomize, uint32_t p_for_dissims)
{
	// Basic sanity check: nbClusters must be clearly less than series count per chunk
	if (10 * nbClusters >= nbSeriesInChunk)
	{
		fprintf(stdout, "WARNING: cluster size (%u) may be too high compared with chunk size (%u).\n",
			nbClusters, nbSeriesInChunk);
	}
	
	// Find out how many processes there are in the default communicator
	int ntasks;
	MPI_Comm_size(MPI_COMM_WORLD, &ntasks);
	if (ntasks <= 1)
	{
		fprintf(stderr,"No slaves available (only master running).\n");
		return;
	}
	
	// initializations
	char* inputFileName = mainInputFileName;
	char* outputFileName;

	uint32_t nbSeriesInFile = totalNbSeries;

	while (nbSeriesInFile > nbSeriesInChunk)
	{
		outputFileName = get_unique_name();
		uint32_t clustOnMedoids = (nbSeriesInFile < totalNbSeries ? 1 : 0);
		clusters_reduce(inputFileName, outputFileName, ntasks, nbSeriesInFile, nbSeriesInChunk,
			idealNbSeriesInChunk, tsLength, nbClusters, clustOnMedoids, 
			randomize, p_for_dissims);

		// read nbSeries in outputFile
		nbSeriesInFile = get_nbSeries(outputFileName);
		
		// update file names
		if (strcmp(mainInputFileName, inputFileName) != 0)
		{
			// No need to keep every intermediate binary
			unlink(inputFileName);
			free(inputFileName);
		}
		inputFileName = outputFileName;
	}

	// read nbSeries in inputFileName (the last one)
	// we know that there is at most 'nbSeriesInChunk' series in it
	nbSeriesInFile = get_nbSeries(inputFileName);

	// Expected size of a Work message in bytes:
	uint32_t work_message_length = get_packedWork_length(nbSeriesInChunk);
	Byte packedWork[work_message_length];

	// Expected size of a Result message in bytes: (uint32_t is on 4 bytes)
	uint32_t result_message_length = get_packedResult_length(nbClusters);
	Byte packedResult[result_message_length];

	// Run a last task by some slave, and get final result
	Work_t* work = get_next_work(inputFileName, nbSeriesInFile, nbSeriesInChunk,
		(double)nbSeriesInFile, 0, 0, nbClusters, 1, 0, p_for_dissims);

	// Send the slave a new work unit
	pack_work(work, nbSeriesInChunk, packedWork);
	free_work(work);
	int selectedSlaveRank = get_rand_int() % (ntasks - 1) + 1;
	fprintf(stdout, "0 / Send final work %s to rank=%i / %u\n", 
		inputFileName, selectedSlaveRank, nbSeriesInFile);
	MPI_Send(packedWork, work_message_length, MPI_BYTE, selectedSlaveRank,
		WORKTAG, MPI_COMM_WORLD);

	MPI_Status status;
	// Wait for him to finish
	MPI_Recv(packedResult, result_message_length, MPI_BYTE, selectedSlaveRank,
			WORKTAG, MPI_COMM_WORLD, &status);
	Result_t* finalResult = unpack_result(packedResult);
	fprintf(stdout, "0 / Receive final result from rank=%i on %s\n",status.MPI_SOURCE,inputFileName);

	//Tell all the slaves to exit by sending an empty message with the DIETAG.
	for (int rank = 1; rank < ntasks; ++rank)
		MPI_Send(0, 0, MPI_BYTE, rank, DIETAG, MPI_COMM_WORLD);

	const char finalBinaryName[] = "ppamFinalSeries.bin";
	result_to_XML(finalResult, inputFileName, finalBinaryName, p_for_dissims);
	free_result(finalResult);

	// free memory
	if (strcmp(mainInputFileName, inputFileName))
	{
		// Keep last input binary, but rename it
		rename(inputFileName, finalBinaryName);
		free(inputFileName);
	}
	else
	{
		// just symlink mainInputFileName
		if (!access(finalBinaryName, F_OK))
			unlink(finalBinaryName);
		if (symlink(mainInputFileName, finalBinaryName))
			fprintf(stderr,"Cannot create symlink to initial binary file.\n");
	}
}
