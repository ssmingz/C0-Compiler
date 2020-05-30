#ifndef _ANALYZER_H
#define _ANALYZER_H

#include "error.h"
#include "parser.h"
#include "symbolTable.h"
#include <vector>
#include <string>

class Analyzer {
public:
	Parser::TreeNode* tree;
	SymbolTable symbolTable;
	void checkStatement(Parser::TreeNode* t, string const& localName = "NULL");
	void checkArguments(Parser::TreeNode* t, string const& functionName, int line);
	Analyzer(Parser::TreeNode* t, SymbolTable table);
};

#endif
