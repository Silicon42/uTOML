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

// get pointer to end of the line, either '\n' or '\0'
inline char* findLineEnd(char const* src)
{	return src + strcspn(src, "\n");	}

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

// returns true if a character is not one of "{}[]"
inline bool isNotBracket(char c)
{
	c |= 0x20;	// '[' to '{', and ']' to '}'
	c += c & 2;	// '{' to '}'
	return c != '}';
}

// expects a pointer to an opening '{' or '['
// assumes only '{' and '[' characters can nest other semantically relevant
// brackets within them, ie grow the stack, strings are skipped over
char* findClosingBracket(char const* src)
{
	// stack of up to 64 booleans that stores the state of what bracket type to look for
	// push == "<< 1" + "| (0 or 1)",
	// read_head == "& 1",
	// pop == ">> 1",
	//NOTE: this is done to mimimize pollution of the call stack with potentially
	// many recursive layers of findClosingBracket()
	uint64_t stack = !!(*src & 0x20);
	char depth = 1;	//stores the depth of the stack

	while(depth)
	{
		// advance past starting bracket/non-bracket chars until any of the following "\0{}[]"
		while((*(++src)) && isNotBracket(*src));

		switch(*src | 0x20)	//'[' to '{', ']' to '}', and '\0' to ' '(0x20)
		{
		case '}':
			// if bracket was matching type to most recent opening bracket
			if(stack & 1 == !!(*src & 0x20))
			{
				--depth;
				stack >>= 1;
				break;
			}
			//else mismatched closing bracket, fall through to error
		case 0x20:	// missing closing bracket, reached end of file
			return NULL;
		case '{':	// add new open bracket to stack
			if(depth < 64)
			{
				++depth;
				stack = (stack << 1) | !!(*src & 0x20);
			}
			else	// out of room on current boolean stack, recursive call to get a new one
			{
				src = findClosingBracket(src);
				if(!src)
					return NULL;
			}
		}
	}
	return src;
}

inline bool isValidBareKeyChar(char c)
{
	char lower = c | 0x20;
	if(lower < 'a' || lower > 'z')		// if not an ascii letter
		if(c < '0' || c > '9')			// and not an ascii number
			if(c != '-' && c != '_')	// and not - or _
				return 0;			// then it's not a valid character
	return 1;	// else it was
}

char* findBareKeyEnd(char const* src)
{
	while(isValidBareKeyChar(*src))
		++src;

	return src;
}

// variant that allows bare keys to use any character but whitespace '=' or '.'
// which can have semantic meaning in that context
inline char* findBareKeyEndPermissive(char const* src)
{	return src + strcspn(src, ". \t]#\"'=\n");	}

// get pointer to the end of a key/chain of dotted keys, searches for an '='
// that isn't enclosed in a string. Characters after any '"' or '\'' to the
// corresponding closing character are considered to be "in a string" regardless
// of valid position after a '.' or validity of preceding bare keys characters
// aside from those with immediately relevant semantic meaning
//NOTE: only used during getting a count of keys so as to find the '=' such that
// we can check whether the following value is a multi-line one
char* findKeyvalKeyEndPermissive(char const* src)
{
	while(*src != '=')
	{
		switch(*src)
		{
		case '"':
			src = findStringBasicClose(src + 1);
			if(*src != '"')	// if failed to close
			{
				//TODO: emit line error (close fail + bad key)
				return src;
			}
			break;
		case '\'':
			src = findStringLiteralClose(src + 1);
			if(*src != '\'')	// if failed to close
			{
				//TODO: emit line error (close fail + bad key)
				return src;
			}
			break;
		case '#':
		case '\n':
		case '\0':
			//TODO: emit line error (bad key)
			return src;
		//default:	// eat bare-keys/'.'/whitespace/invalid chars
		}
		++src;
	}
	return src;
}

// returns pointer to the end of line after the end of the key
// assumes src points to first non-whitespace char beyond the '='
char* findKeyvalValEndPermissive(char const* src)
{
	switch(*src)
	{
	case '\'':
		if(src[1] == '\'' && src[2] == '\'')
			src = findStringMultiLineLiteralClose(src + 3);
		break;
	case '"':
		if(src[1] == '"' && src[2] == '"')
			src = findStringMultiLineBasicClose(src + 3);
		break;
	case '[':
	case '{':
		src = findClosingBracket(src);
	}
	return findLineEnd(src);
}

UTomlError discoverTopLevelKeys(char const* contents, char* root_tbl_data, size_t max_size)
{
	UTomlHandle* handle_array = (UTomlHandle*)root_tbl_data;
	uint16_t key_cnt = 0;
	UTomlErrVal 

	while()
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