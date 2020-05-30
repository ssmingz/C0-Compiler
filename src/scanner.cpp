#include "scanner.h"
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <string>
#include <cassert>
#include <iostream>

using namespace std;

Scanner::Scanner() {
	line = 0;
	bufferPos = 0;
	initKeywords();
	initSymbols();
}

void Scanner::resetRow() {
	line = 0;
}

void Scanner::openFile(const string& filename) {
	fin.open(filename);
	if (fin.fail()) {
		cerr << "file " << filename << " does not exit" << endl;
		exit(-1);
	}
}

void Scanner::closeFile() {
	fin.close();
}

void Scanner::initKeywords() {
	keywords.insert("const");
	keywords.insert("int");
	keywords.insert("char");
	keywords.insert("string");
	keywords.insert("void");
	keywords.insert("main");
	keywords.insert("if");
	keywords.insert("else");
	keywords.insert("while");
	keywords.insert("for");
	keywords.insert("scanf");
	keywords.insert("printf");
	keywords.insert("return");
}

void Scanner::initSymbols() {
	symbols.insert("{");
	symbols.insert("}");
	symbols.insert("[");
	symbols.insert("]");
	symbols.insert("(");
	symbols.insert(")");
	symbols.insert(",");
	symbols.insert(";");
	symbols.insert("+");
	symbols.insert("-");
	symbols.insert("*");
	symbols.insert("/");
	symbols.insert("=");
	symbols.insert(">");
	symbols.insert("<");
	symbols.insert(">=");
	symbols.insert("<=");
	symbols.insert("==");
	symbols.insert("!=");
}

Scanner::TokenType Scanner::searchReserved(const string& s) {
	if (keywords.find(s) != keywords.end())
		return KEYWORD;
	else
		return ID;
}

void Scanner::printToken(Scanner::Token token) {
	switch (token.type) {
	case KEYWORD:
		printf("RESERVED WORD: %s\n", token.lexeme.c_str());
		break;
	case SYMBOL:
		printf("SYMBOL: %s\n", token.lexeme.c_str());
		break;
	case ENDOFFILE:
		printf("EOF\n");
		break;
	case INT:
		printf("INT, value= %s\n", token.lexeme.c_str());
		break;
	case CHAR:
		printf("CHAR, value=%s\n", token.lexeme.c_str());
	case ID:
		printf("ID, name= %s\n", token.lexeme.c_str());
		break;
	case STRING:
		printf("STRING, content=%s\n", token.lexeme.c_str());
		break;
	case ERROR:
		printf("ERROR: %s\n", token.lexeme.c_str());
		break;
	}
}


char Scanner::nextChar() {
	if (bufferPos >= lineBuffer.size()) {
		line++;
		getline(fin, lineBuffer);
		lineBuffer += '\n';
		if (!fin.fail()) {
			bufferPos = 0;
			return lineBuffer[bufferPos++];
		}
		else return EOF;
	}
	else return lineBuffer[bufferPos++];
}

void Scanner::rollBack() {
	assert(bufferPos > 0);
	bufferPos--;
}

void Scanner::ungetToken(Token token) {
	assert(bufferPos > 0);
	for (unsigned i = 0; i < token.lexeme.length(); i++) {
		do {
			bufferPos--;
		} while (isspace(lineBuffer[bufferPos]));
	}
	if (token.type == Scanner::TokenType::CHAR || token.type == Scanner::TokenType::STRING) {
		bufferPos -= 2;
	}
}

Scanner::Token Scanner::nextToken() {
	Token token;
	State state = START_STATE;
	while (state != DONE_STATE) {
		char ch = nextChar();
		if (ch == EOF) {
			token.type = ENDOFFILE;
			break;
		}
		switch (state) {
		case START_STATE:
			if (isspace(ch));
			else if (isalpha(ch) || ch == '_') {
				state = ID_STATE;
				token.type = ID;
				token.lexeme += ch;
				token.line = line;
			}
			else if (isdigit(ch)) {
				if (ch == '0') {
					token.lexeme += ch;
					token.line = line;
					ch = nextChar();
					if (!isdigit(ch)) {
						state = INT_STATE;
						token.type = INT;
						rollBack();
					}
					else {
						state = ERROR_STATE;
						token.type = ERROR;
						token.lexeme += ch;
						token.line = line;
					}
				}
				else {
					state = INT_STATE;
					token.type = INT;
					token.lexeme += ch;
					token.line = line;
				}
			}
			else if (symbols.find({ ch }) != symbols.end()) {
				state = SYMBOL_STATE;
				token.type = SYMBOL;
				token.lexeme += ch;
				token.line = line;
			}
			else if (ch == '!') {
				state = SYMBOL_STATE;
				token.lexeme += ch;
				token.line = line;
			}
			else if (ch == '"') {
				state = STRING_STATE;
				token.type = STRING;
				token.line = line;
			}
			else if (ch == '\'') {
				state = CHAR_STATE;
				token.type = CHAR;
				token.line = line;
			}
			else {
				state = ERROR_STATE;
				token.type = ERROR;
				token.lexeme += ch;
				token.line = line;
			}
			break;
		case INT_STATE:
			if (isdigit(ch))
				token.lexeme += ch;
			else {
				rollBack();
				state = DONE_STATE;
			}
			break;
		case ID_STATE:
			if (isalpha(ch) || isdigit(ch) || ch == '_')
				token.lexeme += ch;
			else {
				rollBack();
				state = DONE_STATE;
			}
			break;
		case CHAR_STATE:
			if (ch != '\\' && ch != '\'') {
				state = CHAR_STATE_A;
				token.lexeme += ch;
			}
			else if (ch == '\\') {
				state = CHAR_STATE_B;
				token.lexeme += ch;
			}
			else if (ch == '\'') {
				state = ERROR_STATE;
				token.type = ERROR;
				token.lexeme += ch;
			}
			break;
		case CHAR_STATE_A:
			if (ch == '\'') state = DONE_STATE;
			else {
				state = ERROR_STATE;
				token.type = ERROR;
				string tmp = "'";
				tmp.append(token.lexeme);
				tmp += ch;
				token.lexeme = tmp;
			}
			break;
		case CHAR_STATE_B:
			if (ch == 'a' || ch == 'b' || ch == 'f' || ch == 'n' || ch == 'r' ||
				ch == 't' || ch == 'v' || ch == '\\' || ch == '?' || ch == '\'' ||
				ch == '"') {
				state = CHAR_STATE_C;
				token.lexeme += ch;
			}
			else {
				state = ERROR_STATE;
				token.type = ERROR;
				string tmp = "'\\";
				tmp += ch;
				token.lexeme = tmp;
			}
			break;
		case CHAR_STATE_C:
			if (ch == '\'') state = DONE_STATE;
			else {
				state = ERROR_STATE;
				token.type = ERROR;
				string tmp = "'";
				tmp.append(token.lexeme);
				tmp += ch;
				token.lexeme = tmp;
			}
			break;
		case STRING_STATE:
			if (ch == '"')
				state = DONE_STATE;
			else if (ch == '\\') {
				state = STRING_STATE_A;
				token.lexeme += ch;
			}
			else token.lexeme += ch;
			break;
		case STRING_STATE_A:
			state = STRING_STATE;
			token.lexeme.pop_back();
			token.lexeme += ch;
			break;
		case SYMBOL_STATE:
			if (token.lexeme == "/") {
				if (ch == '*') {
					state = COMMENT_STATE;
					token.lexeme.pop_back();
				}
				else if (ch == '/') {
					state = START_STATE;
					bufferPos = lineBuffer.length();
					token.lexeme.pop_back();
				}
				else {
					rollBack();
					state = DONE_STATE;
				}
			}
			else if (token.lexeme == "<") {
				if (ch == '=') {
					token.lexeme += ch;
					state = DONE_STATE;
				}
				else {
					rollBack();
					state = DONE_STATE;
				}
			}
			else if (token.lexeme == "=") {
				if (ch == '=') {
					token.lexeme += ch;
					state = DONE_STATE;
				}
				else {
					rollBack();
					state = DONE_STATE;
				}
			}
			else if (token.lexeme == ">") {
				if (ch == '=') {
					token.lexeme += ch;
					state = DONE_STATE;
				}
				else {
					rollBack();
					state = DONE_STATE;
				}
			}
			else if (token.lexeme == "!") {
				if (ch == '=') {
					token.type = SYMBOL;
					token.lexeme += ch;
					state = DONE_STATE;
				}
				else {
					rollBack();
					state = ERROR_STATE;
					token.type = ERROR;
				}
			}
			else {
				rollBack();
				state = DONE_STATE;
			}
			break;
		case COMMENT_STATE:
			if (ch == '*') state = COMMENT_STATE_A;
			break;
		case COMMENT_STATE_A:
			if (ch == '/') state = START_STATE;
			else state = COMMENT_STATE;
			break;
		case ERROR_STATE:
			if (ch == ' ' || ch == '\n' || ch == '\t') state = DONE_STATE;
			else token.lexeme += ch;
			break;
		default:;
		}
		if (state == DONE_STATE && token.type == ID)
			token.type = searchReserved(token.lexeme);
	}
	//printf("%d: ",line);
	//Scanner::printToken(token);
	return token;
}
