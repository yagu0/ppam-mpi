#include "TimeSeries/serialize.h"
#include "lut.h"
#include <unistd.h>
#include <stdio.h>

static void checkFilesEqual(const char* fileName1, const char* fileName2)
{
	FILE* output = fopen(fileName1, "rb");
	FILE* refOutput = fopen(fileName2, "rb");
	while (!feof(output) && !feof(refOutput))
		LUT_ASSERT(fgetc(output) == fgetc(refOutput));
	LUT_ASSERT(feof(output) && feof(refOutput));
	fclose(output);
	fclose(refOutput);
}

void t_serialize1()
{
	const char* csvIfName = "../data/test/sample_byCols.csv";
	const char* binaryIfName = "../data/test/sample_byCols.bin";
	const char* tmpBinaryIfName = "../data/test/sample_byCols.tmp.bin";
	
	// serialize text file into a temporary binary file
	serialize_byCols(csvIfName, tmpBinaryIfName, 0);

	// compare binary result with reference
	checkFilesEqual(tmpBinaryIfName, binaryIfName);

	// remove temp file
	unlink(tmpBinaryIfName);
}

void t_serialize2()
{
	const char* csvIfName = "../data/test/sample_byRows.csv";
	const char* binaryIfName = "../data/test/sample_byRows.bin";
	const char* tmpBinaryIfName = "../data/test/sample_byRows.tmp.bin";
	
	// serialize text file into a temporary binary file
	serialize_byRows(csvIfName, tmpBinaryIfName, 0);

	// compare binary result with reference
	checkFilesEqual(tmpBinaryIfName, binaryIfName);

	// remove temp file
	unlink(tmpBinaryIfName);
}
