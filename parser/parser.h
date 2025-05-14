#ifndef PARSER_H
#define PARSER_H

#include "Scanner.h"
#include "ast.h"
#include "symbol.h"
#include <fstream>

struct had_error {
    bool error; // Flag to indicate if an error occurred
    int line; // Line number where the error occurred
    int char_num; // Character number where the error occurred
    char* message; // Error message
    had_error* next;
    had_error() : error(false), line(0), char_num(0), message(nullptr), next(nullptr) {} // Constructor to initialize members
};

class Parser {
private:
    std::ofstream errorFile; // File stream for error reporting

public:    // Members
    bool had_error; // Flag to indicate if parsing encountered errors
    AST* programAST;   // Pointer to the root of the AST
    SymbolTable* table; // Symbol table for storing identifiers
    TOKEN* currentToken;  // Lookahead token for parsing
    Scanner* scanner;  // Scanner instance for tokenization
    FileDescriptor* fd; // File descriptor
    
    // Token handling methods
    TOKEN* peek_token();  // Look at the next token without consuming it
    TOKEN* consume_token(); // Consume the current token
    TOKEN* match(LEXEME_TYPE expected); // Match and consume expected token
    const char* getTokenTypeName(LEXEME_TYPE type); // Get human-readable token type
    
    // Parsing methods
    void scan_and_check_illegal_token();
    AST* start_parsing();
    ast_list* parseProgram();
    ast_list* parseDeclList();
    AST* parseDecl();
    j_type parseType();
    ste_list* parseFormalList();
    AST* parseFormals();
    AST* parseFormalsTail();
    AST* parseStmt();
    AST* parseStmtIdTail(STEntry* entry);
    AST* parseIfTail(AST* condNode, AST* thenStmtNode);
    AST* parseBlock(); // Parse a code block
    ste_list* parseVarDeclList();
    STEntry* parseVarDecl();
    ast_list* parseStmtList();
    ast_list* parseArgList();
    AST* parseArgs();
    AST* parseArgsTail();
    AST* parseRelOp();
    AST* parseRelConj();
    AST* parseUnaryOp();
    AST* parseAddSubOp();
    AST* parseMultDivOp();
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

    bool noVariableDecl();// function to check if there are no variable declarations and just statements
    bool noStatementsLeft() // function to check if there are no statements left
    {
        return (currentToken->type == kw_end || currentToken->type == lx_eof);
    }
    Parser(FileDescriptor* fd);
    ~Parser();

private:
    STEntry* checkAndAddSymbol(TOKEN* idToken, STE_TYPE steType);
};

#endif // PARSER_H

