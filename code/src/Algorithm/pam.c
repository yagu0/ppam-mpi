#include "Util/types.h"
#include "Util/rng.h"
#include <cds/Vector.h>
#include <stdlib.h>
#include <math.h>

// auxiliary function to obtain a random sample of 1..n with k elements
static void sample(uint32_t* v, uint32_t n, uint32_t k)
{
	// refVect = (0,1,...,n-1,n)
	uint32_t* refVect = (uint32_t*) malloc(n * sizeof(uint32_t));
	for (uint32_t i = 0; i < n; i++)
		refVect[i] = i;

	uint32_t curSize = n; // current size of the sampling set
	for (uint32_t j = 0; j < k; j++)
	{
		// pick an index in sampling set:
		uint32_t index = get_rand_int() % curSize;
		v[j] = refVect[index];
		// move this index outside of sampling set:
		refVect[index] = refVect[--curSize];
	}

	free(refVect);
}

// assign a vector (represented by its dissimilarities to others, as dissimilarities[index,])
// to a cluster, represented by its center ==> output is integer in 0..K-1
static uint32_t assignCluster(uint32_t index, Real* dissimilarities,
	uint32_t* centers, uint32_t n, uint32_t K)
{
	uint32_t minIndex = 0;
	Real minDist = dissimilarities[index * n + centers[0]];

	for (uint32_t j = 1; j < K; j++)
	{
		if (dissimilarities[index * n + centers[j]] < minDist)
		{
			minDist = dissimilarities[index * n + centers[j]];
			minIndex = j;
		}
	}

	return minIndex;
}

// assign centers given a clustering, and also compute corresponding distortion
static void assign_centers(uint32_t nbClusters, Vector** clusters, Real* dissimilarities,
	uint32_t nbItems, uint32_t* ctrs, Real* distor)
{
	*distor = 0.0;
	// TODO [heuristic]: checking only a neighborhood of the former center ?
	for (uint32_t j = 0; j < nbClusters; j++)
	{
		// If the cluster is empty, choose a center at random (pathological case...)
		uint32_t minIndex = get_rand_int() % nbItems;
		Real minSumDist = INFINITY;
		for (uint32_t i = 0; i < vector_size(clusters[j]); i++)
		{
			uint32_t index1;
			vector_get(clusters[j], i, index1);
			// attempt to use current index as center
			Real sumDist = 0.0;
			for (uint32_t ii = 0; ii < vector_size(clusters[j]); ii++)
			{
				uint32_t index2;
				vector_get(clusters[j], ii, index2);
				sumDist += dissimilarities[index1 * nbItems + index2];
			}
			if (sumDist < minSumDist)
			{
				minSumDist = sumDist;
				minIndex = index1;
			}
		}
		ctrs[j] = minIndex;
		*distor += minSumDist;
	}
}

// Core PAM algorithm from a dissimilarity matrix; (e.g. nstart=10, maxiter=100)
void pam(Real* dissimilarities, uint32_t nbItems, uint32_t nbClusters, int clustOnMedoids,
	uint32_t nbStart, uint32_t maxNbIter, Result_t* result)
{
	uint32_t* ctrs = result->medoids_ranks; //shorthand
	uint32_t* oldCtrs = (uint32_t*) malloc(nbClusters * sizeof(uint32_t));
	Vector** clusters = (Vector**) malloc(nbClusters * sizeof(Vector*));
	Vector** bestClusts = (Vector**) malloc(nbClusters * sizeof(Vector*));
	for (uint32_t j = 0; j < nbClusters; j++)
	{
		clusters[j] = vector_new(uint32_t);
		bestClusts[j] = vector_new(uint32_t);
	}

	Real lastDistor, distor, bestDistor = INFINITY;
	for (uint32_t startKount = 0; startKount < nbStart; startKount++)
	{
		// centers (random) [re]initialization
		if (clustOnMedoids)
		{
			// In this special case (clustering groups of medoids), we can improve the sampling
			uint32_t randomMedoidRank = get_rand_int() % (nbItems / nbClusters);
			uint32_t startIndex = randomMedoidRank * nbClusters;
			for (uint32_t index = startIndex; index < startIndex + nbClusters; index++)
				ctrs[index - startIndex] = index;
		}
		else
			// No information: complete random sampling
			sample(ctrs, nbItems, nbClusters);	
		for (uint32_t j = 0; j < nbClusters; j++)
			oldCtrs[j] = 0;
		uint32_t kounter = 0;

		distor = INFINITY;
		do // while 'distortion' decreases...
		{
			// (re)initialize clusters to empty sets
			for (uint32_t j = 0; j < nbClusters; j++)
				vector_clear(clusters[j]);

			// estimate clusters belongings
			for (uint32_t i = 0; i < nbItems; i++)
			{
				uint32_t affectation = assignCluster(i, dissimilarities, ctrs, nbItems, nbClusters);
				vector_push(clusters[affectation], i);
			}

			// copy current centers to old centers
			for (uint32_t j = 0; j < nbClusters; j++)
				oldCtrs[j] = ctrs[j];

			// recompute centers
			lastDistor = distor;
			assign_centers(nbClusters, clusters, dissimilarities, nbItems, ctrs, &distor);
		}
		while (distor < lastDistor && kounter++ < maxNbIter);

		if (distor < bestDistor)
		{
			// copy current clusters into bestClusts
			for (uint32_t j = 0; j < nbClusters; j++)
			{
				vector_clear(bestClusts[j]);
				for (uint32_t i = 0; i < vector_size(clusters[j]); i++)
				{
					uint32_t index;
					vector_get(clusters[j], i, index);
					vector_push(bestClusts[j], index);
				}
			}
			bestDistor = distor;
		}
	}

	// Assign centers from bestClusts
	assign_centers(nbClusters, bestClusts, dissimilarities, nbItems, ctrs, &distor);

	free(oldCtrs);
	for (uint32_t j = 0; j < nbClusters; j++)
	{
		vector_destroy(clusters[j]);
		vector_destroy(bestClusts[j]);
	}
	free(clusters);
	free(bestClusts);
}
