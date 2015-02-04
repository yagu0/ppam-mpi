#ifndef PPAM_SERIALIZE_H
#define PPAM_SERIALIZE_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

//main job: parse a text file into a binary compressed version
void serialize_byCols(const char* ifileName, const char* ofileName, uint32_t nbItems);

//serialize from usual 'by-row' data
void serialize_byRows(const char* ifileName, const char* ofileName, uint32_t nbItems);

#endif
