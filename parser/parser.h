#ifndef PARSER_H
#define PARSER_H

#include "Scanner.h"
#include "ast.h"
#include "symbol.h"

class Parser {
private:
    // Members
    TOKEN* lookahead;  // Lookahead token for parsing
    Scanner* scanner;  // Scanner instance for tokenization
    FileDescriptor* fd; // File descriptor
    bool had_error;    // Flag to indicate if parsing encountered errors

    // Token handling methods
    TOKEN* peek_token();  // Look at the next token without consuming it
    TOKEN* consume_token(); // Consume the current token
    TOKEN* match(LEXEME_TYPE expected); // Match and consume expected token
    const char* getTokenTypeName(LEXEME_TYPE type); // Get human-readable token type

    // Parsing methods
    AST* parseProgram();
    AST* parseDeclList();
    AST* parseDecl();
    AST* parseType();
    AST* parseFormalList();
    AST* parseFormals();
    AST* parseFormalsTail();
    AST* parseStmt();
    AST* parseStmtIdTail(AST* idNode);
    AST* parseIfTail(AST* condNode, AST* thenStmtNode);
    AST* parseBlock();
    AST* parseVarDeclList();
    AST* parseVarDecl();
    AST* parseStmtList();
    AST* parseArgList();
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

public:
    Parser();
    ~Parser();
    AST* parse(FileDescriptor* fd, Scanner* scanner);
};

#endif // PARSER_H

