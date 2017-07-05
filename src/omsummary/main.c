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
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "omsummary.h"


static double scale(const char *str)
{
	//fprintf(stderr, "SCALE: string=%s\n", str);
	char *end = NULL;
	double value = strtod(str, &end);
	while (end != NULL && *end != '\0' && *end == '/')
	{
		double divisor = strtod(end + 1, &end);
		if (divisor == 0) 
		{ 
			fprintf(stderr, "WARNING: Invalid scale has divide by zero.\n");
			value = 0.0;
		}
		else 
		{
			value /= divisor;
		}
	}
	//fprintf(stderr, "SCALE: value=%f\n", value);
	return value;
}


int main(int argc, char *argv[])
{
	int i;
	bool help = false;
	int positional = 0;
	int ret;
	omsummary_settings_t settings = { 0 };

	// Default settings
	memset(&settings, 0, sizeof(settings));
	settings.scale = 1.0f;				// "1/60" for minutes
	settings.scaleProp = 1.0f;			// "100" for percentage
	settings.countOffset = 0;			// "-1" to report count-1
	settings.header = NULL;
	settings.separator = NULL;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--help") == 0) { help = true; }

		else if (strcmp(argv[i], "-in") == 0) { settings.filename = argv[++i]; }
		else if (strcmp(argv[i], "-times") == 0) { settings.timesFilename = argv[++i]; }
		else if (strcmp(argv[i], "-out") == 0) { settings.outFilename = argv[++i]; }

		else if (strcmp(argv[i], "-mode:sleep") == 0) 
		{ 
			settings.scale = 1.0 / 60.0;
			settings.scaleProp = 100.0;
			settings.countOffset = -1;
			settings.header = "Label,Start,End,TimeInBed,SleepTime,SleepOnsetLatency,WakeTime,TimeToGetUp,FirstSleepToLastWakeTime,Awakenings,TotalSleepTime,WakeAfterSleepOnset,SleepEfficiency";

			// Interval: "Time in bed" = (end - start)
			// (First : "Time First Asleep")
			// TimeUntilFirst : "Sleep onset latency" = (first - start)
			// (Last : "Time Last Awoke")
			// (TimeAfterLast : "Time to get up")
			// FirstToLast : "First sleep to last wake time" (last - first) range within period
			// Count(must use "-countoffset -1") : "Number of awakenings" = (COUNT - 1)
			// Duration : "Total sleep time" = (SUM)total within period
			// FirstToLastMinusDuration : "Wake time after sleep onset (WASO)" = FirstToLast - Duration = (last - first) - (SUM)total within period
			// Proportion(must use "-scaleprop 100") : "Sleep efficiency" = 100 * SUM / (end - start)
		}

		else if (strcmp(argv[i], "-scale") == 0) { settings.scale = scale(argv[++i]); }
		else if (strcmp(argv[i], "-scaleprop") == 0) { settings.scaleProp = scale(argv[++i]); }
		else if (strcmp(argv[i], "-countoffset") == 0) { settings.countOffset = atoi(argv[++i]); }
		else if (strcmp(argv[i], "-header") == 0) { settings.header = argv[++i]; }
		else if (strcmp(argv[i], "-separator") == 0)
		{
			settings.separator = argv[++i];
			if (strcmp(settings.separator, "\\t") == 0) {
				settings.separator = "\t";
			}
		}
		
		else if (argv[i][0] == '-')
		{
			fprintf(stderr, "Unknown option: %s\n", argv[i]);
			help = 1;
		}
		else
		{
			if (positional == 0)
			{
				settings.filename = argv[i];
			}
			else
			{
				fprintf(stderr, "Unknown positional parameter (%d): %s\n", positional + 1, argv[i]);
				help = 1;
			}
			positional++;
		}
	}


	if (settings.timesFilename == NULL) { fprintf(stderr, "ERROR: Times file not specified.\n"); help = 1; }

	if (help)
	{
		fprintf(stderr, "omsummary OM Summary Tool\n");
		fprintf(stderr, "V1.03\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "Usage: omsummary [[-in] <input.csv>] -times <times.csv> [-out <output.csv>] [-scale <scale>] [-scaleprop <scale>] [-header <header>]\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "Options:\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "\t[-in] <input.csv>       Input file (defaults to stdin)\n");
		fprintf(stderr, "\t-times <times.csv>      Labelled time spans\n");
		fprintf(stderr, "\t-out <output.csv>       Output file (defaults to stdout)\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "\t-mode:sleep             Use settings for sleep\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "\t-scale <scale>          Time scaling, for minutes: 1/60\n");
		fprintf(stderr, "\t-scaleprop <scale>      Proportion scaling, for percent: 100\n");
		fprintf(stderr, "\t-countoffset <offset>   Offset to apply to count, e.g. -1\n");
		fprintf(stderr, "\t-header <header>        Custom output header line\n");
		fprintf(stderr, "\t-separator <character>  Custom output field separator\n");
		fprintf(stderr, "\n");
		ret = -1;
	}
	else
	{
		// Run summary
		ret = OmSummaryRun(&settings);
	}

#if defined(_WIN32) && defined(_DEBUG)
	if (IsDebuggerPresent()) { fprintf(stderr, "\nPress [enter] to exit <%d>....", ret); getc(stdin); }
#endif

	return ret;
}

