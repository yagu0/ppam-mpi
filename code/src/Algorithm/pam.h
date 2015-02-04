#ifndef PPAM_PAM_H
#define PPAM_PAM_H

#include "Util/types.h"

#define NSTART 5
#define MAXITER 100

// Core PAM algorithm from a 'flat' dissimilarity matrix
void pam(Real* dissimilarities, uint32_t nbItems, uint32_t nbClusters, 
	int clustOnMedoids, uint32_t nbStart, uint32_t maxNbIter, Result_t* result);

#endif
