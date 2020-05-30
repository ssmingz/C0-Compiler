#include "error.h"
#include "symbolTable.h"
#include "parser.h"
#include <cassert>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

using namespace std;

void SymbolTable::clearLocal() {
	localScopeTable.clear();
	localScopeVarCount.clear();
	globalScope = true;
}

void SymbolTable::define(const string& name, const string& type, SymbolKind kind, int value, int size) {
	/*
	if (globalScopeTable.size() >= MAX_SYM_SIZE) {
		error(OUT_OF_TABLE_ERROR, token.line);
		return;
	}
	*/
	if (kind == SymbolTable::SymbolKind::FUNCTION) {
		if (globalScopeTable.find(name) != globalScopeTable.end()) {
			error(FUNC_REDEF_ERROR, token.line);
			return;
		}
		globalScopeTable.emplace(name, IdentifierEntry{ kind, type, value, size, globalScopeVarCount[kind]++ });
	}
	else {
		if (!globalScope) {
			if (localScopeTable.find(name) != localScopeTable.end()) {
				error(VAR_REDEF_ERROR, token.line);
				exit(-1);
				return;
			}
			localScopeTable.emplace(name, IdentifierEntry{ kind, type, value, size, localScopeVarCount[kind]++ });
		}
		else {
			if (globalScopeTable.find(name) != globalScopeTable.end()) {
				error(VAR_REDEF_ERROR, token.line);
				exit(-1);
				return;
			}
			globalScopeTable.emplace(name, IdentifierEntry{ kind, type, value, size, localScopeVarCount[kind]++ });
		}
	}
	return;
}

int SymbolTable::varCount(SymbolKind kind) {
	if (globalScope) {
		auto it = globalScopeVarCount.find(kind);
		if (it != globalScopeVarCount.cend()) {
			return it->second;
		}
	}
	else {
		auto it = localScopeVarCount.find(kind);
		if (it != localScopeVarCount.cend()) {
			return it->second;
		}
	}
	return 0;
}

SymbolTable::SymbolKind SymbolTable::kindOf(const string& name, string localName) {
	if (localName == "NULL") {
		if (!globalScope) {
			auto it = localScopeTable.find(name);
			if (it != localScopeTable.cend()) {
				return it->second.kind;
			}
		}
		auto it = globalScopeTable.find(name);
		if (it != globalScopeTable.cend()) {
			return it->second.kind;
		}
	}
	else {
		auto it = localScopeTables.find(localName);
		if (it != localScopeTables.cend()) {
			auto p = it->second.find(name);
			if (p != it->second.cend()) {
				return p->second.kind;
			}
			else {
				p = globalScopeTable.find(name);
				if (p != globalScopeTable.cend()) {
					return p->second.kind;
				}
			}
		}
	}
	return SymbolKind::NONE;
}

string SymbolTable::typeOf(const string& name, string localName) {
	if (localName == "NULL") {
		if (!globalScope) {
			auto it = localScopeTable.find(name);
			if (it != localScopeTable.cend()) {
				return it->second.type;
			}
		}
		auto it = globalScopeTable.find(name);
		if (it != globalScopeTable.cend()) {
			return it->second.type;
		}
	}
	else {
		auto it = localScopeTables.find(localName);
		if (it != localScopeTables.cend()) {
			auto p = it->second.find(name);
			if (p != it->second.cend()) {
				return p->second.type;
			}
			else {
				p = globalScopeTable.find(name);
				if (p != globalScopeTable.cend()) {
					return p->second.type;
				}
			}
		}
	}
	throw runtime_error{ "Symbol-Table: " + name + " does not exist." };
}

int SymbolTable::valueOf(const string& name, string localName) {
	if (localName == "NULL") {
		if (!globalScope) {
			auto it = localScopeTable.find(name);
			if (it != localScopeTable.cend()) {
				return it->second.value;
			}
		}
		auto it = globalScopeTable.find(name);
		if (it != globalScopeTable.cend()) {
			return it->second.value;
		}
	}
	else {
		auto it = localScopeTables.find(localName);
		if (it != localScopeTables.cend()) {
			auto p = it->second.find(name);
			if (p != it->second.cend()) {
				return p->second.value;
			}
			else {
				p = globalScopeTable.find(name);
				if (p != globalScopeTable.cend()) {
					return p->second.value;
				}
			}
		}
	}
	throw runtime_error{ "Symbol-Table: " + name + " does not exist." };
}

int SymbolTable::sizeOf(const string& name, string localName) {
	if (localName == "NULL") {
		if (!globalScope) {
			auto it = localScopeTable.find(name);
			if (it != localScopeTable.cend()) {
				return it->second.size;
			}
		}
		auto it = globalScopeTable.find(name);
		if (it != globalScopeTable.cend()) {
			return it->second.size;
		}
	}
	else {
		auto it = localScopeTables.find(localName);
		if (it != localScopeTables.cend()) {
			auto p = it->second.find(name);
			if (p != it->second.cend()) {
				return p->second.size;
			}
			else {
				p = globalScopeTable.find(name);
				if (p != globalScopeTable.cend()) {
					return p->second.size;
				}
			}
		}
	}
	throw runtime_error{ "Symbol-Table: " + name + " does not exist." };
}

int SymbolTable::indexOf(const string& name, string localName) {
	if (localName == "NULL") {
		if (!globalScope) {
			auto it = localScopeTable.find(name);
			if (it != localScopeTable.cend()) {
				return it->second.index;
			}
		}
		auto it = globalScopeTable.find(name);
		if (it != globalScopeTable.cend()) {
			return it->second.index;
		}
	}
	else {
		auto it = localScopeTables.find(localName);
		if (it != localScopeTables.cend()) {
			auto p = it->second.find(name);
			if (p != it->second.cend()) {
				return p->second.index;
			}
			else {
				p = globalScopeTable.find(name);
				if (p != globalScopeTable.cend()) {
					return p->second.index;
				}
			}
		}
	}
	throw runtime_error{ "Symbol-Table " + name + " does not exist." };
}

const unordered_map <SymbolTable::SymbolKind, string> SYMBOL_KIND_TO_STRING{
			{ SymbolTable::SymbolKind::FUNCTION, "FUNC" },
			{ SymbolTable::SymbolKind::ARG, "ARG" },
			{ SymbolTable::SymbolKind::VAR,  "VAR" },
			{ SymbolTable::SymbolKind::CONST, "CONST" },
			{ SymbolTable::SymbolKind::NONE, "NONE" }
};

void SymbolTable::printGlobalSymbolTable() {
	cout << "**********************************global symbol table**********************************" << endl;
	cout.setf(ios::left);
	cout << setw(16) << "name" << setw(16) << "kind" << setw(16) << "type" << setw(16) << "value" << setw(16) << "size" << setw(16) << "index" << endl;
	for (auto it = globalScopeTable.cbegin(); it != globalScopeTable.cend(); it++) {
		cout.setf(ios::left);
		cout << setw(16) << it->first << setw(16) << (SYMBOL_KIND_TO_STRING.find(it->second.kind))->second << setw(16) << it->second.type << setw(16)
			<< it->second.value << setw(16) << it->second.size << setw(16) << it->second.index << endl;
	}
	cout << endl << endl;
}

void SymbolTable::printLocalSymbolTable() {
	for (auto it = localScopeTables.cbegin(); it != localScopeTables.cend(); it++) {
		cout << "**********************************local symbol table**********************************" << endl;
		cout << "function: " << it->first << endl;
		cout.setf(ios::left);
		cout << setw(16) << "name" << setw(16) << "kind" << setw(16) << "type" << setw(16) << "value" << setw(16) << "size" << setw(16) << "index" << endl;
		for (auto it2 = it->second.cbegin(); it2 != it->second.cend(); it2++) {
			cout.setf(ios::left);
			cout << setw(16) << it2->first << setw(16) << (SYMBOL_KIND_TO_STRING.find(it2->second.kind))->second << setw(16) << it2->second.type << setw(16)
				<< it2->second.value << setw(16) << it2->second.size << setw(16) << it2->second.index << endl;
		}
		cout << endl << endl;
	}
}
