#include "TimeSeries/serialize.h"
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <math.h>
#include "Util/types.h"
#include "Util/utils.h"
#include <cds/Vector.h>
#include <string.h>

// parse a line into two integers (ID, raw power)
static void scan_line(FILE* ifile, int posID, uint32_t* ID, int posPower, uint32_t* rawPower)
{
	char nextChar;
	int position = 1;
	while (1)
	{
		if (position == posID)
		{
			int64_t ID_on64bits;
			nextChar = readInt(ifile, &ID_on64bits);
			*ID = (uint32_t)ID_on64bits;
		}
		else if (position == posPower)
		{
			Real untruncatedPower;
			nextChar = readReal(ifile, &untruncatedPower);
			if (untruncatedPower < 0.0)
				untruncatedPower = 0.0;
			*rawPower = (uint32_t) floor(untruncatedPower*10.0);
		}
		else
			//erase the comma (and skip field then)
			nextChar = fgetc(ifile);
		
		//continue until next comma (or line end or file end)
		while (!feof(ifile) && nextChar != '\n' && nextChar != '\r' && nextChar != ',')
			nextChar = fgetc(ifile);
		position++;

		if (feof(ifile) || nextChar == '\n' || nextChar == '\r')
		{
			// skip all potential line feeds
			while (!feof(ifile) && nextChar == '\n' || nextChar == '\r')
				nextChar = fgetc(ifile);
			if (!feof(ifile))
				ungetc(nextChar, ifile);
			break;
		}
	}
}

//main job: parse a text file into a binary compressed version
//TODO [long term]: adapt to distributed files/DB, maybe to distributed binary chunks
void serialize_byCols(const char* ifileName, const char* ofileName, uint32_t nbItems)
{
	// use the header to know positions of ID and rawPower
	FILE* ifile = fopen(ifileName, "r");
	uint32_t headerShift = 0;
	char curChar;
	Vector* header = vector_new(char);
	do
	{
		curChar = fgetc(ifile);
		headerShift++;
		if (curChar == '\n' || curChar == '\r')
		{
			//flush all potential other line feeds
			while (curChar == '\n' || curChar == '\r')
				curChar = fgetc(ifile);
			ungetc(curChar, ifile);
			break;
		}
		vector_push(header, curChar);
	}
	while (1);
	char* headerString = (char*)malloc((vector_size(header) + 1)*sizeof(char));
	VectorIterator* it = vector_get_iterator(header);
	int index = 0;
	while (vectorI_has_data(it))
	{
		vectorI_get(it, headerString[index]);
		vectorI_move_next(it);
		index++;
	}
	vectorI_destroy(it);
	headerString[index] = 0;
	vector_destroy(header);
	int position = 1, posID = 0, posPower = 0;
	char* columnDescriptor = strtok(headerString, ",");
	while (columnDescriptor != NULL)
	{
		if (!strcmp(columnDescriptor,"FK_CCU_ID") || !strcmp(columnDescriptor,"fk_ccu_id"))
			posID = position;
		else if (!strcmp(columnDescriptor,"CPP_PUISSANCE_BRUTE"))
			posPower = position;
		position++;
		columnDescriptor = strtok(NULL, ",");
	}
	free(headerString);
	
	//estimate tsLength with a scan of the 3 first series
	uint32_t ID=0, rawPower=0, lastID=0, refTsLength=0;
	scan_line(ifile, posID, &ID, posPower, &rawPower);
	//'sl' = sample lengths (short because a lot of comparisons then)
	uint32_t* sl = (uint32_t*) calloc(3, sizeof(uint32_t));
	for (int i=0; i<3; i++)
	{
		lastID = ID;
		while (ID == lastID)
		{
			sl[i]++;	
			scan_line(ifile, posID, &ID, posPower, &rawPower);
		}
	}
	if (sl[1] <= sl[0] <= sl[2] || sl[2] <= sl[0] <= sl[1])
		refTsLength = sl[0];
	else if (sl[0] <= sl[1] <= sl[2] || sl[2] <= sl[1] <= sl[0])
		refTsLength = sl[1];
	else
		refTsLength = sl[2];
	free(sl);
	//go back at the beginning of the first series (ready to read '\n'...)
	fseek(ifile, headerShift-1, SEEK_SET);

	// output file to write time-series sequentially, binary format.
	// Format: for each series, ID is stored on 4 bytes (unsigned integer32). Then,
	//         (<rawPower>)+ follow, with rawPower stored as a "3 bytes int"
	//         rawPower values are multiplied by 10 and truncated one digit after 0
	// NOTE: no raw power should be exactly zero
	FILE* ofile = fopen(ofileName, "wb");

	// leave space to write the number of series (32bits), and their length in bytes (32bits)
	for (int i = 0; i < 8; i++)
		fputc(0, ofile);

	// process one client (ID in first column) at a time
	uint64_t processedLines = 0; //execution trace
	uint32_t seriesCount=0, skippedSeriesCount=0, tsLength=0;
	uint32_t mismatchLengthCount=0, overflowCount=0;
	Byte tsBuffer[4+3*refTsLength];
	int overflow = 0;
	lastID = 0;
	while (!feof(ifile))
	{
		// next element to read always start with a digit
		do
			curChar = fgetc(ifile);
		while (!feof(ifile) && (curChar < '0' || curChar > '9'));
		if (feof(ifile))
			break;
		ungetc(curChar, ifile);

		// read line
		scan_line(ifile, posID, &ID, posPower, &rawPower);
		if (ID != lastID)
		{
			//just starting a new time-series: must process the last one (if there is a last one !)
			if (lastID > 0)
			{				
				if (tsLength == refTsLength && !overflow)
				{
					seriesCount++;
					fwrite(tsBuffer, 4+3*tsLength, 1, ofile);
					if (nbItems > 0 && seriesCount >= nbItems)
						break;
				}
				//if something wrong happened, skip series
				else
				{
					skippedSeriesCount++;
					if (tsLength != refTsLength)
						mismatchLengthCount++;
					if (overflow)
						overflowCount++;
				}
			}
			
			// ID for the new series is printed only once:
			write_int(ID, 4, tsBuffer);
			// reinitialize flags
			overflow = 0;
			tsLength = 0;
			lastID = ID;
		}

		overflow = (overflow || (rawPower >= (1 << 24)));
		//We cannot write more than refTsLength bytes
		if (tsLength < refTsLength)
			write_int(rawPower, 3, tsBuffer + 4+3*tsLength);
		tsLength++;
		
		if ((++processedLines) % 1000000 == 0)
			fprintf(stdout,"Processed %"PRIu64" lines\n", processedLines);
	}

	if (!overflow && tsLength == refTsLength && (nbItems <= 0 || seriesCount < nbItems))
	{
		// flush last time-series if all conditions are met
		fwrite(tsBuffer, 4+3*tsLength, 1, ofile);
		seriesCount++;
	}
	else if (nbItems <= 0 || seriesCount < nbItems)
	{
		if (tsLength != refTsLength)
			mismatchLengthCount++;
		if (overflow)
			overflowCount++;
	}

	// write lines count and size of a time-series in bytes
	Byte intBuffer[4];
	fseek(ofile, 0, SEEK_SET);
	write_int(seriesCount, 4, intBuffer);
	fwrite(intBuffer, 1, 4, ofile);
	// re-express tsLength in bytes (not forgetting the ID))
	write_int(4 + 3 * refTsLength, 4, intBuffer);
	fwrite(intBuffer, 1, 4, ofile);

	// finally print some statistics
	if (seriesCount < nbItems)
		fprintf(stdout,"Warning: only %u series retrieved.\n",seriesCount);
	fprintf(stdout,"%u overflows / %u mismatch series lengths.\n",overflowCount,mismatchLengthCount);
	
	fclose(ifile);
	fclose(ofile);
}

//serialize from usual 'by-row' data (for StarLight example and toy dataset)
void serialize_byRows(const char* ifileName, const char* ofileName, uint32_t nbItems)
{
	FILE* ifile = fopen(ifileName, "r");
	// first scan to know dimensions
	uint32_t nbValues = 0; //ID (or, more useful, real class number) comes first
	char curChar = ' ';
	while (curChar != '\n' && curChar != '\r')
	{
		curChar = fgetc(ifile);
		if (curChar == ',')
		{
			nbValues++;
			//skip potential consecutive commas (could be hard to spot)
			while (curChar == ',')
				curChar = fgetc(ifile);
			ungetc(curChar, ifile);
		}
	}
	while (curChar == '\n' || curChar == '\r')
		curChar = fgetc(ifile);
	ungetc(curChar, ifile);
	uint32_t nbSeries = 1; //we already read 1st line
	while (!feof(ifile))
	{
		if ((curChar = fgetc(ifile)) == '\n')
			nbSeries++;
	}
	fseek(ifile, 0, SEEK_SET);

	//write meta info first
	uint32_t tsLength = 3*nbValues+4;
	FILE* ofile = fopen(ofileName, "wb");
	Byte intBuffer[4];
	write_int(nbSeries, 4, intBuffer);
	fwrite(intBuffer, 1, 4, ofile);
	write_int(tsLength, 4, intBuffer);
	fwrite(intBuffer, 1, 4, ofile);
	Real rawPower;
	int64_t ID;
	
	//DEBUG / TEST (ugly, TOFIX...)
	double minrp = INFINITY, maxrp = -INFINITY;
	
	for (uint32_t i=0; i<nbSeries; i++)
	{
		//skip potential line feeds before next line
		while (curChar == '\n' || curChar == '\r')
			curChar = fgetc(ifile);
		ungetc(curChar, ifile);
		curChar = readInt(ifile, &ID);
		write_int((uint32_t)ID, 4, intBuffer);
		fwrite(intBuffer, 1, 4, ofile);
		while (curChar == ',')
			curChar = fgetc(ifile);
		ungetc(curChar, ifile);
		for (uint32_t j=0; j<nbValues; j++)
		{
			curChar = readReal(ifile, &rawPower);
			
			//DEBUG / TEST (ugly, TOFIX...)
			if (rawPower < minrp)
				minrp = rawPower;
			if (rawPower > maxrp)
				maxrp = rawPower;
			
			write_int((uint32_t)floor(10.0*(rawPower+0.0)), 3, intBuffer); //x10... +3...
			fwrite(intBuffer, 1, 3, ofile);
			while (curChar == ',')
				curChar = fgetc(ifile);
			ungetc(curChar, ifile);
		}
	}
	fclose(ifile);
	fclose(ofile);
	
	//DEBUG / TEST (ugly, TOFIX...)
	printf("min / max values = %g %g\n",minrp,maxrp);
}
