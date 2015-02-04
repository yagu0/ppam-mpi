#ifndef PPAM_GET_CLASS_H
#define PPAM_GET_CLASS_H

#include "Util/types.h"

uint32_t* get_class(PowerCurve* data, uint32_t nbSeries, PowerCurve* medoids, 
	uint32_t nbClusters, uint32_t nbValues, uint32_t p_for_dissims, double* DISTOR);

#endif
