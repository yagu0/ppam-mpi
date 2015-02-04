#ifndef PPAM_UNPACK_H
#define PPAM_UNPACK_H

#include "Util/types.h"

// deserialize a Work_t object from a bytes string
Work_t* unpack_work(Byte* packedWork, uint32_t nbSeriesInChunk);

// deserialize a Result_t object from a bytes string
Result_t* unpack_result(Byte* packedResult);

#endif
