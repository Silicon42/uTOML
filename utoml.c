/*
 * MIT License
 * 
 * Copyright (c) 2025 Silicon42 a.k.a. Curtis Stofer
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <string.h>
#include <stdio.h>
#include "utoml.h"


// Returned pointer must be freed when done using, no need to free on error
char* readFileToCstring(char* fname)
{
	FILE* file = fopen(fname, "rb");
	if(!file)
		return NULL;

	// get rough file size and allocate string
	fseek(file, 0, SEEK_END);
	size_t f_size = ftell(file);
	rewind(file);

	char* contents = malloc(f_size + 1);	// +1 to have enough room to add null termination
	if(!contents)
		return NULL;

	// contents may be smaller due to line endings being partially stripped on read
	f_size = fread(contents, sizeof(char), f_size, file);
	fclose(file);
	// terminate the string properly
	contents[f_size] = '\0';

	return contents;
}

// get pointer to next non space or tab character
inline char* findNextNonWhiteSpace(char const* src)
{	return src + strspn(src, " \t");	}

// get pointer to closing single quote for literal string, or failing that,
// pointer to next newline or null character to indicate string failed to close
// assumes src is beyond opening single quote
inline char* findStringLiteralClose(char const* src)
{	return src + strcspn(src, "'\n");	}

// get pointer to closing triple single quote for multi-line literal string,
// if no closing triple single quote found, returns NULL
// assumes src is beyond opening triple single quote
inline char* findStringMultiLineLiteralClose(char const* src)
{	return strstr(src, "'''");	}
/*	//alternate implementation, might be more efficient?
{
	while(1)
	{
		src = strchr(src, '\'');
		if(src[1] == '\'')
		{
			if(src[2] == '\'')
				break;
			++src;
		}
		++src;
	}
	return src;
}*/

// returns true if the backslash count before src is even,
// used for checking if the suspected last character of a basic string was actually escaped
// if it's odd, it was escaped, if even, it wasn't
inline bool isRewindBackslashCountEven(char const* src)
{
	int rewind = 0;
	while(src[--rewind] == '\\');
	// odd/even flipped because rewind is decremented before checking
	return rewind & 1;
}

// get pointer to closing double quote for basic string, or failing that,
// cointer to the next newline or null character to indicate string failed to close
// assumes src is beyond opening double quote
char* findStringBasicClose(char const* src)
{
	while(1)
	{
		// search for a character that can end the string
		src += strcspn(src, "\"\n");
		if(*src != '"')	// if the reason we stopped is not a '"' character, the string wasn't closed properly
			return src;
		if(isRewindBackslashCountEven(src))
			return src;
		// else increment past the current '"' character
		src++;
	}
}

// get pointer to closing triple double quote for multi-line literal string,
// if no closing triple double quote found, returns NULL
// assumes src is beyond opening triple double quote
char* findStringMultiLineBasicClose(char const* src)
{
	while(1)
	{
		src = strstr(src, "\"\"\"");
		if(!src)	
			return NULL;
		if(isRewindBackslashCountEven(src))
			return src;
		// else increment past the current '"' character
		src++;
	}
}

// contents: inlcudes both the file 
// expects contents to be aligned to native alignment, this is true by default if it was malloc'd
// assumes contents are null terminated
UTomlTable utomlParse(char* contents, size_t max_size)
{
	char* curr_pos = contents;

}