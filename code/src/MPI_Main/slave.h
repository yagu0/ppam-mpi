#ifndef PPAM_SLAVE_H
#define PPAM_SLAVE_H

#include <stdint.h>

// code executed by slave process
void slave_run(int myrank, uint32_t nbSeriesInChunk, uint32_t nbClusters);

#endif
