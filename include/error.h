#ifndef _ERROR_H
#define _ERROR_H

#include "scanner.h"

#define FUNC_REDEF_ERROR 1
#define VAR_REDEF_ERROR 2
#define CONST_ASSIGNMENT_ERROR 3
#define ID_NO_DEF_ERROR 4
#define ID_TYPE_ERROR 5
#define FUNC_NO_DEF_ERROR 6
#define ARG_SIZE_ERROR 7
#define OUT_OF_TABLE_ERROR 8
#define VAR_NO_DEF_ERROR 9
#define ARRAY_NO_DEF_ERROR 10
#define CALL_NO_RETURN_FUNC_ERROR 11
#define SCANF_NON_VAR_ERROR 12
#define VAR_ASSIGN_ERROR 13
#define RETURN_FUNCDEF_ERROR 14
#define VOID_RETURN_ERROR 15
#define ARRAY_INDEX_ERROR 16
#define TAC_ERROR 17

extern int errorNum;

bool hasError();
void syntaxError(string expected, Scanner::Token token);
void error(int errorType, int line);

#endif
