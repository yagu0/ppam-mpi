#ifndef PPAM_PACK_H
#define PPAM_PACK_H

#include "Util/types.h"

// serialize a Work_t object into a bytes string
void pack_work(Work_t* work, uint32_t nbSeriesInChunk, Byte* packedWork);

// serialize a Result_t object into a bytes string
void pack_result(Result_t* result, Byte* packedResult);

#endif
