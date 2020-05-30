#include "analyzer.h"
#include "error.h"
#include <iostream>

using namespace std;

Analyzer::Analyzer(Parser::TreeNode* t, SymbolTable table) {
	symbolTable = table;
	tree = t;
	errorNum = 0;
}

void Analyzer::checkArguments(Parser::TreeNode* t, string const& functionName, int line) {
	int argumentSize = 0;
	for (auto it = t->child.begin(); (it != t->child.end()) && (*it != nullptr); it++) {
		argumentSize++;
	}
	if (argumentSize != symbolTable.sizeOf(functionName))
		error(ARG_SIZE_ERROR, line);
	return;
}

void Analyzer::checkStatement(Parser::TreeNode* t, string const& localName) {
	SymbolTable::SymbolKind ckind;
	int csize;
	string ctype;
	bool flag = false;
	Parser::TreeNode* crtstmt = new Parser::TreeNode;
	crtstmt->nodeType = Parser::NodeType::N_NONE;
	if (t != nullptr) {
		switch (t->nodeType) {
		case Parser::NodeType::N_READ_STMT:
			for (auto it = t->child.cbegin(); it != t->child.cend(); it++) {
				ckind = symbolTable.kindOf((*it)->token.lexeme, localName);
				csize = symbolTable.sizeOf((*it)->token.lexeme, localName);
				if (ckind != SymbolTable::SymbolKind::VAR || csize >= 0)
					error(SCANF_NON_VAR_ERROR, (*it)->token.line);
			}
			break;
		case Parser::NodeType::N_ASSIGN:
			ckind = symbolTable.kindOf(t->child[0]->token.lexeme, localName);
			csize = symbolTable.sizeOf(t->child[0]->token.lexeme, localName);
			if (ckind != SymbolTable::SymbolKind::VAR || csize != -1)
				error(VAR_ASSIGN_ERROR, t->child[0]->token.line);
			break;
		case Parser::NodeType::N_CALL_STMT:
			checkArguments(t->child[1], t->child[0]->token.lexeme, t->child[0]->token.line);
			break;
		case Parser::NodeType::N_MAIN:
			checkStatement(t->child[0], "main");
			break;
		case Parser::NodeType::N_FUNC_DEF:
			ctype = t->child[0]->token.lexeme;
			for (auto it = t->child[3]->child.cbegin(); it != t->child[3]->child.end(); it++) {
				if ((*it)->nodeType == Parser::NodeType::N_STMTS) {
					for (auto p = (*it)->child.cbegin(); p != (*it)->child.cend(); p++) {
						if ((*p)->nodeType == Parser::NodeType::N_RETURN_STMT) {
							crtstmt = *p;
							flag = true;
							break;
						}
					}
				}
			}
			if (ctype != "void" && (flag == false || crtstmt->child[0] == nullptr))
				error(RETURN_FUNCDEF_ERROR, t->child[1]->token.line);
			if (ctype == "void" && crtstmt->child[0] != nullptr)
				error(VOID_RETURN_ERROR, t->child[1]->token.line);
			checkStatement(t->child[3], t->child[1]->token.lexeme);
			break;
		case Parser::NodeType::N_COMP_STMT:
			for (auto it = t->child.cbegin(); it != t->child.cend(); it++) {
				if ((*it)->nodeType == Parser::NodeType::N_STMTS) {
					checkStatement(*it, localName);
				}
			}
			break;
		case Parser::NodeType::N_FOR_INIT:
			ckind = symbolTable.kindOf(t->child[0]->token.lexeme, localName);
			csize = symbolTable.sizeOf(t->child[0]->token.lexeme, localName);
			if (ckind != SymbolTable::SymbolKind::VAR || csize != -1)
				error(VAR_ASSIGN_ERROR, t->child[0]->token.line);
			break;
		case Parser::NodeType::N_FOR_INCREMENT:
			ckind = symbolTable.kindOf(t->child[0]->token.lexeme, localName);
			csize = symbolTable.sizeOf(t->child[0]->token.lexeme, localName);
			if (ckind != SymbolTable::SymbolKind::VAR || csize != -1)
				error(VAR_ASSIGN_ERROR, t->child[0]->token.line);
			ckind = symbolTable.kindOf(t->child[1]->child[0]->token.lexeme, localName);
			csize = symbolTable.sizeOf(t->child[1]->child[0]->token.lexeme, localName);
			if (ckind != SymbolTable::SymbolKind::VAR || csize != -1)
				error(VAR_ASSIGN_ERROR, t->child[0]->token.line);
			break;
		case Parser::NodeType::N_STMTS:
			for (auto it = t->child.cbegin(); it != t->child.cend(); it++) {
				checkStatement(*it, localName);
			}
			break;
		case Parser::NodeType::N_PROGRAM:
			for (auto it = t->child.cbegin(); it != t->child.cend(); it++) {
				if ((*it)->nodeType == Parser::NodeType::N_FUNC_DEF || (*it)->nodeType == Parser::NodeType::N_FUNC_DEF) {
					checkStatement(*it, localName);
				}
			}
		default:
			;
		}
	}
}
