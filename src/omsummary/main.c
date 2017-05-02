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


int main(int argc, char *argv[])
{
	int i;
	bool help = false;
	int positional = 0;
	int ret;
	omsummary_settings_t settings = { 0 };

	// Default settings
	memset(&settings, 0, sizeof(settings));
	//settings.xyz = -1;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--help") == 0) { help = true; }

		else if (strcmp(argv[i], "-times") == 0) { settings.timesFilename = argv[++i]; }
		else if (strcmp(argv[i], "-out") == 0) { settings.outFilename = argv[++i]; }

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


	if (settings.filename == NULL) { fprintf(stderr, "ERROR: Input file not specified.\n"); help = 1; }

	if (help)
	{
		fprintf(stderr, "Usage: omsummary <input.csv> [-times <times.csv>] [-out <output.csv>]\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "Options:\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "\t-times <times.csv>  Labelled time spans\n");
		fprintf(stderr, "\t-out <output.csv>   Output summary file\n");
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

