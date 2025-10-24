/*
 * global.h
 * Stuff that's global to a lot of the compiler.
 *
 * Author: Saumya Debray
 */

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "error.h"
#include "util.h"

#define t_Char   0
#define t_Int    1
#define t_Addr   2
#define t_Bool   3
#define t_Array  4
#define t_Func   5
#define t_None   6
#define t_Error  7

#endif  /* _GLOBAL_H_ */
