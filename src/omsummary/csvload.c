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

// CSV Load
// Dan Jackson

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#endif

#include <stdlib.h>
#include <string.h>

#include "csvload.h"


// Open a CSV file and optionally load the header line
int CsvOpen(csv_load_t *csv, const char *filename, csv_header_t header)
{
	memset(csv, 0, sizeof(csv_load_t));
	if (filename == NULL || filename[0] == '\0')
	{
		fprintf(stderr, "ERROR: CSV file not specified.\n");
		return false;
	}
	csv->fp = fopen(filename, "rt");
	if (csv->fp == NULL) 
	{ 
		fprintf(stderr, "ERROR: Problem opening CSV file: %s\n", filename);
		return false;
	}
	csv->lineNumber = 0;
	
	// If we have a header
	csv->numTokens = 0;
	if (header != CSV_HEADER_NONE)
	{
		// Read (potential) header line
		CsvReadLine(csv);

		// CSV_HEADER_DETECT_NON_NUMERIC = Automatically detect if each non-empty value is non-numeric
		if (header != CSV_HEADER_ALWAYS)
		{
			bool allEmpty = true;
			bool anyNumerical = false;
			for (int i = 0; i < csv->numTokens; i++)
			{
				char c = csv->tokens[i][0];
				if (c != '\0') { allEmpty = false; }
				if (c == '-' || (c >= '0' && c <= '9')) { anyNumerical = true; }
			}

			// If it wasn't detected as a header, "unread" the first data line
			if (allEmpty || anyNumerical)
			{
				csv->numTokens = 0;
				csv->pushed = true;
			}
		}
	}

	return csv->numTokens;
}


// Read the next row of CSV data, returns number of columns of data (<0 = EOF)
int CsvReadLine(csv_load_t *csv)
{
	// Was a line pushed back ("unread")?
	if (csv->pushed)
	{
		// Return this line instead
		csv->pushed = false;
	}
	else
	{
		// Read line
		if (csv->fp == NULL || fgets(csv->line, sizeof(csv->line) / sizeof(csv->line[0]), csv->fp) == NULL)
		{
			// End of file
			csv->numTokens = -1;
		}
		else
		{
			csv->lineNumber++;

			// Remove trailing CR/LF
			int len = (int)strlen(csv->line);
			if (len > 0 && csv->line[len - 1] == '\n') { csv->line[len - 1] = '\0'; len--; }
			if (len > 0 && csv->line[len - 1] == '\r') { csv->line[len - 1] = '\0'; len--; }

			// TODO: write a custom parser to cope with quoted strings including commas
			// Parse comma-separated tokens
			csv->numTokens = 0;
			const char *tokens = ",\r\n";
			for (char *token = strtok(csv->line, tokens); token != NULL; token = strtok(NULL, tokens))
			{
				if (csv->numTokens < sizeof(csv->tokens) / sizeof(csv->tokens[0]))
				{
					csv->tokens[csv->numTokens] = token;
					csv->numTokens++;
				}
				else
				{
					fprintf(stderr, "WARNING: Too many columns in CSV on line %d, ignoring after token %d.\n", csv->lineNumber, csv->numTokens);
					break;
				}
			}
		}
	}
	return csv->numTokens;
}


// Get current line number of CSV file
int CsvLineNumber(csv_load_t *csv)
{
	return csv->lineNumber;
}


// Get number of CSV tokens on the current line
int CsvTokenCount(csv_load_t *csv)
{
	return csv->numTokens;
}


// Get CSV token string
char *CsvTokenString(csv_load_t *csv, int index)
{
	if (index < 0 || index >= csv->numTokens)
	{
		return "";
	}
	return csv->tokens[index];
}


// Get CSV token integer
int CsvTokenInt(csv_load_t *csv, int index)
{
	return (int)strtol(CsvTokenString(csv, index), NULL, 0);
}


// Get CSV token double
double CsvTokenFloat(csv_load_t *csv, int index)
{
	return atof(CsvTokenString(csv, index));
}


// Close CSV file
void CsvClose(csv_load_t *csv)
{
	csv->numTokens = -1;
	if (csv->fp != NULL)
	{
		fclose(csv->fp);
		csv->fp = NULL;
	}
}

