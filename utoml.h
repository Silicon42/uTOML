#ifndef UTOML_H
#define UTOML_H
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

#include <stdint.h>
#include <stdbool.h>

typedef uint16_t	UTomlHandle;
typedef UTomlHandle	UTomlTable;
typedef char		UTomlError;
//TODO: lookup typedefs that prevent casting?
typedef enum _UTomlTypeEnum{
	UTOML_INVALID = 0,
	UTOML_BOOL,
	UTOML_INT,
	UTOML_FLOAT,
	UTOML_STRING,
	UTOML_TABLE,
	UTOML_ARRAY,
	UTOML_MIXED,	// only valid as sub-type for array
	UTOML_EMPTY,	// only valid as sub-type for array
	//UNUSED
	//UNUSED
	//UNUSED
	UTOML_DATE_TIME = 12,
	UTOML_TIME,
	UTOML_DATE,
	UTOML_OFFSET_DATE_TIME,
} UTomlTypeEnum;

typedef struct __attribute__((packed)) _UTomlValType{
	UTomlTypeEnum main:4;
	UTomlTypeEnum  sub:4;
} UTomlValType;

enum UTomlErrVal{
	UTOML_OK = 0,				// no error
	UTOML_ERR_OUT_OF_MEMORY,	//
	UTOML_ERR_MISSING_CLOSE,	// closing quote/square bracket/curly brace
};

#endif//UTOML_H