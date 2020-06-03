#include "parser.h"
#include <cassert>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <set>

using namespace std;

Scanner::Token token; // current token
SymbolTable symTable;
TAC tac;
string midresult;

Parser::Parser(const string& filename) {
	this->ParserFilename = filename;
	syntaxTree = nullptr;
}

bool Parser::match(string expected) {
	bool flag = true;
	token = this->scanner.nextToken();
	if (token.lexeme != expected) {
		syntaxError(expected, token);
		flag = false;
	}
	return flag;
}

Parser::TreeNode* Parser::parseProgram() {
	scanner.openFile(ParserFilename);
	Parser::TreeNode* t = new Parser::TreeNode;
	t->nodeType = Parser::NodeType::N_PROGRAM;
	token = scanner.nextToken();
	if (token.lexeme == "const") {
		scanner.ungetToken(token);
		t->child[0] = new Parser::TreeNode;
		t->child[0] = parseConstList();
		token = scanner.nextToken();
	}
	//变量定义和有返回值的函数有相同的声明头部，需预读3个单词
	while (token.lexeme == "int" || token.lexeme == "char") {
		Scanner::Token tmp1, tmp2; //用于恢复现场
		tmp1 = token;
		token = scanner.nextToken();
		if (token.type != Scanner::TokenType::ID) {
			syntaxError("identifier", token);
			return t;
		}
		tmp2 = token;
		token = scanner.nextToken();
		//若为,或[或；就是变量定义
		if (token.lexeme == "," || token.lexeme == "[" || token.lexeme == ";") {
			scanner.ungetToken(token);
			scanner.ungetToken(tmp1);
			scanner.ungetToken(tmp2);
			if (t->child[0] == nullptr) {
				t->child[0] = new Parser::TreeNode;
				t->child[0] = parseVarList();
			}
			else {
				t->child.push_back(parseVarList());
			}
		}
		else {
			scanner.ungetToken(token);
			scanner.ungetToken(tmp2);
			token = tmp1;
			break;
		}
		token = scanner.nextToken();
	}
	//函数部分,这里token=int/char/void
	while (token.lexeme == "int" || token.lexeme == "char" || token.lexeme == "void") {
		Scanner::Token tmp1 = token;
		token = scanner.nextToken();
		if (token.lexeme == "main") {
			scanner.ungetToken(token);
			scanner.ungetToken(tmp1);
			break;
		}
		scanner.ungetToken(token);
		scanner.ungetToken(tmp1);
		if (t->child[0] == nullptr) {
			t->child[0] = new Parser::TreeNode;
			t->child[0] = parseFunctionDef();
		}
		else {
			t->child.push_back(parseFunctionDef());
		}
		token = scanner.nextToken();
	}
	if (t->child[0] == nullptr) {
		t->child[0] = new Parser::TreeNode;
		t->child[0] = parseMainFunction();
	}
	else {
		t->child.push_back(parseMainFunction());
	}
	scanner.closeFile();
	return t;
}

Parser::TreeNode* Parser::parseConstDec() {
	Parser::TreeNode* t = new Parser::TreeNode;
	token = scanner.nextToken();
	if (token.lexeme != "int" && token.lexeme != "char") {
		syntaxError("\"int\" or \"char\"", token);
		return t;
	}
	t->nodeType = Parser::NodeType::N_CONST_DEC;
	int flag = (token.lexeme == "int") ? 0 : 1; //0 for int, 1 for char
	token = scanner.nextToken();
	while (token.type == Scanner::TokenType::ID) {
		Parser::TreeNode* p = new Parser::TreeNode;
		p->token = token;
		p->nodeType = (flag == 0) ? Parser::NodeType::N_CONST_INT : Parser::NodeType::N_CONST_CHAR;
		token = scanner.nextToken();
		if (token.lexeme != "=") {
			syntaxError("=", token);
			return t;
		}
		token = scanner.nextToken();
		if (flag == 0 && token.type != Scanner::TokenType::INT) {
			syntaxError("an integer", token);
			return t;
		}
		if (flag == 1 && token.type != Scanner::TokenType::CHAR) {
			syntaxError("an character", token);
			return t;
		}
		p->child[0] = new Parser::TreeNode;
		p->child[0]->token = token;
		p->child[0]->nodeType = (flag == 0) ? Parser::NodeType::N_INT : Parser::NodeType::N_CHAR;

		//symbol table action
		int cvalue;
		if (flag == 0) cvalue = atoi((p->child[0]->token.lexeme).c_str());
		else {
			string str = (p->child[0]->token.lexeme).c_str();
			auto it = TOKEN_ESCAPE_STRING_TO_CHAR.find(str);
			cvalue = it == TOKEN_ESCAPE_STRING_TO_CHAR.end() ? (int)((char*)(str.data()))[0] : it->second;
		}
		symTable.define(p->token.lexeme, flag == 0 ? "int" : "char", SymbolTable::SymbolKind::CONST, cvalue, (int)-1);

		if (t->child[0] == nullptr) {
			t->child[0] = new Parser::TreeNode;
			t->child[0] = p;
		}
		else {
			t->child.push_back(p);
		}
		token = scanner.nextToken();
		if (token.lexeme == ",") {
			token = scanner.nextToken();
			if (token.type != Scanner::TokenType::ID) {
				syntaxError("identifier", token);
				return t;
			}
		}

		//generate tac
		tac.NewQuad( (flag==0)?(TAC::QuadOpKind::CSTI):(TAC::QuadOpKind::CSTC), p->token.lexeme, p->child[0]->token.lexeme, "");
	}
	scanner.ungetToken(token);
	return t;
}

Parser::TreeNode* Parser::parseConstList() {
	Parser::TreeNode* t = nullptr;
	token = this->scanner.nextToken();
	if (token.lexeme != "const") {
		syntaxError("\"const\"", token);
		return t;
	}
	while (token.lexeme == "const") {
		if (t == nullptr) {
			t = new Parser::TreeNode;
			t->nodeType = Parser::NodeType::N_CONST_LIST;
			t->child[0] = parseConstDec();
		}
		else {
			t->child.push_back(parseConstDec());
		}
		token = scanner.nextToken();
		if (token.lexeme != ";") {
			syntaxError("\';\'", token);
			return t;
		}
		token = scanner.nextToken();
	}
	scanner.ungetToken(token);
	return t;
}

Parser::TreeNode* Parser::parseType() {
	Parser::TreeNode* t = nullptr;
	token = this->scanner.nextToken();
	if (token.lexeme == "int" || token.lexeme == "char") {
		t = new Parser::TreeNode;
		t->nodeType = Parser::NodeType::N_BASIC_TYPE;
		t->token = token;
	}
	else {
		syntaxError("basic type", token);
		return t;
	}
	return t;
}

Parser::TreeNode* Parser::parseVarDec() {
	Parser::TreeNode* t = new Parser::TreeNode;
	t->nodeType = Parser::NodeType::N_VAR_DEC;
	t->child[0] = new Parser::TreeNode;
	t->child[0] = parseType();
	token = this->scanner.nextToken();
	if (token.type != Scanner::TokenType::ID) {
		syntaxError("identifier", token);
		return t;
	}
	while (token.type == Scanner::TokenType::ID) {
		Parser::TreeNode* p = new Parser::TreeNode;
		p->nodeType = Parser::NodeType::N_VAR;
		p->token = token;
		token = scanner.nextToken();
		if (token.lexeme == "[") {
			token = scanner.nextToken();
			if (token.type != Scanner::TokenType::INT || token.lexeme == "0") {
				syntaxError("nonzero integer", token);
				return t;
			}
			p->nodeType = Parser::NodeType::N_ARRAY;
			p->child[0] = new Parser::TreeNode;
			p->child[0]->nodeType = Parser::NodeType::N_INT;
			p->child[0]->token = token;
			token = scanner.nextToken();
			if (token.lexeme != "]") {
				syntaxError("\']\'", token);
				return t;
			}
		}
		else {
			scanner.ungetToken(token);
		}

		//symbol table action
		//generate tac
		if (p->nodeType == Parser::NodeType::N_ARRAY) {
			symTable.define(p->token.lexeme, t->child[0]->token.lexeme, SymbolTable::SymbolKind::VAR, 0, atoi((p->child[0]->token.lexeme).c_str()));
			tac.NewQuad( (t->child[0]->token.lexeme == "int")?(TAC::QuadOpKind::ARRI):(TAC::QuadOpKind::ARRC), p->token.lexeme, (p->child[0]->token.lexeme).c_str(), "");
		}
		else {
			symTable.define(p->token.lexeme, t->child[0]->token.lexeme, SymbolTable::SymbolKind::VAR, 0, (int)-1);
			tac.NewQuad( (t->child[0]->token.lexeme == "int")?(TAC::QuadOpKind::INT):(TAC::QuadOpKind::CHAR), p->token.lexeme, "", "");
		}

		t->child.push_back(p);
		token = scanner.nextToken();
		if (token.lexeme == ",") {
			token = scanner.nextToken();
			if (token.type != Scanner::TokenType::ID) {
				syntaxError("identifier", token);
				return t;
			}
		}
	}
	scanner.ungetToken(token);
	return t;
}

Parser::TreeNode* Parser::parseVarList() {
	Parser::TreeNode* t = nullptr;
	token = scanner.nextToken();
	if (token.lexeme != "int" && token.lexeme != "char") {
		syntaxError("\"int\" or  \"char\"", token);
		return t;
	}
	while (token.lexeme == "int" || token.lexeme == "char") {
		Scanner::Token tmp1, tmp2;
		tmp1 = scanner.nextToken();
		tmp2 = scanner.nextToken();
		if (tmp1.type == Scanner::TokenType::ID && tmp2.lexeme == "(") {
			scanner.ungetToken(tmp2);
			scanner.ungetToken(tmp1);
			break;
		}
		scanner.ungetToken(tmp2);
		scanner.ungetToken(tmp1);
		scanner.ungetToken(token);
		if (t == nullptr) {
			t = new Parser::TreeNode;
			t->nodeType = Parser::NodeType::N_VAR_LIST;
			t->child[0] = parseVarDec();
		}
		else {
			t->child.push_back(parseVarDec());
		}
		token = scanner.nextToken();
		if (token.lexeme != ";") {
			syntaxError("\';\'", token);
			return t;
		}
		token = scanner.nextToken();
	}
	scanner.ungetToken(token);
	return t;
}

Parser::TreeNode* Parser::parseParameterList() {
	Parser::TreeNode* t = new Parser::TreeNode;
	t->nodeType = Parser::NodeType::N_PARAM_LIST;

	//symbol table action
	symTable.globalScope = false;

	token = scanner.nextToken();
	while (token.lexeme == "int" || token.lexeme == "char") {
		scanner.ungetToken(token);
		Parser::TreeNode* p1 = new Parser::TreeNode, * p2 = new Parser::TreeNode;
		p1->nodeType = Parser::NodeType::N_PARAM;
		p1->child[0] = parseType();
		token = scanner.nextToken();
		if (token.type != Scanner::TokenType::ID) {
			syntaxError("identifier", token);
			return t;
		}
		p2->nodeType = Parser::NodeType::N_FORMAL_PARAM;
		p2->token = token;
		p1->child.push_back(p2);

		//symbol table action
		symTable.define(p1->child[1]->token.lexeme, p1->child[0]->token.lexeme, SymbolTable::SymbolKind::ARG, 0, (int)-1);

		//generate tac
		tac.NewQuad(TAC::QuadOpKind::PARM, p1->child[1]->token.lexeme, p1->child[0]->token.lexeme, "");

		if (t->child[0] == nullptr) {
			t->child[0] = new Parser::TreeNode;
			t->child[0] = p1;
		}
		else {
			t->child.push_back(p1);
		}
		token = scanner.nextToken();
		if (token.lexeme == ",") {
			token = scanner.nextToken();
			if (token.lexeme != "int" && token.lexeme != "char") {
				syntaxError("basic type", token);
				return t;
			}
		}
	}
	scanner.ungetToken(token);
	return t;
}

Parser::TreeNode* Parser::parseFunctionDef() {
	Parser::TreeNode* t = new Parser::TreeNode;
	t->nodeType = Parser::NodeType::N_FUNC_DEF;
	token = scanner.nextToken();
	if (token.lexeme != "int" && token.lexeme != "char" && token.lexeme != "void") {
		syntaxError("valid return type", token);
		return t;
	}
	t->child[0] = new Parser::TreeNode;
	t->child[0]->nodeType = Parser::NodeType::N_RETURN_TYPE;
	t->child[0]->token = token;
	token = scanner.nextToken();
	if (token.type != Scanner::TokenType::ID) {
		syntaxError("identifier", token);
		return t;
	}
	t->child.push_back(new Parser::TreeNode);
	t->child[1]->nodeType = Parser::NodeType::N_FUNC_NAME;
	t->child[1]->token = token;

	//generate tac
	tac.NewQuad(TAC::QuadOpKind::FUNC, token.lexeme, t->child[0]->token.lexeme, "");

	//symbol table action
	symTable.define(t->child[1]->token.lexeme, t->child[0]->token.lexeme, SymbolTable::SymbolKind::FUNCTION, 0, (int)-1);

	if (!match("(")) { return t; }
	t->child.push_back(parseParameterList());

	//symbol table action
	symTable.globalScopeTable.find(t->child[1]->token.lexeme)->second.size = t->child[2]->child[0] == nullptr ? 0 : t->child[2]->child.size();

	if (!match(")")) { return t; }
	if (!match("{")) { return t; }
	t->child.push_back(parseCompoundStatement());
	if (!match("}")) { return t; }

	//generate tac
	tac.NewQuad(TAC::QuadOpKind::END, "", "", "");

	//symbol table action
	symTable.localScopeTables.insert({ t->child[1]->token.lexeme, symTable.localScopeTable });
	symTable.clearLocal();

	return t;
}

Parser::TreeNode* Parser::parseMainFunction() {
	Parser::TreeNode* t = new Parser::TreeNode;
	t->nodeType = Parser::NodeType::N_MAIN;
	t->token.lexeme = "main";
	if (!match("void") || !match("main") ||
		!match("(") || !match(")") || !match("{"))
		return t;

	//generate tac
	tac.NewQuad(TAC::QuadOpKind::FUNC, "main", "void", "");

	//symbol table action
	symTable.define("main", "void", SymbolTable::SymbolKind::FUNCTION, 0, (int)-1);

	t->child[0] = new Parser::TreeNode;
	t->child[0] = parseCompoundStatement();
	if (!match("}")) return t;

	//generate tac
	tac.NewQuad(TAC::QuadOpKind::END, "", "", "");

	//symbol table action
	symTable.localScopeTables.insert({ "main", symTable.localScopeTable });
	symTable.clearLocal();

	return t;
}

Parser::TreeNode* Parser::parseCompoundStatement() {
	Parser::TreeNode* t = new Parser::TreeNode;
	t->nodeType = Parser::NodeType::N_COMP_STMT;

	//symbol table action
	symTable.globalScope = false;

	token = scanner.nextToken();
	if (token.lexeme == "const") {
		scanner.ungetToken(token);
		if (t->child[0] == nullptr) {
			t->child[0] = parseConstList();
		}
		else {
			t->child.push_back(parseConstList());
		}
		token = scanner.nextToken();
	}
	if (token.lexeme == "int" || token.lexeme == "char") {
		scanner.ungetToken(token);
		if (t->child[0] == nullptr) {
			t->child[0] = parseVarList();
		}
		else {
			t->child.push_back(parseVarList());
		}
		token = scanner.nextToken();
	}
	scanner.ungetToken(token);
	if (t->child[0] == nullptr) {
		t->child[0] = parseStatements();
	}
	else {
		t->child.push_back(parseStatements());
	}
	return t;
}

Parser::TreeNode* Parser::parseStatements() {
	Parser::TreeNode* t = nullptr;
	token = scanner.nextToken();
	while (token.lexeme == "return" || token.lexeme == "printf" || token.lexeme == "scanf" || token.lexeme == "if" || token.lexeme == "while" || token.lexeme == "for" ||
		token.type == Scanner::TokenType::ID ||
		token.lexeme == "{" || token.lexeme == ";") {
		scanner.ungetToken(token);
		if (t == nullptr) {
			t = new Parser::TreeNode;
			t->nodeType = Parser::NodeType::N_STMTS;
			t->child[0] = parseStatement();
		}
		else {
			t->child.push_back(parseStatement());
		}
		token = scanner.nextToken();
	}
	scanner.ungetToken(token);
	return t;
}

Parser::TreeNode* Parser::parseStatement() {
	Parser::TreeNode* t = new Parser::TreeNode;
	token = scanner.nextToken();
	if (token.lexeme == "return") {
		scanner.ungetToken(token);
		t = parseReturnStmt();
	}
	else if (token.lexeme == "printf") {
		scanner.ungetToken(token);
		t = parseWriteStmt();
	}
	else if (token.lexeme == "scanf") {
		scanner.ungetToken(token);
		t = parseReadStmt();
	}
	else if (token.lexeme == "if") {
		scanner.ungetToken(token);
		t = parseIfStmt();
		return t;
	}
	else if (token.lexeme == "while") {
		scanner.ungetToken(token);
		t = parseLoopStmt();
		return t;
	}
	else if (token.lexeme == "for") {
		scanner.ungetToken(token);
		t = parseLoopStmt();
		return t;
	}
	else if (token.type == Scanner::TokenType::ID) {
		Scanner::Token tmp = token;
		token = scanner.nextToken();
		if (token.lexeme == "=" || token.lexeme == "[") {
			scanner.ungetToken(token);
			scanner.ungetToken(tmp);
			t = parseAssignStmt();
		}
		else if (token.lexeme == "(") {
			scanner.ungetToken(token);
			scanner.ungetToken(tmp);
			t = parseCallStmt();
		}
		else {
			scanner.ungetToken(token);
			scanner.ungetToken(tmp);
			syntaxError("= or [", token);
			return t;
		}
	}
	else if (token.lexeme == "{") {
		t = parseStatements();
		token = scanner.nextToken();
		if (token.lexeme != "}") {
			syntaxError("}", token);
			return t;
		}
		return t;
	}
	// 空语句
	else if (token.lexeme == ";") {
		t->nodeType = Parser::NodeType::N_NONE;
		scanner.ungetToken(token);
	}
	else {
		scanner.ungetToken(token);
		syntaxError("identifier", token);
		return t;
	}
	token = scanner.nextToken();
	if (token.lexeme != ";") {
		syntaxError(";", token);
		return t;
	}
	return t;
}

Parser::TreeNode* Parser::parseReturnStmt() {
	string midvar;
	Parser::TreeNode* t = nullptr;
	token = scanner.nextToken();
	if (token.lexeme != "return") {
		syntaxError("return", token);
		return t;
	}
	Parser::TreeNode* p = new Parser::TreeNode;
	p->token = token;
	p->nodeType = Parser::NodeType::N_RETURN_STMT;
	token = scanner.nextToken();
	if (token.lexeme == "(") {
		t = parseExpression();
		midvar = midresult;
		p->child[0] = t;
		token = scanner.nextToken();
		if (token.lexeme != ")") {
			syntaxError(")", token);
			return t;
		}
		//generate tac
		tac.NewQuad(TAC::QuadOpKind::RET, "", "", midvar);
	}
	else {
		scanner.ungetToken(token);
		//generate tac
		tac.NewQuad(TAC::QuadOpKind::RET, "", "", "");
	}
	t = p;
	return t;
}

Parser::TreeNode* Parser::parseReadStmt() {
	string midname;
	Parser::TreeNode* t = new Parser::TreeNode;
	token = scanner.nextToken();
	if (token.lexeme != "scanf") {
		syntaxError("scanf", token);
		return t;
	}
	t->token = token;
	t->nodeType = Parser::NodeType::N_READ_STMT;
	token = scanner.nextToken();
	if (token.lexeme != "(") syntaxError("(", token);
	else {
		token = scanner.nextToken();
		if (token.type != Scanner::TokenType::ID) syntaxError("identifier", token);
		else {
			t->child[0] = new Parser::TreeNode;
			t->child[0]->token = token;
			t->child[0]->nodeType = Parser::NodeType::N_VAR;
			midname = token.lexeme;

			//check symbol table
			SymbolTable::SymbolKind ckind = symTable.kindOf(token.lexeme);
			if (ckind != SymbolTable::SymbolKind::VAR)
				error(VAR_NO_DEF_ERROR, token.line);

			//generate tac
			tac.NewQuad(TAC::QuadOpKind::RD, "", "", midname);

			token = scanner.nextToken();
			while (token.lexeme == ",") {
				token = scanner.nextToken();
				if (token.type != Scanner::TokenType::ID) {
					syntaxError("identifier", token);
					return t;
				}
				else {
					Parser::TreeNode* p = new Parser::TreeNode;
					p->token = token;
					p->nodeType = Parser::NodeType::N_VAR;
					midname = token.lexeme;

					//check symbol table
					ckind = symTable.kindOf(token.lexeme);
					if (ckind != SymbolTable::SymbolKind::VAR)
						error(VAR_NO_DEF_ERROR, token.line);
					//generate tac
					tac.NewQuad(TAC::QuadOpKind::RD, "", "", midname);

					t->child.push_back(p);
					token = scanner.nextToken();
				}

			}
		}
		if (token.lexeme != ")") syntaxError(")", token);
	}
	return t;
}

Parser::TreeNode* Parser::parseWriteStmt() {
	string midvar1 = "", midvar2 = "";	//1 for string, 2 for expression
	Parser::TreeNode* t = new Parser::TreeNode;
	token = scanner.nextToken();
	if (token.lexeme != "printf") {
		syntaxError("printf", token);
		return t;
	}
	t->token = token;
	t->nodeType = Parser::NodeType::N_WRITE_STMT;
	token = scanner.nextToken();
	if (token.lexeme != "(") syntaxError("(", token);
	else {
		token = scanner.nextToken();
		t->child[0] = new Parser::TreeNode;
		if (token.type == Scanner::TokenType::STRING) {
			midvar1 = token.lexeme;
			t->child[0]->token = token;
			t->child[0]->nodeType = Parser::NodeType::N_STRING;
			token = scanner.nextToken();
			if (token.lexeme == ",") {
				Parser::TreeNode* p = parseExpression();
				midvar2 = midresult;
				t->child.push_back(p);
				token = scanner.nextToken();
			}
		}
		else {
			scanner.ungetToken(token);
			Parser::TreeNode* p = parseExpression();
			midvar2 = midresult;
			t->child[0] = p;
			token = scanner.nextToken();
		}
		if (token.lexeme != ")") syntaxError(")", token);
	}
	//generate tac
	tac.NewQuad(TAC::QuadOpKind::WR, midvar1, midvar2, "");
	return t;
}

Parser::TreeNode* Parser::parseCondition() {
	string midvar1, midvar2;
	Parser::TreeNode* t = new Parser::TreeNode;
	t->nodeType = Parser::NodeType::N_CONDITION;
	Parser::TreeNode* p1 = parseExpression();
	midvar1 = midresult;
	token = scanner.nextToken();
	if (token.lexeme == ">" || token.lexeme == "<" || token.lexeme == ">=" || token.lexeme == "<=" || token.lexeme == "==" || token.lexeme == "!=") {
		Parser::TreeNode* p2 = new Parser::TreeNode;
		p2->nodeType = Parser::NodeType::N_RELOP;
		p2->token = token;
		p2->child[0] = new Parser::TreeNode;
		p2->child[0] = p1;
		p2->child.push_back(parseExpression());
		midvar2 = midresult;
		midresult = tac.NewVar();

		//generate tac
		string relop = p2->token.lexeme;
		if (relop == ">")
			tac.NewQuad(TAC::QuadOpKind::GTR, midvar1, midvar2, midresult);
		else if(relop == "<")
			tac.NewQuad(TAC::QuadOpKind::LES, midvar1, midvar2, midresult);
		else if (relop == ">=")
			tac.NewQuad(TAC::QuadOpKind::GEQ, midvar1, midvar2, midresult);
		else if (relop == "<=")
			tac.NewQuad(TAC::QuadOpKind::LEQ, midvar1, midvar2, midresult);
		else if (relop == "==")
			tac.NewQuad(TAC::QuadOpKind::EQU, midvar1, midvar2, midresult);
		else if (relop == "!=")
			tac.NewQuad(TAC::QuadOpKind::NEQ, midvar1, midvar2, midresult);

		p1 = p2;
	}
	else if (token.lexeme == ")") {
		scanner.ungetToken(token);
		//generate tac
		midresult = tac.NewVar();
		tac.NewQuad(TAC::QuadOpKind::NEQ, midvar1, "", midresult);
	}
	else {
		syntaxError("relation operator", token);
		return t;
	}
	t->child[0] = new Parser::TreeNode;
	t->child[0] = p1;
	return t;
}

Parser::TreeNode* Parser::parseIfStmt() {
	string label1 = tac.NewLabel(), label2 = tac.NewLabel(), midvar;
	Parser::TreeNode* t = new Parser::TreeNode;
	t->nodeType = Parser::NodeType::N_IF_STMT;
	if (!match("if")) { return t; }
	if (!match("(")) { return t; }
	t->child[0] = new Parser::TreeNode;
	t->child[0] = parseCondition();
	midvar = midresult;
	
	//generate tac
	tac.NewQuad(TAC::QuadOpKind::IFZ, midvar, "", label1);
	
	if (!match(")")) { return t; }
	t->child.push_back(parseStatement());
	
	//generate tac
	tac.NewQuad(TAC::QuadOpKind::GOTO, "", "", label2);
	tac.NewQuad(TAC::QuadOpKind::LAB, "", "", label1);

	token = scanner.nextToken();
	if (token.lexeme == "else") {
		t->child.push_back(parseStatement());
	}
	else {
		scanner.ungetToken(token);
	}
	tac.NewQuad(TAC::QuadOpKind::LAB, "", "", label2);	//generate tac
	return t;
}

Parser::TreeNode* Parser::parseLoopStmt() {
	string label1 = tac.NewLabel(), label2 = tac.NewLabel(); //for tac generation
	string midname1, midname2, midname3, midvar;
	Parser::TreeNode* t = new Parser::TreeNode;
	token = scanner.nextToken();
	if (token.lexeme == "while") {
		t->nodeType = Parser::NodeType::N_WHILE_STMT;
		if (!match("(")) { return t; }
		t->child[0] = new Parser::TreeNode;

		//generate tac
		tac.NewQuad(TAC::QuadOpKind::LAB, "", "", label1);

		t->child[0] = parseCondition();
		if (!match(")")) { return t; }

		//generate tac
		tac.NewQuad(TAC::QuadOpKind::IFZ, midresult, "", label2);

		t->child.push_back(parseStatement());
		//generate tac
		tac.NewQuad(TAC::QuadOpKind::GOTO, "", "", label1);
		tac.NewQuad(TAC::QuadOpKind::LAB, "", "", label2);
	}
	if (token.lexeme == "for") {
		t->nodeType = Parser::NodeType::N_FOR_STMT;
		if (!match("(")) { return t; }
		token = scanner.nextToken();
		if (token.type != Scanner::TokenType::ID) {
			syntaxError("identifier", token);
			return t;
		}

		//check symbol table
		SymbolTable::SymbolKind ckind = symTable.kindOf(token.lexeme);
		if (ckind == SymbolTable::SymbolKind::NONE) {
			error(ID_NO_DEF_ERROR, token.line);
			exit(-1);
			return t;
		}

		Parser::TreeNode* p1 = new Parser::TreeNode;
		p1->nodeType = Parser::NodeType::N_FOR_INIT;
		p1->child[0] = new Parser::TreeNode;
		p1->child[0]->token = token;
		p1->child[0]->nodeType = Parser::NodeType::N_VAR;
		midname1 = token.lexeme;
		if (!match("=")) { return t; }
		p1->token = token;
		p1->child.push_back(parseExpression());
		tac.NewQuad(TAC::QuadOpKind::ASN, midresult, "", midname1);			//generate tac
		t->child[0] = new Parser::TreeNode;
		t->child[0] = p1;
		if (!match(";")) { return t; }
		tac.NewQuad(TAC::QuadOpKind::LAB, "", "", label1);					//generate tac
		t->child.push_back(parseCondition());
		tac.NewQuad(TAC::QuadOpKind::IFZ, midresult, "", label2);			//generate tac
		if (!match(";")) { return t; }
		token = scanner.nextToken();
		if (token.type != Scanner::TokenType::ID) {
			syntaxError("identifier", token);
			return t;
		}
		
		//check symbol table
		ckind = symTable.kindOf(token.lexeme);
		if (ckind == SymbolTable::SymbolKind::NONE) {
			error(ID_NO_DEF_ERROR, token.line);
			exit(-1);
			return t;
		}

		Parser::TreeNode* p2 = new Parser::TreeNode;
		p2->nodeType = Parser::NodeType::N_FOR_INCREMENT;
		p2->child[0] = new Parser::TreeNode;
		p2->child[0]->token = token;
		midname2 = token.lexeme;
		p2->child[0]->nodeType = Parser::NodeType::N_VAR;
		if (!match("=")) { return t; }
		p2->token = token;
		token = scanner.nextToken();
		if (token.type != Scanner::TokenType::ID) {
			syntaxError("identifier", token);
			return t;
		}
		midname3 = token.lexeme;

		Scanner::Token tmp = token;
		Parser::TreeNode* p3 = new Parser::TreeNode;
		p3->nodeType = Parser::NodeType::N_ARITHOP;
		token = scanner.nextToken();
		if (token.lexeme != "+" && token.lexeme != "-") { syntaxError("+ or -", token); return t; }
		p3->token = token;
		p3->child[0] = new Parser::TreeNode;
		p3->child[0]->nodeType = Parser::NodeType::N_VAR;

		//check symbol table
		ckind = symTable.kindOf(tmp.lexeme);
		if (ckind == SymbolTable::SymbolKind::NONE) {
			error(ID_NO_DEF_ERROR, token.line);
			exit(-1);
			return t;
		}

		p3->child[0]->token = tmp;
		token = scanner.nextToken();
		if (token.type != Scanner::TokenType::INT || token.lexeme == "0") { syntaxError("nonzero integer", token); return t; }
		p3->child.push_back(new Parser::TreeNode);
		p3->child[1]->nodeType = Parser::NodeType::N_INT;
		p3->child[1]->token = token;
		p2->child.push_back(p3);
		t->child.push_back(p2);
		if (!match(")")) { syntaxError(")", token); return t; }
		t->child.push_back(parseStatement());

		//generate tac
		midvar = tac.NewVar();
		tac.NewQuad((p3->token.lexeme == "+") ? (TAC::QuadOpKind::ADD) : (TAC::QuadOpKind::SUB), midname3, p3->child[1]->token.lexeme, midvar);
		tac.NewQuad(TAC::QuadOpKind::ASN, midvar, "", midname2);
		tac.NewQuad(TAC::QuadOpKind::GOTO, "", "", label1);
		tac.NewQuad(TAC::QuadOpKind::LAB, "", "", label2);
	}
	return t;
}

Parser::TreeNode* Parser::parseValueParameters() {
	vector<string> midparm;
	Parser::TreeNode* t = new Parser::TreeNode;
	t->nodeType = Parser::NodeType::N_VPARAMS;
	t->token.line = token.line;
	token = scanner.nextToken();
	if (token.lexeme == ")") {
		scanner.ungetToken(token);
		return t;
	}
	scanner.ungetToken(token);
	if (t->child[0] == nullptr) {
		t->child[0] = parseExpression();
	}
	else {
		t->child.push_back(parseExpression());
	}
	midparm.push_back(midresult);
	token = scanner.nextToken();
	while (token.lexeme == ",") {
		t->child.push_back(parseExpression());
		midparm.push_back(midresult);
		token = scanner.nextToken();
	}

	//generate tac
	int len = midparm.size();
	for (int i = 0; i < len; i++) {
		tac.NewQuad(TAC::QuadOpKind::PUSH, "", "", midparm[i]);
	}
	midparm.~vector();

	scanner.ungetToken(token);
	return t;
}

Parser::TreeNode* Parser::parseCallStmt() {
	string midvar, midname, ctype;
	Parser::TreeNode* t = new Parser::TreeNode;
	t->nodeType = Parser::NodeType::N_CALL_STMT;
	token = scanner.nextToken();
	if (token.type != Scanner::TokenType::ID) {
		syntaxError("identifier", token);
		return t;
	}
	t->child[0] = new Parser::TreeNode;
	t->child[0]->token = token;
	t->child[0]->nodeType = Parser::NodeType::N_FUNC_NAME;
	midname = token.lexeme;

	//check symbol table
	SymbolTable::SymbolKind  ckind = symTable.kindOf(token.lexeme);
	if (ckind != SymbolTable::SymbolKind::FUNCTION)
		error(FUNC_NO_DEF_ERROR, token.line);
	ctype = symTable.typeOf(token.lexeme);

	if (!match("(")) { return t; }
	t->child.push_back(parseValueParameters());
	if (!match(")")) { return t; }

	//generate tac
	if (ctype == "void") {
		tac.NewQuad(TAC::QuadOpKind::CALL, midname, "", "");
	} else {
		midvar = tac.NewVar();
		tac.NewQuad(TAC::QuadOpKind::CALL, midname, "", midvar);
		midresult = midvar;
	}

	return t;
}

Parser::TreeNode* Parser::parseLeftValue() {
	Parser::TreeNode* t = new Parser::TreeNode;
	t->nodeType = Parser::NodeType::N_VAR;
	token = scanner.nextToken();
	t->token = token;

	//check symbol table
	SymbolTable::SymbolKind ckind = symTable.kindOf(token.lexeme);
	if (ckind == SymbolTable::SymbolKind::NONE)
		error(VAR_NO_DEF_ERROR, token.line);

	token = scanner.nextToken();
	if (token.lexeme == "[") {
		t->nodeType = Parser::NodeType::N_ARRAY;
		t->child[0] = parseExpression();
		token = scanner.nextToken();
		if (token.lexeme != "]") {
			syntaxError("]", token);
			return t;
		}
	}
	else if (token.lexeme == "=") {
		scanner.ungetToken(token);
	}
	return t;
}

Parser::TreeNode* Parser::parseAssignStmt() {
	string index;
	Parser::TreeNode* t = new Parser::TreeNode;
	t->nodeType = Parser::NodeType::N_ASSIGN;
	t->child[0] = parseLeftValue();
	index = midresult;
	token = scanner.nextToken();
	t->token = token;
	t->child.push_back(parseExpression());
	//generate tac
	if (t->child[0]->nodeType == Parser::NodeType::N_ARRAY) {
		tac.NewQuad(TAC::QuadOpKind::STORE, midresult, index, t->child[0]->token.lexeme);
	} else {
		tac.NewQuad(TAC::QuadOpKind::ASN, midresult, "", t->child[0]->token.lexeme);
	}
	return t;
}

Parser::TreeNode* Parser::parseExpression() {
	string midvar1, midvar2, midvar3;	//for tac generation
	Parser::TreeNode* t = parseTerm();
	midvar3 = midresult;
	token = scanner.nextToken();
	while (token.lexeme == "+" || token.lexeme == "-") {
		Parser::TreeNode* p = new Parser::TreeNode;
		p->nodeType = Parser::NodeType::N_ARITHOP;
		p->token = token;
		if (p->child[0] == nullptr) p->child[0] = t;
		else p->child.push_back(t);
		t = p;
		midvar1 = midvar3;
		p->child.push_back(parseTerm());

		//generate tac
		midvar2 = midresult;
		midvar3 = tac.NewVar();
		tac.NewQuad( (p->token.lexeme == "+")?(TAC::QuadOpKind::ADD):(TAC::QuadOpKind::SUB), midvar1, midvar2, midvar3 );

		token = scanner.nextToken();
	}
	scanner.ungetToken(token);
	midresult = midvar3;
	return t;
}

Parser::TreeNode* Parser::parseTerm() {
	string midvar1, midvar2, midvar3;
	Parser::TreeNode* t = parseFactor();
	midvar3 = midresult;
	token = scanner.nextToken();
	while (token.lexeme == "*" || token.lexeme == "/") {
		Parser::TreeNode* p = new Parser::TreeNode;
		p->nodeType = Parser::NodeType::N_ARITHOP;
		p->token = token;
		if (p->child[0] == nullptr) p->child[0] = t;
		else p->child.push_back(t);
		t = p;
		midvar1 = midvar3;
		p->child.push_back(parseFactor());
		
		//generate tac
		midvar2 = midresult;
		midvar3 = tac.NewVar();
		tac.NewQuad( (p->token.lexeme == "*")?(TAC::QuadOpKind::MUL):(TAC::QuadOpKind::DIV), midvar1, midvar2, midvar3 );

		token = scanner.nextToken();
	}
	scanner.ungetToken(token);
	midresult = midvar3;
	return t;
}

Parser::TreeNode* Parser::parseFactor() {
	string midvar;
	Parser::TreeNode* t = nullptr;
	token = scanner.nextToken();
	if (token.type == Scanner::TokenType::ERROR) {
		syntaxError("valid token", token);
		return t;
	}
	if (token.lexeme == "(") {
		t = parseExpression();
		token = scanner.nextToken();
		if (token.lexeme != ")") {
			syntaxError(")", token);
			return t;
		}
	}
	else if (token.lexeme == "\'") {
		token = scanner.nextToken();
		if (token.lexeme.length() != 1) {
			syntaxError("character", token);
			return t;
		}
		token = scanner.nextToken();
		if (token.lexeme != "\'") {
			syntaxError("\'", token);
			return t;
		}
	}
	else if (token.type == Scanner::TokenType::INT) {
		t = new Parser::TreeNode;
		t->token = token;
		t->nodeType = Parser::NodeType::N_INT;
		midresult = t->token.lexeme;
	}
	else if (token.type == Scanner::TokenType::CHAR) {
		t = new Parser::TreeNode;
		t->token = token;
		t->nodeType = Parser::NodeType::N_CHAR;
		midresult = t->token.lexeme;
	}
	else if (token.type == Scanner::TokenType::ID) {
		t = new Parser::TreeNode;
		Scanner::Token tmp = token;
		t->token = token;
		t->nodeType = Parser::NodeType::N_VAR;

		// check symbol table
		SymbolTable::SymbolKind ckind = symTable.kindOf(token.lexeme);
		if (ckind == SymbolTable::SymbolKind::NONE)
			error(ID_NO_DEF_ERROR, token.line);

		midresult = t->token.lexeme;
		token = scanner.nextToken();
		if (token.lexeme == "[") {
			t->nodeType = Parser::NodeType::N_ARRAY;

			//check symbol table
			int csize = symTable.sizeOf(tmp.lexeme);
			if (csize <= 0)
				error(ARRAY_NO_DEF_ERROR, token.line);

			t->child[0] = parseExpression();
			string index = midresult;

			//check symbol table
			//to see if index of array out of boundary
			if (t->child[0]->nodeType == Parser::NodeType::N_INT) {
				int csize = symTable.sizeOf(t->token.lexeme);
				int cindex = atoi(t->child[0]->token.lexeme.c_str());
				if (cindex < 0 || cindex >= csize) {
					error(ARRAY_INDEX_ERROR, t->token.line);
				}
			}

			token = scanner.nextToken();
			if (token.lexeme != "]") {
				syntaxError("]", token);
				return t;
			}

			//generate tac
			midresult = tac.NewVar();
			tac.NewQuad(TAC::QuadOpKind::LOAD, t->token.lexeme, index, midresult);

		}
		else if (token.lexeme == "(") {
			scanner.ungetToken(token);
			scanner.ungetToken(tmp);

			//check symbol table
			ckind = symTable.kindOf(tmp.lexeme);
			if (ckind != SymbolTable::SymbolKind::FUNCTION)
				error(FUNC_NO_DEF_ERROR, tmp.line);
			string ctype = symTable.typeOf(tmp.lexeme);
			if (ctype == "void")
				error(CALL_NO_RETURN_FUNC_ERROR, tmp.line);

			Parser::TreeNode* p = parseCallStmt();
			t = p;
		}
		else {
			scanner.ungetToken(token);
		}
	}
	else {
		if (token.type != Scanner::TokenType::ENDOFFILE) {
			syntaxError("valid factor", token);
			return t;
		}
	}
	return t;
}

string Parser::traversingTree2str(Parser::TreeNode* a) {
	string t = "";
	if (a == NULL) return "";
	else {
		t += a->token.lexeme;
		for (vector<Parser::TreeNode*>::iterator it = a->child.begin(); it != a->child.end(); it++) {
			t += traversingTree2str(*it);
		}
		printf("%s\n", t.c_str());
		return t;
	}
}

void Parser::printSyntaxTree(Parser::TreeNode* tree) {
	static int n = 0;
	n += 2;
	if (tree != nullptr) {
		for (int i = 0; i < n; i++)
			cout << " ";
		switch (tree->nodeType) {
		case Parser::NodeType::N_ARITHOP:
			cout << "operation " << tree->token.lexeme << endl;
			break;
		case Parser::NodeType::N_RELOP:
			cout << "operation " << tree->token.lexeme << endl;
			break;
		case Parser::NodeType::N_CHAR:
			cout << "char_value " << tree->token.lexeme << endl;
			break;
		case Parser::NodeType::N_INT:
			cout << "int_value " << tree->token.lexeme << endl;
			break;
		case Parser::NodeType::N_STRING:
			cout << "string_value " << tree->token.lexeme << endl;
			break;
		case Parser::NodeType::N_KEYWORD:
			cout << "keyword " << tree->token.lexeme << endl;
			break;
		case Parser::NodeType::N_CONST_LIST:
			cout << "const_list " << endl;
			break;
		case Parser::NodeType::N_CONST_DEC:
			cout << "const_declare " << endl;
			break;
		case Parser::NodeType::N_CONST_INT:
			cout << "int_const " << tree->token.lexeme << endl;
			break;
		case Parser::NodeType::N_CONST_CHAR:
			cout << "char_const " << tree->token.lexeme << endl;
			break;
		case Parser::NodeType::N_VAR_LIST:
			cout << "var_list " << endl;
			break;
		case Parser::NodeType::N_VAR_DEC:
			cout << "var_declare " << endl;
			break;
		case Parser::NodeType::N_VAR:
			cout << "variable " << tree->token.lexeme << endl;
			break;
		case Parser::NodeType::N_BASIC_TYPE:
			cout << "basic_type " << tree->token.lexeme << endl;
			break;
		case Parser::NodeType::N_ARRAY:
			cout << "array " << tree->token.lexeme << endl;
			break;
		case Parser::NodeType::N_MAIN:
			cout << "main " << endl;
			break;
		case Parser::NodeType::N_FUNC_DEF:
			cout << "function_def " << endl;
			break;
		case Parser::NodeType::N_RETURN_TYPE:
			cout << "return_type " << tree->token.lexeme << endl;
			break;
		case Parser::NodeType::N_FUNC_NAME:
			cout << "function_name " << tree->token.lexeme << endl;
			break;
		case Parser::NodeType::N_PARAM_LIST:
			cout << "para_list " << endl;
			break;
		case Parser::NodeType::N_PARAM:
			cout << "para " << endl;
			break;
		case Parser::NodeType::N_FORMAL_PARAM:
			cout << "formal_para " << tree->token.lexeme << endl;
			break;
		case Parser::NodeType::N_COMP_STMT:
			cout << "compound_stmt " << endl;
			break;
		case Parser::NodeType::N_STMTS:
			cout << "statements " << endl;
			break;
		case Parser::NodeType::N_IF_STMT:
			cout << "if_stmt " << endl;
			break;
		case Parser::NodeType::N_CONDITION:
			cout << "condition " << endl;
			break;
		case Parser::NodeType::N_WHILE_STMT:
			cout << "while_stmt " << endl;
			break;
		case Parser::NodeType::N_FOR_STMT:
			cout << "for_stmt " << endl;
			break;
		case Parser::NodeType::N_FOR_INIT:
			cout << "for_init " << endl;
			break;
		case Parser::NodeType::N_FOR_INCREMENT:
			cout << "for_increment " << endl;
			break;
		case Parser::NodeType::N_EXPRESSION:
			cout << "expression " << tree->token.lexeme << endl;
			break;
		case Parser::NodeType::N_ASSIGN:
			cout << "assign " << endl;
			break;
		case Parser::NodeType::N_CALL_STMT:
			cout << "call_stmt " << endl;
			break;
		case Parser::NodeType::N_VPARAMS:
			cout << "para_values " << endl;
			break;
		case Parser::NodeType::N_READ_STMT:
			cout << "read_stmt " << endl;
			break;
		case Parser::NodeType::N_WRITE_STMT:
			cout << "write_stmt " << endl;
			break;
		case Parser::NodeType::N_RETURN_STMT:
			cout << "return_stmt " << endl;
			break;
		case Parser::NodeType::N_PROGRAM:
			cout << "program " << endl;
			break;
		default:
			;
		}
		for (vector<Parser::TreeNode*>::iterator it = tree->child.begin(); it != tree->child.end(); it++) {
			printSyntaxTree(*it);
		}
	}
	n -= 2;
}
