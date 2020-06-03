#pragma warning(disable:4996);

#include <iostream>
#include <string>
#include <iomanip>
#include "error.h"
#include "tacgen.h"


using namespace std;

TAC::TAC() {
	QuadNum = 0;
	labelcnt = 0;
	varcnt = 0;
}

void TAC::NewQuad(int op, string src1, string src2, string rlt) {
	QuadList.push_back({op, src1, src2, rlt});
	QuadNum++;
	return;
}

string TAC::NewLabel() {
	string label = "@L_" + to_string(labelcnt);
	labelcnt++;
	return label;
}

string TAC::NewVar() {
	string var = "$T_" + to_string(varcnt);
	TempVars.push_back(var);
	varcnt++;
	return var;
}

string TAC::QuadOpr2Str(int op) {
	switch (op) {
	case 0: return "ADD";
	case 1: return "SUB";
	case 2: return "MUL";
	case 3: return "DIV";
	case 4: return "GTR";
	case 5: return "GEQ";
	case 6: return "EQU";
	case 7: return "NEQ";
	case 8: return "LEQ";
	case 9: return "LES";
	case 10: return "ASSIN";
	case 11: return "LAB";
	case 12: return "GOTO";
	case 13: return "IF";
	case 14: return "IFZ";
	case 15: return "READ";
	case 16: return "WRITE";
	case 17: return "CONSTINT";
	case 18: return "CONSTCHAR";
	case 19: return "INT";
	case 20: return "CHAR";
	case 21: return "INTARRAY";
	case 22: return "CHARARRAY";
	case 23: return "FUNC";
	case 24: return "PARAM";
	case 25: return "RET";
	case 26: return "END";
	case 27: return "STORE";
	case 28: return "LOAD";
	case 29: return "PUSH";
	case 30: return "CALL";
	default: return "";
	}
}

void TAC::DisplayAllQuads() {
	cout.setf(ios::left);
	cout << setw(16) << "Opr" << setw(16) << "Src1" << setw(16) << "Src2" << setw(16) << "Rlt"<<endl;
	for (int i = 0; i < QuadNum; i++) {
		cout << setw(16) << TAC::QuadOpr2Str(QuadList[i].op)
			<< setw(16) << QuadList[i].src1
			<< setw(16) << QuadList[i].src2
			<< setw(16) << QuadList[i].rlt << endl;
	}
}

void TAC::OutputTAC() {
	TACfile = fopen("midcode.txt", "w");
	int len = QuadList.size();
	for (int i = 0; i < len; i++) {
		switch (QuadList[i].op) {
		case ADD:
			fprintf(TACfile, "%s = %s + %s\n", QuadList[i].rlt.c_str(), QuadList[i].src1.c_str(), QuadList[i].src2.c_str());
			break;
		case SUB:
			fprintf(TACfile, "%s = %s - %s\n", QuadList[i].rlt.c_str(), QuadList[i].src1.c_str(), QuadList[i].src2.c_str());
			break;
		case MUL:
			fprintf(TACfile, "%s = %s * %s\n", QuadList[i].rlt.c_str(), QuadList[i].src1.c_str(), QuadList[i].src2.c_str());
			break;
		case DIV:
			fprintf(TACfile, "%s = %s / %s\n", QuadList[i].rlt.c_str(), QuadList[i].src1.c_str(), QuadList[i].src2.c_str());
			break;
		case GTR:
			fprintf(TACfile, "%s = %s > %s\n", QuadList[i].rlt.c_str(), QuadList[i].src1.c_str(), QuadList[i].src2.c_str());
			break;
		case GEQ:
			fprintf(TACfile, "%s = %s >= %s\n", QuadList[i].rlt.c_str(), QuadList[i].src1.c_str(), QuadList[i].src2.c_str());
			break;
		case LES:
			fprintf(TACfile, "%s = %s < %s\n", QuadList[i].rlt.c_str(), QuadList[i].src1.c_str(), QuadList[i].src2.c_str());
			break;
		case LEQ:
			fprintf(TACfile, "%s = %s <= %s\n", QuadList[i].rlt.c_str(), QuadList[i].src1.c_str(), QuadList[i].src2.c_str());
			break;
		case EQU:
			fprintf(TACfile, "%s = %s == %s\n", QuadList[i].rlt.c_str(), QuadList[i].src1.c_str(), QuadList[i].src2.c_str());
			break;
		case NEQ:
			fprintf(TACfile, "%s = %s != %s\n", QuadList[i].rlt.c_str(), QuadList[i].src1.c_str(), QuadList[i].src2.c_str());
			break;
		case ASN:
			fprintf(TACfile, "%s = %s\n", QuadList[i].rlt.c_str(), QuadList[i].src1.c_str());
			break;
		case LAB:
			fprintf(TACfile, "%s\n", QuadList[i].rlt.c_str());
			break;
		case GOTO:
			fprintf(TACfile, "GOTO %s\n", QuadList[i].rlt.c_str());
			break;
		case IFZ:
			fprintf(TACfile, "IFZ %s %s\n", QuadList[i].src1.c_str(), QuadList[i].rlt.c_str());
			break;
		case RD:
			fprintf(TACfile, "SCANF %s\n", QuadList[i].src1.c_str());
			break;
		case WR:
			if(QuadList[i].src2 == "")
				fprintf(TACfile, "PRINTF %s\n", QuadList[i].src1.c_str());
			else
				fprintf(TACfile, "PRINTF %s %s\n", QuadList[i].src1.c_str(), QuadList[i].src2.c_str());
			break;
		case CSTI:
			fprintf(TACfile, "CONST INT %s %s\n", QuadList[i].src1.c_str(), QuadList[i].src2.c_str());
			break;
		case CSTC:
			fprintf(TACfile, "CONST CHAR %s %s\n", QuadList[i].src1.c_str(), QuadList[i].src2.c_str());
			break;
		case INT:
			fprintf(TACfile, "INT %s\n", QuadList[i].src1.c_str());
			break;
		case CHAR:
			fprintf(TACfile, "CHAR %s\n", QuadList[i].src1.c_str());
			break;
		case ARRI:
			fprintf(TACfile, "ARRAY INT %s %s\n", QuadList[i].src1.c_str(), QuadList[i].src2.c_str());
			break;
		case ARRC:
			fprintf(TACfile, "ARRAY CHAR %s %s\n", QuadList[i].src1.c_str(), QuadList[i].src2.c_str());
			break;
		case FUNC:
			fprintf(TACfile, "FUNC %s %s\n", QuadList[i].src2.c_str(), QuadList[i].src1.c_str());
			break;
		case PARM:
			fprintf(TACfile, "PARM %s %s\n", QuadList[i].src2.c_str(), QuadList[i].src1.c_str());
			break;
		case RET:
			if (QuadList[i].rlt == "")
				fprintf(TACfile, "RT\n");
			else
				fprintf(TACfile, "RT %s\n", QuadList[i].rlt.c_str());
			break;
		case END:
			fprintf(TACfile, "END\n");
			break;
		case STORE:
			fprintf(TACfile, "%s[%s] = %s\n", QuadList[i].rlt.c_str(), QuadList[i].src2.c_str(), QuadList[i].src1.c_str());
			break;
		case LOAD:
			fprintf(TACfile, "%s = %s[%s]\n", QuadList[i].rlt.c_str(), QuadList[i].src1.c_str(), QuadList[i].src2.c_str());
			break;
		case PUSH:
			fprintf(TACfile, "PUSH %s\n", QuadList[i].rlt.c_str());
			break;
		case CALL:
			fprintf(TACfile, "CALL %s %s\n", QuadList[i].src1.c_str(), QuadList[i].rlt.c_str());
			break;
		default:
			fprintf(TACfile, "UNDEFINED TAC OPT SYMBOL\n");
			error(TAC_ERROR, -1);
			cout << "Generate midcode.txt unsuccessfully!" << endl;
			return ;
		}
	}
	cout << "Generate midcode.txt successfully!" << endl;
}