#include "Util/utils.h"
#include <math.h>
#include <cds/Vector.h>

void free_work(Work_t* work)
{
	free(work->inputFileName);
	free(work->ranks);
	free(work);
}

void free_result(Result_t* result)
{
	free(result->medoids_ID);
	free(result->medoids_ranks);
	free(result);
}

char readInt(FILE* stream, int64_t* integer)
{
	*integer = 0;
	char curChar = fgetc(stream);
	int sign = (curChar == '-' ? -1 : 1);
	while (curChar < '0' || curChar > '9')
		curChar = fgetc(stream);
	ungetc(curChar, stream);
	while ((curChar = fgetc(stream)) >= '0' && curChar <= '9')
		*integer = 10 * (*integer) + (int64_t) (curChar - '0');
	(*integer) *= sign;
	return curChar;
}

char readReal(FILE* stream, Real* real)
{
	int64_t integerPart;
	char nextChar = readInt(stream, &integerPart);
	int64_t fractionalPart = 0;
	int countZeros = 0;
	if (nextChar == '.')
	{
		//need to count zeros
		while ((nextChar = fgetc(stream)) == '0')
			countZeros++;
		if (nextChar >= '0' && nextChar <= '9')
		{
			ungetc(nextChar, stream);
			nextChar = readInt(stream, &fractionalPart);
		}
	}
	int64_t exponent = 0;
	if (nextChar == 'e' || nextChar == 'E')
		nextChar = readInt(stream, &exponent);
	int64_t divisorForFractional = pow(10, floor(log10(fractionalPart > 0 ? fractionalPart : 1))+1);
	*real = ( (Real)integerPart 
		+ (integerPart < 0 ? -1 : 1) * (Real)fractionalPart/(divisorForFractional*pow(10,countZeros)) )
			* pow(10,exponent);
	return nextChar;
}

// convert n-bytes binary integers to uint32_t
uint32_t bInt_to_uint(Byte* pInteger, size_t bytesCount)
{
	uint32_t integer = 0;
	for (size_t i = 0; i < bytesCount; i++)
		integer += ((uint32_t) (pInteger[i])) << (i << 3);
	return integer;
}

// serialize integers with a portable bytes order
void write_int(uint32_t integer, size_t bytesCount, Byte* buffer)
{
	Byte chunk;
	// write from left to right, from least to most significative bit
	for (size_t i = 0; i < bytesCount; i++)
	{
		chunk = (integer >> (i << 3)) & 0xFF;
		buffer[i] = chunk;
	}
}

// Expected size of a Work message in bytes:
uint32_t get_packedWork_length(uint32_t nbSeriesInChunk)
{
	return NCHAR_FNAME + 4 + 4*nbSeriesInChunk + 4 + 4 + 4;
}

// Expected size of a Result message in bytes: (uint32_t is on 4 bytes)
uint32_t get_packedResult_length(uint32_t nbClusters)
{
	return 4 + 4 * nbClusters + 4 * nbClusters;
}

// get metadata: nbSeries
uint32_t get_nbSeries(const char* ifileName)
{
	FILE* ifile = fopen(ifileName, "rb");
	fseek(ifile, 0, SEEK_SET);
	Byte binaryInt[4];
	size_t lengthRead = fread(binaryInt, 4, 1, ifile);
	if (lengthRead != 1)
		fprintf(stderr,"Warning: getting nbSeries from truncated binary file.\n");
	fclose(ifile);
	return bInt_to_uint(binaryInt, 4);
}

// get metadata: tsLength
uint32_t get_tsLength(const char* ifileName)
{
	FILE* ifile = fopen(ifileName, "rb");
	fseek(ifile, 4, SEEK_SET);
	Byte binaryInt[4];
	size_t lengthRead = fread(binaryInt, 4, 1, ifile);
	if (lengthRead != 1)
		fprintf(stderr,"Warning: getting tsLength from truncated binary file.\n");
	fclose(ifile);
	return bInt_to_uint(binaryInt, 4);
}
