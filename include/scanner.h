#ifndef SCANNER_H_
#define SCANNER_H_

#include <set>
#include <string>
#include <fstream>

using namespace std;

class Scanner {
private:
	enum State {
		START_STATE,
		ID_STATE,
		INT_STATE,
		CHAR_STATE, CHAR_STATE_A, CHAR_STATE_B, CHAR_STATE_C,
		STRING_STATE, STRING_STATE_A,
		COMMENT_STATE, COMMENT_STATE_A,
		SYMBOL_STATE,
		DONE_STATE,
		ERROR_STATE
	};

public:
	set<string> keywords;
	set<string> symbols;
	enum TokenType {
		KEYWORD, ID,
		INT, CHAR, STRING, SYMBOL,
		ERROR, ENDOFFILE
	};
	struct Token {
		TokenType type;
		string lexeme;
		unsigned line;
	};
	void initKeywords();
	void initSymbols();

private:
	string lineBuffer;	// ������
	unsigned bufferPos;	// �����е�ָ��
	unsigned line;		// ��ǰ��������Դ�����е��к�
	ifstream fin;
	char nextChar();	// ���ػ���������һ���ַ�
	TokenType searchReserved(string const& s);
	void printToken(Scanner::Token token);
public:
	Scanner();
	void openFile(string const& filename);
	void closeFile();
	void rollBack();	// �ع�������
	void ungetToken(Token token);
	Token nextToken();
	void resetRow();
};

#endif // SCANNER_H_INCLUDED
