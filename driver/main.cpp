#include "analyzer.h"
#include "parser.h"
#include "scanner.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

using namespace std;

FILE* compile_file;

int main()
{
    char data_path[100];
    scanf("%s", data_path);
    string str(data_path);
    if ((compile_file = fopen(data_path, "r")) == NULL) {
        printf("Cannot find %s\n", data_path);
        exit(1);
    }
    Parser parser(str);
    parser.syntaxTree = parser.parseProgram();
    if (!hasError()) {
        parser.printSyntaxTree(parser.syntaxTree);
        Analyzer analyzer(parser.syntaxTree, parser.symTable);
        analyzer.checkStatement(analyzer.tree);
        parser.symTable.printGlobalSymbolTable();
        parser.symTable.printLocalSymbolTable();
    }
    return 0;
}
