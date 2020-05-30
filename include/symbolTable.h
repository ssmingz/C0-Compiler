#ifndef _SYMBOL_TABLE_H
#define _SYMBOL_TABLE_H

#include "error.h"
#include <iostream>
#include <unordered_map>
#include <vector>
#include <map>
#include <string>

using namespace std;

#define MAX_SYM_SIZE 1024

class SymbolTable {
public:
	enum class SymbolKind { FUNCTION, ARG, VAR, CONST, NONE };
	bool globalScope = true;
	struct IdentifierEntry {
		SymbolKind kind;
		string type;	//void, int, char
		int value;		//const: value, ascii code
						//others: 0
		int size;		//function: number of para
						//array: number of elements
						//others: -1
		int index;		//index assigned to the named identifier
	};
	unordered_map<string, IdentifierEntry> globalScopeTable;
	unordered_map<SymbolKind, int> globalScopeVarCount;
	unordered_map<string, IdentifierEntry> localScopeTable; //for current use
	unordered_map<SymbolKind, int> localScopeVarCount;
	map<string, unordered_map<string, IdentifierEntry> > localScopeTables; //save the produced localScopeTables

public:
	void clearLocal();
	void define(const string& name, const string& type, SymbolKind kind, int value, int size);
	int varCount(SymbolKind kind);
	SymbolKind kindOf(const string& name, string localName = "NULL");
	string typeOf(const string& name, string localName = "NULL");
	int valueOf(const string& name, string localName = "NULL");
	int sizeOf(const string& name, string localName = "NULL");
	int indexOf(const string& name, string localName = "NULL");
	void printGlobalSymbolTable();
	void printLocalSymbolTable();
};

#endif
