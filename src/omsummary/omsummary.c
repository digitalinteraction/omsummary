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
#include <math.h>

#include "omsummary.h"
#include "timestamp.h"
#include "csvload.h"



typedef struct
{
	char label[256];	// label for this interval

	double start;		// start of this interval
	double end;			// end of this interval

	double first;		// earliest timestamp found within this interval
	double last;		// latest timestamp found within this interval
	double duration;	// sum of all time span durations intersecting this interval
	int count;			// count of all time spans overlapping this interval
} interval_t;

typedef struct
{
	int numIntervals;
	interval_t *intervals;
} times_t;

int TimesLoad(times_t *times, const char *filename)
{
	csv_load_t csv;
	int colStart = -1, colEnd = -1, colLabel = -1;
	int err = 0;

	// Zero return
	memset(times, 0, sizeof(times_t));

	int headerCells = CsvOpen(&csv, filename, CSV_HEADER_DETECT_NON_NUMERIC, CSV_SEPARATORS);
	if (headerCells > 0)
	{
		// TODO: Parse header cells
		int i;
		for (i = 0; i < headerCells; i++)
		{
			const char *heading = CsvTokenString(&csv, i);
			//fprintf(stderr, "HEADER %d: %s\n", i + 1, heading);
			if (!_strcasecmp(heading, "Start")) { colStart = i; }
			else if (!_strcasecmp(heading, "End")) { colEnd = i; }
			else if (!_strcasecmp(heading, "Label")) { colLabel = i; }
			else
			{
				fprintf(stderr, "WARNING: Unknown column %d heading: '%s'.\n", i + 1, heading);
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
		return -1;
	}

	interval_t *intervals = NULL;
	int capacityIntervals = 0;

	double lastEnd = 0;
	int tokens;
	int numIntervals = 0;
	interval_t newInterval = { 0 };
	while ((tokens = CsvReadLine(&csv)) >= 0)
	{
		if (tokens > colStart && tokens > colEnd)
		{
			if (colLabel >= 0 && tokens > colLabel)
			{
				// Use the label
				strcpy(newInterval.label, CsvTokenString(&csv, colLabel));
			}
			else 
			{
				// Use the start as the label
				strcpy(newInterval.label, CsvTokenString(&csv, colStart));
			}
			newInterval.start = TimeParse(CsvTokenString(&csv, colStart));
			newInterval.end = TimeParse(CsvTokenString(&csv, colEnd));

			if (newInterval.end < newInterval.start)
			{
				fprintf(stderr, "ERROR: Line %d has a negative interval (end before start).\n", CsvLineNumber(&csv));
				err++;
			}
			if (newInterval.start < lastEnd)
			{
				fprintf(stderr, "ERROR: Line %d has an interval that starts before a preceeding interval ends.\n", CsvLineNumber(&csv));
				err++;
			}
			if (newInterval.end > lastEnd)
			{
				lastEnd = newInterval.end;
			}

			// Add interval
			// Need more capacity?
			if (numIntervals + 1 > capacityIntervals)
			{
				capacityIntervals = 15 * capacityIntervals / 10 + 1;	// Grow by ~1.5x
				intervals = (interval_t *)realloc(intervals, capacityIntervals * sizeof(interval_t));
			}
			intervals[numIntervals] = newInterval;
			numIntervals++;
		}
		else if (tokens > 0)	// Ignore completely blank lines
		{
			fprintf(stderr, "WARNING: Too-few columns, ignoring row on line %d.\n", CsvLineNumber(&csv));
		}
	}

	// Set return
	times->intervals = intervals;
	times->numIntervals = numIntervals;

	return err;
}


int OmSummaryRun(omsummary_settings_t *settings)
{
	// Load times
	fprintf(stderr, "Opening times: %s\n", settings->timesFilename);
	times_t times;
	if (TimesLoad(&times, settings->timesFilename) != 0)
	{
		fprintf(stderr, "ERROR: There was a problem with the times data: %s\n", settings->timesFilename);
	}

	// 

	// Load data
	csv_load_t csv;
	int colStart = -1, colEnd = -1, colDuration = -1;
	if (settings->filename != NULL && settings->filename[0] != '\0')
	{
		fprintf(stderr, "Opening data: %s\n", settings->filename);
	}
	int headerCells = CsvOpen(&csv, settings->filename, CSV_HEADER_DETECT_NON_NUMERIC, CSV_SEPARATORS);
	if (headerCells > 0)
	{
		// Parse header cells
		int i;
		for (i = 0; i < headerCells; i++)
		{
			const char *heading = CsvTokenString(&csv, i);

			if (!_strcasecmp(heading, "Start")) { colStart = i; }
			else if (!_strcasecmp(heading, "End")) { colEnd = i; }
			else if (!_strcasecmp(heading, "Duration(s)")) { colDuration = i; }
			else
			{
				fprintf(stderr, "WARNING: Unknown data column %d heading: '%s'.\n", i + 1, heading);
			}
		}
	}

	if (colStart < 0 && colEnd < 0 && colDuration < 0)
	{
		fprintf(stderr, "WARNING: No recognized data heading line -- default columns will be used.\n");
		colStart = 0;
		colEnd = 1;
		colDuration = 2;
	}

	if (colStart < 0)
	{
		fprintf(stderr, "ERROR: One or more required data columns ('start') is missing.\n");
	}


	int currentTime = 0;
	int tokens;
	while ((tokens = CsvReadLine(&csv)) >= 0)
	{
		if (tokens > colStart)
		{
			// Event time
			double start = TimeParse(CsvTokenString(&csv, colStart));

			// Default to an instantaneous event if no end
			double end = start;
			double duration = 0;

			// When given end time
			if (colEnd >= 0 && tokens > colEnd)
			{
				end = TimeParse(CsvTokenString(&csv, colEnd));
				duration = end - start;
			}

			// When given a specific duration, use that
			if (colDuration >= 0 && tokens > colDuration)
			{
				duration = CsvTokenFloat(&csv, colDuration);
				if (end != start && fabs(duration - (end - start)) > 0.01)
				{
					fprintf(stderr, "WARNING: Duration does not match (end - start) on data line %d.", CsvLineNumber(&csv));
				}
			}

//fprintf(stderr, "@%s, %f\n", TimeString(start, NULL), duration);

			// If we have any periods left
			while (currentTime < times.numIntervals)
			{
				interval_t *it = &times.intervals[currentTime];
				double localStart = start;
				double localEnd = end;

				// If start before this, advance to it
				if (localStart < it->start)
				{
					localStart = it->start;
				}

				// If start before this, advance to it
				if (localEnd > it->end)
				{
					localEnd = it->end;
				}

				// Interval
				double localDuration = localEnd - localStart;

//fprintf(stderr, "checking interval %d\n", currentTime);

				// If an interval remains
				if (localDuration >= 0.0)
				{

//fprintf(stderr, "within interval %d ", currentTime);
//fprintf(stderr, "(%s - ", TimeString(it->start, NULL)); 
//fprintf(stderr, "%s)", TimeString(it->end, NULL));
//fprintf(stderr, ": %f\n", localDuration);

					if (it->count <= 0)
					{
						it->first = localStart;
					}
					it->last = localEnd;
					it->duration += localDuration;
					it->count++;
				}

				// Time to check the next period
				if (end >= it->end)
				{
					currentTime++; 
					continue;
				}

				break;
			}


		}
		else if (tokens > 0)	// Ignore completely blank lines
		{
			fprintf(stderr, "WARNING: Too-few columns, ignoring row on line %d.\n", CsvLineNumber(&csv));
		}
	}


	// Output data
	FILE *ofp;

	if (settings->outFilename == NULL || settings->outFilename[0] == '\0')
	{
		ofp = stdout;
	}
	else
	{
		fprintf(stderr, "Saving data: %s\n", settings->outFilename);
		ofp = fopen(settings->outFilename, "wt");
	}

	if (ofp == NULL)
	{
		fprintf(stderr, "ERROR: Problem opening CSV file for output: %s\n", settings->outFilename);
		return -1;
	}

	// Write header (with custom separator)
	const char *header = "Label,Start,End,Interval,First,TimeUntilFirst,Last,TimeAfterLast,FirstToLast,Count,Duration,FirstToLastMinusDuration,Proportion";
	const char *separator = ",";
	if (settings->header != NULL)
	{
		header = settings->header;
	}
	if (settings->separator != NULL)
	{
		separator = settings->separator;
	}
	if (header != NULL && header[0] != '\0')
	{
		for (const char *p = header; *p != '\0'; p++)
		{
			if (*p == ',')
			{
				fprintf(ofp, "%s", separator);
			}
			else
			{
				fprintf(ofp, "%c", *p);
			}
		}
		fprintf(ofp, "\n");
	}

	int j = 0;
	for (j = 0; j < times.numIntervals; j++)
	{
		interval_t *it = &times.intervals[j];
		double interval = it->end - it->start;
		double proportion = 0;
		if (interval > 0)
		{
			proportion = it->duration / interval;
		}

		fprintf(ofp, "%s%s", it->label, separator);										// Label
		fprintf(ofp, "%s%s", TimeString(it->start, NULL), separator);					// Start
		fprintf(ofp, "%s%s", TimeString(it->end, NULL), separator);						// End
		fprintf(ofp, "%f%s", interval * settings->scale, separator);					// Interval

		if (it->first <= 0)
		{
			fprintf(ofp, "%s%s", separator, separator);
		}
		else
		{
			fprintf(ofp, "%s%s", TimeString(it->first, NULL), separator);				// First
			fprintf(ofp, "%f%s", (it->first - it->start) * settings->scale, separator); // TimeUntilFirst
		}

		if (it->last <= 0)
		{
			fprintf(ofp, "%s%s", separator, separator);
		}
		else
		{
			fprintf(ofp, "%s%s", TimeString(it->last, NULL), separator);				// Last
			fprintf(ofp, "%f%s", (it->end - it->last) * settings->scale, separator);	// TimeAfterLast
		}

		if (it->first <= 0 || it->last <= 0)
		{
			fprintf(ofp, "%s", separator);
		}
		else
		{
			fprintf(ofp, "%f%s", (it->last - it->first) * settings->scale, separator);	// FirstToLast
		}

		fprintf(ofp, "%d%s", it->count + settings->countOffset, separator);				// Count
		fprintf(ofp, "%f%s", it->duration * settings->scale, separator);				// Duration

		if (it->first <= 0 || it->last <= 0)
		{
			fprintf(ofp, "%s", separator);
		}
		else
		{
			fprintf(ofp, "%f%s", ((it->last - it->first) - it->duration) * settings->scale, separator);	// FirstToLastMinusDuration
		}

		fprintf(ofp, "%f", proportion * settings->scaleProp);							// Proportion

		fprintf(ofp, "\n");

	}

	if (ofp != stdout)
	{
		fclose(ofp);
	}
	//ofp = NULL;

	return 0;
}

