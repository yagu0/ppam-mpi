#ifndef PPAM_TYPES_H
#define PPAM_TYPES_H

#include <stdint.h>

// types of work given to a slave
#define WORKTAG 1
#define DIETAG 2

// 256 characters for file name should be enough
#define NCHAR_FNAME 256

typedef unsigned char Byte;

typedef double Real;

// Type to describe a job to be done in a node
//TODO: merge with packed version to avoid extra copy by MPI
typedef struct Work_t {
	// "structural" parameters:
	char* inputFileName;
	uint32_t nbSeries;
	uint32_t* ranks;
	// clustering parameters [to be completed]:
	uint32_t nbClusters;
	uint32_t clustOnMedoids; //a boolean, but 1 byte storage would be inefficient
	uint32_t p_for_dissims;
} Work_t;

// Type returned by a worker (clusters labels and medoids)
//TODO: merge with packed version to avoid extra copy by MPI
typedef struct Result_t {
	// parameters describing sizes
	uint32_t nbClusters;
	// informative parameters:
	uint32_t* medoids_ID;
	uint32_t* medoids_ranks;
} Result_t;

// data structure to store a customer ID + [time-]serie
typedef struct PowerCurve {
	uint32_t ID;
	Real* values;
} PowerCurve;

#endif
