#ifndef _PARSER_H
#define _PARSER_H

#include "error.h"
#include "scanner.h"
#include "symbolTable.h"
#include <string>
#include <vector>
#include <fstream>
#include <deque>

using namespace std;

class SymbolTable;

extern Scanner::Token token; // current token
extern SymbolTable symTable;

const unordered_map <string, char> TOKEN_ESCAPE_STRING_TO_CHAR{
			{"\\a", '\a'},
			{"\\b", '\b'},
			{"\\f", '\f'},
			{"\\n", '\n'},
			{"\\r", '\r'},
			{"\\t", '\t'},
			{"\\v", '\v'},
			{"\\\\", '\\'},
			{"\\?", '\?'},
			{"\\'", '\''},
			{"\\\"", '\"'}
};

class Parser {
public:
	enum NodeType {
		N_NONE,			//0
		N_ARITHOP,		//1
		N_RELOP,		//2
		N_CHAR,			//3
		N_INT,			//4
		N_STRING,		//5
		N_KEYWORD,		//6
		N_CONST_LIST,
		N_CONST_DEC,
		N_CONST_INT,
		N_CONST_CHAR,
		N_VAR_LIST,
		N_VAR_DEC,
		N_VAR,
		N_BASIC_TYPE,
		N_ARRAY,
		N_MAIN,
		N_FUNC_DEF,
		N_RETURN_TYPE,
		N_FUNC_NAME,
		N_PARAM_LIST,
		N_PARAM,
		N_FORMAL_PARAM,
		N_COMP_STMT,
		N_STMTS,
		N_IF_STMT,
		N_CONDITION,
		N_WHILE_STMT,
		N_FOR_STMT,
		N_FOR_INIT,
		N_FOR_INCREMENT,
		N_EXPRESSION,
		N_ASSIGN,
		N_CALL_STMT,
		N_VPARAMS,
		N_READ_STMT,
		N_WRITE_STMT,
		N_RETURN_STMT,
		N_PROGRAM
	};
	class TreeNode {
	public:
		Scanner::Token token;
		vector<TreeNode*> child;
		NodeType nodeType;
		TreeNode() {
			nodeType = N_NONE;
			token.lexeme = "";
			child.push_back(nullptr);
		}
	};

public:
	bool match(string expected);

	TreeNode* parseFactor();
	TreeNode* parseTerm();
	TreeNode* parseExpression();
	TreeNode* parseReturnStmt();
	TreeNode* parseReadStmt();
	TreeNode* parseWriteStmt();
	TreeNode* parseLeftValue();
	TreeNode* parseAssignStmt();
	TreeNode* parseStatement();
	TreeNode* parseStatements();
	TreeNode* parseConstDec();
	TreeNode* parseConstList();
	TreeNode* parseType();
	TreeNode* parseVarDec();
	TreeNode* parseVarList();
	TreeNode* parseCompoundStatement();
	TreeNode* parseMainFunction();
	TreeNode* parseValueParameters();
	TreeNode* parseCallStmt();
	TreeNode* parseParameterList();
	TreeNode* parseFunctionDef();
	TreeNode* parseCondition();
	TreeNode* parseIfStmt();
	TreeNode* parseLoopStmt();
	TreeNode* parseProgram();

public:
	string ParserFilename;
	Scanner scanner;
	TreeNode* syntaxTree;
	SymbolTable symTable;
	Parser(const string& filename);
	string traversingTree2str(Parser::TreeNode* a);
	void printSyntaxTree(Parser::TreeNode* tree);
};

#endif
