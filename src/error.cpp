#include "error.h"
#include <iostream>

using namespace std;

int errorNum;

bool hasError() { return errorNum; }

void syntaxError(string expected, Scanner::Token token) {
	errorNum++;
	cerr << "Error in line " << token.line
		<< ": expect a " << expected << ", but got a " << token.lexeme.c_str() << " of type " << token.type << "\n";
}

void error(int errorType, int line) {
	switch (errorType) {
	case 1:
		cerr << "line " << line << ": this function name already exists!" << endl;
		errorNum++;
		break;
	case 2:
		cerr << "line " << line << ": this variable name already exists!" << endl;
		errorNum++;
		break;
	case 3:
		cerr << "line " << line << ": constant cannot be assigned!" << endl;
		errorNum++;
		break;
	case 4:
		cerr << "line " << line << ": undefined identifier" << endl;
		errorNum++;
		break;
	case 5:
		cerr << "line " << line << ": identifier wrong type" << endl;
		errorNum++;
		break;
	case 6:
		cerr << "line " << line << ": undefined function call" << endl;
		errorNum++;
		break;
	case 7:
		cerr << "line " << line << ": the num of value para is not equal to defined para" << endl;
		errorNum++;
		break;
	case 8:
		cerr << "line " << line << ": table overflow!" << endl;
		errorNum++;
		break;
	case 9:
		cerr << "line " << line << ": undefined variable or wrong type" << endl;
		errorNum++;
		break;
	case 10:
		cerr << "line " << line << ": undefined array" << endl;
		errorNum++;
		break;
	case 11:
		cerr << "line " << line << ": call a no return-value function in the expression" << endl;
		errorNum++;
		break;
	case 12:
		cerr << "line " << line << ": identifier in read statement is not a normal variable, array or const not allowed" << endl;
		errorNum++;
		break;
	case 13:
		cerr << "line " << line << ": identifier in assign statement is not a normal variable, array name or const not allowed" << endl;
		errorNum++;
		break;
	case 14:
		cerr << "line " << line << ": no return statement or return value in int or char function" << endl;
		errorNum++;
		break;
	case 15:
		cerr << "line " << line << ": in void function, should not return value" << endl;
		errorNum++;
		break;
	case 16:
		cerr << "line " << line << ": index of array out of boundary" << endl;
		errorNum++;
		break;
	}
}
