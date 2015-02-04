#ifndef PPAM_MASTER_H
#define PPAM_MASTER_H

#include <stdint.h>

// code executed by master process
void master_run(char* fileName, uint32_t totalNbSeries, uint32_t nbSeriesInChunk,
	double idealNbSeriesInChunk, uint32_t tsLength, uint32_t nbClusters, 
	int randomize, uint32_t p_for_dissims);

#endif
