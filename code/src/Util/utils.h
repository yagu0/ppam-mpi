#ifndef PPAM_UTIL_UTILS_H
#define PPAM_UTIL_UTILS_H

#include "Util/types.h"
#include <stdlib.h>
#include <stdio.h>

#define CURVES_PER_REQUEST 50

void free_work(Work_t* work);

void free_result(Result_t* result);

char readInt(FILE* stream, int64_t* integer);

char readReal(FILE* stream, Real* real);

// convert n-bytes binary integers to uint32_t
uint32_t bInt_to_uint(Byte* pInteger, size_t bytesCount);

// serialize integers with a portable bytes order
void write_int(uint32_t integer, size_t bytesCount, Byte* buffer);

// Expected size of a Work message in bytes:
uint32_t get_packedWork_length(uint32_t nbSeriesInChunk);

// Expected size of a Result message in bytes: (uint32_t is on 4 bytes)
uint32_t get_packedResult_length(uint32_t nbClusters);

// get metadata: nbSeries
uint32_t get_nbSeries(const char* ifileName);

// get metadata: tsLength
uint32_t get_tsLength(const char* ifileName);

#endif
