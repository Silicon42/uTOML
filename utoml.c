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
// if it's odd, it was escaped, if it's even, it wasn't
inline bool isRewindBackslashCountEven(char const* src)
{
	int rewind = 0;
	while(src[--rewind] == '\\');
	// odd/even flipped because rewind is decremented before checking
	return rewind & 1;
}

// get pointer to closing double quote for basic string, or failing that,
// pointer to the next newline or null character to indicate string failed to close
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

inline bool isValidBareKeyChar(char c)
{
	char lower = c | 0x20;
	if(lower < 'a' || lower > 'z')		// if not an ascii letter
		if(c < '0' || c > '9')			// and not an ascii number
			if(c != '-' && c != '_')	// and not - or _
				return false;			// then it's not a valid character
	return true;	// else it was
}

char* findBareKeyEnd(char const* src)
{
	while(isValidBareKeyChar(*src))
		++src;

	return src;
}

// get pointer to the end of a key/chain of dotted keys
char* findKeyEnd(char const* src)
{
	while
}

// contents: inlcudes both the file 
// expects contents to be aligned to native alignment, this is true by default if it was malloc'd
// assumes contents are null terminated
UTomlTable utomlParse(char* contents, size_t max_size)
{
	char* curr_pos = contents;
	uint16_t line_num = 1;
	// first count keys
	// complicated b/c some values can spill onto subsequent lines, especially arrays and "inline" tables
	// this means you can't just parse the key and move to the next line, you have to verify that the value after the '=' isn't one of those
	// or a multiline string and if it is, you have to match the closing symbol which involves a bracket context stack
	// heading style tables and table arrays suffer from this especially since this means you can't just search for '[' at the start of a line,
	// you also have to make sure it's not part of a value, which involves checking all keys-val pairs in them

}