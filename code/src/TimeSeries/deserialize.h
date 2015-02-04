#ifndef PPAM_DESERIALIZE_H
#define PPAM_DESERIALIZE_H

#include <stdint.h>
#include <stdlib.h>
#include "Util/types.h"

// deserialize a portion of a binary file into an array of PowerCurve
PowerCurve* deserialize(const char* ifileName, const char* ofileName,
	uint32_t* ranks, uint32_t nbRanks);

#endif
