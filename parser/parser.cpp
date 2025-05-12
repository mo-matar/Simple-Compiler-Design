#include "parser.h"
#include <stdarg.h>

Parser::Parser(FileDescriptor* fd) {
    scanner = new Scanner(fd);
    table = new SymbolTable();
    currentToken = new TOKEN();
    programAST = nullptr;
    had_error = false;
}

const char* Parser::getTokenTypeName(LEXEME_TYPE type) {
    switch (type) {
        case lx_identifier: return "Identifier";
        case kw_var: return "Keyword var";
        case kw_integer: return "Keyword integer";
        case kw_string: return "Keyword string";
        case kw_bool: return "Keyword bool";
        case kw_if: return "Keyword if";
        case kw_else: return "Keyword else";
        case kw_read: return "Keyword read";
        case kw_write: return "Keyword write";
        case lx_semicolon: return "Semicolon";
        case lx_colon: return "Colon";
        case lx_eof: return "End of File";
        default: return "Unknown Token Type";
    }
}

TOKEN* Parser::match(LEXEME_TYPE expected) {
    if (currentToken->type == expected) {
        TOKEN* matchedToken = currentToken;
        std::cout << "Matched token: " << getTokenTypeName(currentToken->type) << std::endl;
        currentToken = scanner->Scan(); // Advance to the next token
        std::cout << "Next token: " << getTokenTypeName(currentToken->type) << std::endl;
        return matchedToken;
    } else {
        had_error = true;
        std::cerr << "Syntax Error: Expected token of type " 
                  << getTokenTypeName(expected) 
                  << " but found " 
                  << getTokenTypeName(currentToken->type) 
                  << "." << std::endl;
        return nullptr;
    }
}

void Parser::checkForRedeclaration(TOKEN* idToken) {


}



void Parser::scan_and_check_illegal_token() {
    currentToken = scanner->Scan();
    std::cout << "Scanned token type: " << currentToken->type << " (" << getTokenTypeName(currentToken->type) << ")" << std::endl;
    if (currentToken->type == lx_identifier && currentToken->str_ptr) {
        std::cout << "Identifier value: " << currentToken->str_ptr << std::endl;
    }
    
    if (currentToken->type == illegal_token) {
        had_error = true;
        std::cout << "Error: Illegal token encountered from parser." << std::endl;
    }
    else std::cout << "no illegal token yet... parser" << std::endl;
}

void Parser::printParsedAST(AST* node) {
    if (node == nullptr) {
        std::cout << "Error: Null AST node encountered." << std::endl;
        return;
    }
    FILE* outputFile = fopen("output_program.txt", "w");

    print_ast_node(outputFile, node);
}

AST* Parser::start_parsing() {
    std::cout << "Starting parsing..." << std::endl;
    
    // programAST = new AST();
    
    ast_list* programStatements = parseProgram();
    std::cout << "Program parsed, creating AST node" << std::endl;
    AST* programAST = make_ast_node(ast_program, programStatements);
    std::cout << "AST node created" << std::endl;

    return programAST;
}

ast_list* Parser::parseProgram() {
    currentToken = scanner->Scan();
    ast_list* declList = parseDeclList();
    
    return declList;
}


ast_list* Parser::parseDeclList() {
    ast_list* declList = nullptr;
    std::cout << "In parseDeclList()" << std::endl;
    //scan_and_check_illegal_token();
    std::cout << "Current token type: " << getTokenTypeName(currentToken->type) << std::endl;
    //this is the lambda part of the grammar
    if(currentToken->type == lx_eof) {
        std::cout << "End of file reached, returning empty list" << std::endl;
        return declList;
    }
    else{//not lambda, then it is a declaration
        std::cout << "Parsing declaration..." << std::endl;
        AST* decl = parseDecl();
        std::cout << "Declaration parsed, looking for semicolon" << std::endl;

        match(lx_semicolon);
        std::cout << "Semicolon matched, recursively parsing next declaration" << std::endl;

        declList = cons_ast(decl, parseDeclList());
        std::cout << "Returning from parseDeclList()" << std::endl;

        return declList;
    }
}


AST* Parser::parseDecl() {
    std::cout << "In parseDecl(), token type: " << getTokenTypeName(currentToken->type) << std::endl;
    if (currentToken->type == lx_identifier && currentToken->str_ptr) {
        std::cout << "   Current identifier value: " << currentToken->str_ptr << std::endl;
    }
    
    AST* declNode = nullptr;
    TOKEN* idToken = nullptr;
    j_type typeNode = type_none;
    STEntry* STE = nullptr;
    
    switch (currentToken->type) {
        case kw_var:
            std::cout << "Found var keyword" << std::endl;
            match(kw_var);
            idToken = match(lx_identifier);
            std::cout << "Found identifier: " << idToken->str_ptr << std::endl;
            // Check for redeclaration
            STE = table->GetSymbolFromScopes(idToken->str_ptr);
            if (STE != nullptr){
                had_error = true;

                char error_msg[] = "Redeclaration of identifier";
                scanner->fd->ReportError(error_msg);
            }
            else {
                std::cout << "No redeclaration found for identifier: " << idToken->str_ptr << std::endl;
                STE = table->PutSymbol(idToken->str_ptr, STE_INT, scanner->getLineNum());
            }
            match(lx_colon);
            std::cout << "Found colon, parsing type" << std::endl;
            typeNode = parseType();

            
            declNode = make_ast_node(ast_var_decl, STE, typeNode);
            
            break;
        default:
            had_error = true;
            std::cerr << "Syntax Error: Expected a declaration but found " 
                      << getTokenTypeName(currentToken->type) 
                      << "." << std::endl;
            break;
        }
        return declNode;
}


j_type Parser::parseType() {
    switch (currentToken->type) {
        case kw_integer:
            match(kw_integer);
            return type_integer;
            // typeNode = make_ast_node(ast_integer);
            //print_ast_node(stdout, typeNode);
            //std::cout << "Printed integer type node" << std::endl;
            break;
        case kw_string:
            match(kw_string);
            return type_string;
            // typeNode = make_ast_node(ast_string);
            //print_ast_node(stdout, typeNode);
            //std::cout << "Printed string type node" << std::endl;
            break;
        case kw_bool:
            match(kw_bool);
            return type_boolean;   
            // typeNode = make_ast_node(ast_boolean);
           // print_ast_node(stdout, typeNode);
            //std::cout << "Printed boolean type node" << std::endl;
            break;
        default:
            had_error = true;
            std::cerr << "Syntax Error: Expected a type but found " 
                      << getTokenTypeName(currentToken->type) 
                      << "." << std::endl;
    }
    return type_none;
}

    
    

// Global scanner and file descriptor pointers
