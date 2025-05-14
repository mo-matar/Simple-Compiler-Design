#include <iostream>
#include "Parser.h"
using namespace std;

int main()
{
        FileDescriptor *fd = new FileDescriptor("test5_all_operators.txt");
        Parser *parser = new Parser(fd);
        AST* root = parser->start_parsing();
        if (parser->had_error) {
            cout << "Parsing failed with errors." << endl;
        } else {
            cout << "Parsing succeeded." << endl;
            parser->printParsedAST(root);
        }
                    parser->printParsedAST(root);

        return 0;
}