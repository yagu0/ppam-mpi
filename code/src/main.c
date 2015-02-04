#include "MPI_Main/master.h"
#include "MPI_Main/slave.h"
#include "Util/utils.h"
#include "Util/rng.h"
#include <sys/stat.h>
#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include "TimeSeries/serialize.h"
#include "TimeSeries/deserialize.h"
#include "Classification/getClass.h"
#include <string.h>
#include <cds/Vector.h>
#include <libxml/xmlreader.h>

// serialize text file argv[1] into a binary file argv[2]
int serialize_main(int argc, char** argv)
{
	const char* ifileName = argv[1];
	const char* ofileName = argv[2];
	int byCols = atoi(argv[3]);
	uint32_t nbItems = atoi(argv[4]); //==0 for "all series"
	
	if (byCols)
		serialize_byCols(ifileName, ofileName, nbItems);
	else
		serialize_byRows(ifileName, ofileName, nbItems);
	return 0;
}

// deserialize binary file argv[1] into text file argv[2]
int deserialize_main(int argc, char** argv)
{
	const char* ifileName = argv[1];
	const char* ofileName = argv[2];
	Vector* vranks = vector_new(uint32_t);
	//each token is at most two ints (a-b = from a to b included)
	char* token = strtok(argv[3], ",");
	int retrieveAll = 0;
	uint32_t* ranks = NULL;
	while (token)
	{
		//scan token to find middle position of '-' (if any)
		int minusPos = -1;
		int tokenLength = strlen(token);
		//loop starts at index 1 because -N is allowed (and means 'everything')
		for (int i=1; i<tokenLength; i++)
		{
			if (token[i] == '-')
			{
				minusPos = i;
				break;
			}
		}
		if (minusPos < 0)
		{
			int64_t rank = (int64_t)atoi(token);
			if (rank <= 0)
			{
				retrieveAll = 1;
				break;
			}
			vector_push(vranks, (uint32_t)rank);
		}
		else
		{
			token[minusPos] = 0;
			int int1 = atoi(token);
			int int2 = atoi(token+minusPos+1);
			for (uint32_t i=int1; i<=int2; i++)
				vector_push(vranks, i);
		}
		token = strtok(NULL, ",");
	}
	uint32_t nbRanks = retrieveAll
		? 0
		: vector_size(vranks);
	if (!retrieveAll)
	{
		ranks = (uint32_t*) malloc(nbRanks*sizeof(uint32_t));
		for (uint32_t i=0; i<nbRanks; i++)
		{
			vector_get(vranks, i, ranks[i]);
			ranks[i]--; //re-express on 0...{n-1}
		}
	}
	vector_destroy(vranks);
	
	deserialize(ifileName, ofileName, ranks, nbRanks);
	return 0;
}

//main clustering task (master or slave)
int cluster_main(int argc, char **argv)
{
	MPI_Init(&argc, &argv);

	char* ifileName = argv[1]; //could be "../data/test.bin"
	uint32_t nbSeriesInChunk = atoi(argv[2]); //could be 3000
	uint32_t nbClusters = atoi(argv[3]); //could be 15
	int randomize = atoi(argv[4]); //boolean
	uint32_t p_for_dissims = atoi(argv[5]); //1 for L1, 2 for L2, ...etc

	// Get totalNbSeries and tsLength
	uint32_t totalNbSeries = get_nbSeries(ifileName);
	uint32_t tsLength = get_tsLength(ifileName);

	// Basic sanity checks
	if (nbClusters <= 0 || nbSeriesInChunk <= 1)
	{
		MPI_Finalize();
		return 0;
	}
	if (nbSeriesInChunk > totalNbSeries)
		nbSeriesInChunk = totalNbSeries;
	if (nbClusters > nbSeriesInChunk)
		nbClusters = nbSeriesInChunk;

	double idealNbSeriesInChunk = 0.0; //unused if randomize == TRUE
	if (!randomize) 
	{
		// Adjust nbSeriesInChunk to avoid small remainders.
		// Each node should have at least nbSeriesInChunk (as given to the function).

		// ==> We seek for the largest N such that (double)totalNbSeries / N >= nbSeriesInChunk
		uint32_t N = totalNbSeries / nbSeriesInChunk + 1;
		while ((double)totalNbSeries / N < nbSeriesInChunk) N--;
		// At this point N>=1 is the solution
		idealNbSeriesInChunk = (double)totalNbSeries / N;
		nbSeriesInChunk = ceil(idealNbSeriesInChunk);
	}
	
	// Initialize random generator
	init_rng(1);
	
	// Find out my identity in the default communicator
	int myrank;
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

	if (myrank == 0)
	{
		// create temporary folder for intermediate results
		mkdir(".tmp", S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
		
		master_run(ifileName, totalNbSeries, nbSeriesInChunk, idealNbSeriesInChunk, 
			tsLength, nbClusters, randomize, p_for_dissims);
	}
	
	else
		slave_run(myrank, nbSeriesInChunk, nbClusters);

	MPI_Finalize();
	return 0;
}

//main classification task (using clustering result)
int classif_main(int argc, char** argv)
{
	const char* ifileName = argv[1];
	const char* xmlFileName = argv[2];
	
	// 1] load and retrieve info from XML file
	xmlDoc* doc = xmlReadFile(xmlFileName, NULL, 0);

	// Get the root element node
	xmlNode* root_element = xmlDocGetRootElement(doc);

	uint32_t p_for_dissims = 0;
	uint32_t* ranks = NULL;
	uint32_t nbClusters = 0;
	char* binFileName;
	for (xmlNode* cur_node=root_element->children; cur_node; cur_node=cur_node->next) 
	{
		if (cur_node->type != XML_ELEMENT_NODE)
			continue;
		if (!strcmp(cur_node->name,"p_for_dissims"))
			p_for_dissims = atoi(cur_node->last->content);
		else if (!strcmp(cur_node->name,"ranks"))
		{
			//first pass: find nbClusters
			for (xmlNode* rankNode=cur_node->children; rankNode; rankNode=rankNode->next)
			{
				if (rankNode->type == XML_ELEMENT_NODE && !strcmp(rankNode->name,"rank"))
					nbClusters++;
			}
			//second pass: fill ranks (not optimal, but not very important here)
			ranks = (uint32_t*) malloc(nbClusters*sizeof(uint32_t));
			uint32_t index = 0;
			for (xmlNode* rankNode=cur_node->children; rankNode; rankNode=rankNode->next)
			{
				if (rankNode->type == XML_ELEMENT_NODE && !strcmp(rankNode->name,"rank"))
					ranks[index++] = atoi(rankNode->last->content) - 1;
			}
		}
		else if (!strcmp(cur_node->name,"file"))
		{
			binFileName = (char*) malloc(strlen(cur_node->last->content)+1);
			strcpy(binFileName, cur_node->last->content);
		}
	}

	xmlFreeDoc(doc);
	xmlCleanupParser();

	uint32_t tsLength1 = get_tsLength(ifileName);
	uint32_t tsLength2 = get_tsLength(binFileName);
	if (tsLength1 != tsLength2)
	{
		fprintf(stderr,"Warning: nbValues do not match. Data will be truncated.\n");
		if (tsLength1 > tsLength2)
			tsLength1 = tsLength2;
	}
	uint32_t nbValues = (tsLength1 - 4) / 3;
	
	// 2] Classify all series by batches of CURVES_PER_REQUEST
	uint32_t nbSeries = get_nbSeries(ifileName);
	PowerCurve* medoids = deserialize(binFileName, NULL, ranks, nbClusters);
	free(binFileName);
	free(ranks);
	ranks = (uint32_t*)malloc(CURVES_PER_REQUEST*sizeof(uint32_t));
	
	uint32_t smallestNonProcessedIndex = 0;
	double DISTOR = 0.0;
	while (smallestNonProcessedIndex < nbSeries)
	{
		uint32_t lowerBound = smallestNonProcessedIndex;
		uint32_t upperBound = smallestNonProcessedIndex + CURVES_PER_REQUEST;
		if (upperBound > nbSeries)
			upperBound = nbSeries;
		for (uint32_t i=0; i<upperBound-lowerBound; i++)
			ranks[i] = lowerBound + i;
		PowerCurve* data = deserialize(ifileName, NULL, ranks, upperBound-lowerBound);
		uint32_t* labels = get_class(data, upperBound-lowerBound, medoids, nbClusters, 
			nbValues, p_for_dissims, &DISTOR);
		// send labels to standard output
		for (uint32_t i=0; i<upperBound-lowerBound; i++)
		{
			free(data[i].values);
			fprintf(stdout, "%u\n",labels[i]);
		}
		free(data);
		free(labels);
		smallestNonProcessedIndex += (upperBound-lowerBound);
	}
	for (uint32_t i=0; i<nbClusters; i++)
		free(medoids[i].values);
	free(medoids);
	free(ranks);
	fprintf(stderr, "DISTOR = %g\n",DISTOR);
	return 0;
}

int main(int argc, char** argv)
{
	if (argc <= 1)
	{
		fprintf(stderr, "No argument provided. Exit.\n");
		return 1;
	}
	
	if (!strcmp(argv[1], "serialize"))
		return serialize_main(argc-1, argv+1);
	if (!strcmp(argv[1], "deserialize"))
		return deserialize_main(argc-1, argv+1);
	if (!strcmp(argv[1], "cluster"))
		return cluster_main(argc-1, argv+1);
	if (!strcmp(argv[1], "classif"))
		return classif_main(argc-1, argv+1);

	fprintf(stderr, "Unknown first argument.\n");
	return 1;
}
