#ifndef _TACGEN_H
#define _TACGEN_H

#include <string>
#include <vector>
#include "error.h"

using namespace std;

/*
*	ADD		0		-> src1, src2, rlt
*	SUB		1		-> src1, src2, rlt
*	MUL		2		-> src1, src2, rlt
*	DIV		3		-> src1, src2, rlt
*	GTR		4		-> src1, src2, rlt
*	GEQ		5		-> src1, src2, rlt
*	EQU		6		-> src1, src2, rlt
*	NEQ		7		-> src1, src2, rlt
*	LEQ		8		-> src1, src2, rlt
*	LES		9		-> src1, src2, rlt
*	ASN		10		-> src1, NULL, rlt
*	LAB		11		-> src1, NULL, NULL (set label)
*	GOTO	12		-> NULL, NULL, rlt (goto branch)
*	IF		13		-> src1, NULL, rlt (ifTrue then goto)
*	IFZ		14		-> src1, NULL, rlt (ifFalse then goto)
*	RD		15		-> src1, NULL, NULL (read from user input)
*	WR		16		-> src1, NULL, NULL (print to the console)
*	CSTI	17		-> src1(name), src2(value)
*	CSTC	18		-> src1(name), src2(value)
*	INT		19		-> src1(name)
*	CHAR	20		-> src1(name)
*	ARRI	21		-> src1(name), src2(size)
*	ARRC	22		-> src1(name), src2(size)
*	FUNC	23		-> src1(name), src2(return type: int/char/void)
*	PARM	24		-> src1(name), src2(type)
*	RET		25		-> rlt(or null)
*	END		26		-> src1(name) (end of def. of function)
*	STORE	27		-> []=, src1(rvalue), src2(index), rlt(array) (x[i]=y)
*	LOAD	28		-> load, src1(array), src2(index), rlt(lvalue) (x=y[i])
*	PUSH	29		-> push, rlt(a function parameter)
*	CALL	30		-> src1(name)				(f())
					-> src1(name), rlt(lvalue)	(a=f())
*/


class TAC {
public:
	enum QuadOpKind {
		ADD, SUB, MUL, DIV,
		GTR, GEQ, EQU, NEQ, LEQ, LES,
		ASN,
		LAB, GOTO, IF, IFZ,
		RD, WR,
		CSTI, CSTC, INT, CHAR, ARRI, ARRC,
		FUNC, PARM, RET, END,
		STORE, LOAD,
		PUSH, CALL
	};
	typedef struct {
		int op;
		string src1;
		string src2;
		string rlt;
	}Quadruple;
	vector<Quadruple> QuadList;
	int QuadNum;
	int labelcnt;
	int varcnt;
	vector<string> TempVars;

	FILE* TACfile;

	TAC();
	void NewQuad(int op, string src1, string src2, string rlt);
	string NewLabel();
	string NewVar();
	string QuadOpr2Str(int op);
	void DisplayAllQuads();
	void OutputTAC();
};

#endif
