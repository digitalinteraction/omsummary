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


// TODO: Add tab-separated optoin (with auto-detection)
// TODO: Add proper parsing of quoted strings (including quotes-within-quotes, possibly strings containing new-lines?)
// TODO: Add specific header size (e.g. allow specified count, detect on specific string, and detect '---' prefix and treat as header lines until similar found -- requires multi-line detection).


#include <stdbool.h>
#include <stdio.h>

#define CSV_MAX_LINE	1024
#define CSV_MAX_TOKENS	128

typedef struct
{
	FILE *fp;							// CSV file pointer
	int lineNumber;						// Current line number
	char line[CSV_MAX_LINE];			// Line buffer
	char *tokens[CSV_MAX_TOKENS];		// Parsed token pointers
	int numTokens;						// Token count
	bool pushed;						// The last line was "unread", return again
} csv_load_t;

int CsvLineNumber(csv_load_t *csv);

int CsvTokenCount(csv_load_t *csv);

char *CsvTokenString(csv_load_t *csv, int index);

int CsvTokenInt(csv_load_t *csv, int index);

double CsvTokenFloat(csv_load_t *csv, int index);

typedef enum
{
	CSV_HEADER_DETECT_NON_NUMERIC = -1,	// Automatically detect if each non-empty value is non-numeric
	CSV_HEADER_NONE = 0,				// No header in CSV file
	CSV_HEADER_ALWAYS = 1,				// Always header in CSV file
} csv_header_t;

int CsvOpen(csv_load_t *csv, const char *filename, csv_header_t header);
int CsvReadLine(csv_load_t *csv);
void CsvClose(csv_load_t *csv);

