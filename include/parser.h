#ifndef PARSER_H
#define PARSER_H

#include "Scanner.h"
#include "ast.h"
#include "symbol.h"
#include <fstream>

struct had_error {
    bool error;
    int line;
    int char_num;
    char* message;
    had_error* next;
    had_error() : error(false), line(0), char_num(0), message(nullptr), next(nullptr) {}
};

class Parser {
private:
    std::ofstream errorFile;

public:
    bool had_error;
    AST* programAST;
    SymbolTable* table;
    TOKEN* currentToken;
    Scanner* scanner;
    FileDescriptor* fd;    TOKEN* match(LEXEME_TYPE expected);
    const char* getTokenTypeName(LEXEME_TYPE type);
    
    void scan_and_check_illegal_token();
    AST* start_parsing();
    ast_list* parseProgram();
    ast_list* parseDeclList();
    AST* parseDecl();
    j_type parseType();
    ste_list* parseFormalList();
    AST* parseStmt();
    AST* parseStmtIdTail(STEntry* entry);
    AST* parseIfTail(AST* condNode, AST* thenStmtNode);
    AST* parseBlock();
    ste_list* parseVarDeclList();
    STEntry* parseVarDecl();
    ast_list* parseStmtList();
    AST* parsePrimaryExprTail(AST* idNode);
    AST* parseExpr();
    AST* parseExprTail(AST* leftNode);
    AST* parseExpr2();
    AST* parseExpr2Tail(AST* leftNode);
    AST* parseExpr3();
    AST* parseExpr3Tail(AST* leftNode);
    AST* parseExpr4();
    AST* parseExpr4Tail(AST* leftNode);
    AST* parseExpr5();
    AST* parsePrimaryExpr();

    void printParsedAST(AST* node);
    void checkForRedeclaration(TOKEN* idToken);

    bool noVariableDecl();
    bool noStatementsLeft()
    {
        return (currentToken->type == kw_end || currentToken->type == lx_eof);
    }
    Parser(FileDescriptor* fd);
    ~Parser();

private:
    STEntry* checkAndAddSymbol(TOKEN* idToken, STE_TYPE steType);
};

#endif // PARSER_H

