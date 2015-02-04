#include "TimeSeries/deserialize.h"
#include <stdio.h>
#include "Util/utils.h"

// Deserialize a portion of a binary file into an array of PowerCurve, or a file
// WARNING: NO start bytes, NO date times (all series have the same length, at same times).
PowerCurve* deserialize(const char* ifileName, const char* ofileName,
	uint32_t* ranks, uint32_t nbRanks)
{
	// Read tsLength at the beginning of the file
	uint32_t tsLength = get_tsLength(ifileName);
	
	uint32_t valuesPerSerie = (tsLength - 4) / 3; //remove 4 bytes of ID

	FILE* ifile = fopen(ifileName, "rb");
	FILE* ofile = NULL;
	if (ofileName)
		ofile = fopen(ofileName, "w");
	
	if (!ranks || nbRanks <= 0)
	{
		nbRanks = get_nbSeries(ifileName);
		ranks = NULL;
	}
	
	PowerCurve* powerCurves = NULL;
	if (!ofile)
		powerCurves = (PowerCurve*) malloc(nbRanks * sizeof(PowerCurve));

	for (uint32_t i = 0; i < nbRanks; i++)
	{
		// position to the beginning of current (binarized) time-series
		// NOTE: shift by 8 bytes, because data size and series length are written first
		fseek(ifile, 8 + (ranks ? ranks[i] : i) * tsLength, SEEK_SET);

		PowerCurve* powerCurve;
		if (!ofile)
		{
			powerCurve = powerCurves + i;
			powerCurve->values = (Real*) malloc(valuesPerSerie * sizeof(Real));
		}
		
		// translate 4-bytes binary integer into integer ID
		void* binaryID = malloc(4);
		size_t lengthRead = fread(binaryID, 4, 1, ifile);
		if (lengthRead != 1)
			fprintf(stderr,"Warning: deserializing truncated binary file.\n");
		uint32_t ID = bInt_to_uint((Byte*) binaryID, 4);
		free(binaryID);
		if (ofile)
			fprintf(ofile, "%u,", ID);
		else
			powerCurve->ID = ID;

		// translate 3-bytes binary integers into Real
		Byte* binarySerie = (Byte*) malloc(3 * valuesPerSerie);
		lengthRead = fread(binarySerie, 1, 3*valuesPerSerie, ifile);
		if (lengthRead != 3*valuesPerSerie)
			fprintf(stderr,"Warning: deserializing truncated binary file.\n");
		for (uint32_t i = 0; i < valuesPerSerie; i++)
		{
			uint32_t powerInt = bInt_to_uint(binarySerie + 3 * i, 3);
			if (ofile)
			{
				fprintf(ofile, "%g", powerInt / 10.0 - 0.0);
				if (i < valuesPerSerie-1)
					fprintf(ofile, ",");
			}
			else
				powerCurve->values[i] = powerInt / 10.0 - 0.0;
		}
		free(binarySerie);
		if (ofile)
			fprintf(ofile, "\n");
	}

	fclose(ifile);
	if (ofile)
		fclose(ofile);

	return powerCurves;
}
