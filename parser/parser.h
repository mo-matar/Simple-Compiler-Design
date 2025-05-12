#ifndef PARSER_H
#define PARSER_H

#include "Scanner.h"
#include "ast.h"
#include "symbol.h"

class Parser {
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
    ast_list* parseFormalList();
    AST* parseFormals();
    AST* parseFormalsTail();
    AST* parseStmt();
    AST* parseStmtIdTail(AST* idNode);
    AST* parseIfTail(AST* condNode, AST* thenStmtNode);
    AST* parseBlock();
    ast_list* parseVarDeclList();
    AST* parseVarDecl();
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

    Parser(FileDescriptor* fd);
    ~Parser();

private:
    STEntry* checkAndAddSymbol(TOKEN* idToken, STE_TYPE steType);
};

#endif // PARSER_H

