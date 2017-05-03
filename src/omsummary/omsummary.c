/*
* Copyright Newcastle University, UK.
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
* 1. Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/

// Open Movement Summary Generator
// Dan Jackson

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#define _strcasecmp _stricmp
#else
#define _strcasecmp strcasecmd
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "omsummary.h"
#include "timestamp.h"
#include "csvload.h"



typedef struct
{
	char label[256];
	double start;
	double end;
	double total;
} interval_t;

typedef struct
{
	int dummy;
} times_t;

int TimesLoad(times_t *times, const char *filename)
{
	interval_t newInterval = { 0 };
	csv_load_t csv;
	int colStart = -1, colEnd = -1, colLabel = -1;

	int headerCells = CsvOpen(&csv, filename, CSV_HEADER_DETECT_NON_NUMERIC);
	if (headerCells > 0)
	{
		// TODO: Parse header cells
		for (int i = 0; i < headerCells; i++)
		{
			const char *heading = CsvTokenString(&csv, i);
			//fprintf(stderr, "HEADER %d: %s\n", i + 1, heading);
			if (!_strcasecmp(heading, "Start")) { colStart = i; }
			else if (!_strcasecmp(heading, "End")) { colEnd = i; }
			else if (!_strcasecmp(heading, "Label")) { colLabel = i; }
			else
			{
				fprintf(stderr, "WARNING: Unknown column heading: %s.\n", heading);
			}
		}
	}

	if (colStart < 0 && colEnd < 0 && colLabel < 0)
	{
		fprintf(stderr, "WARNING: No recognized heading line -- default columns will be used.\n");
		colStart = 0;
		colEnd = 1;
		colLabel = 2;
	}

	if (colStart < 0 || colEnd < 0)
	{
		fprintf(stderr, "ERROR: One or more required columns ('start', 'end') are missing.\n");
	}

	int tokens;
	while ((tokens = CsvReadLine(&csv)) >= 0)
	{
		if (tokens > colStart && tokens > colEnd)
		{
			if (colLabel >= 0 && tokens > colLabel)
			{
				strcpy(newInterval.label, CsvTokenString(&csv, colLabel));
			}
			else 
			{
				// Use start as label
				strcpy(newInterval.label, CsvTokenString(&csv, colStart));
			}
			newInterval.start = TimeParse(CsvTokenString(&csv, colStart));
			newInterval.end = TimeParse(CsvTokenString(&csv, colEnd));

			printf("---\n");
			printf("-S: %s\n", TimeString(newInterval.start, NULL));
			printf("-E: %s\n", TimeString(newInterval.end, NULL));
			printf("-L: %s\n", newInterval.label);
		}
		else if (tokens > 0)	// Ignore completely blank lines
		{
			fprintf(stderr, "WARNING: Too-few columns, ignoring row on line %d.\n", CsvLineNumber(&csv));
		}
	}

	return 0;
}


int OmSummaryRun(omsummary_settings_t *settings)
{
	times_t times;

	fprintf(stderr, "Opening times: %s\n", settings->timesFilename);
	TimesLoad(&times, settings->timesFilename);

	fprintf(stderr, "Opening data: %s\n", settings->filename);
	return 0;
}


