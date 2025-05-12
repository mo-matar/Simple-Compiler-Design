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
    STEntry* STE = table->GetSymbolFromScopes(idToken->str_ptr);
    if (STE != nullptr){
        had_error = true;
        char error_msg[] = "Redeclaration of identifier";
        scanner->fd->ReportError(error_msg);
        return;
    }
    
    std::cout << "No redeclaration found for identifier: " << idToken->str_ptr << std::endl;
}

STEntry* Parser::checkAndAddSymbol(TOKEN* idToken, STE_TYPE steType) {
    STEntry* STE = table->GetSymbolFromScopes(idToken->str_ptr);
    if (STE != nullptr){
        had_error = true;
        char error_msg[] = "Redeclaration of identifier";
        scanner->fd->ReportError(error_msg);
        return nullptr;
    }
    
    std::cout << "No redeclaration found for identifier: " << idToken->str_ptr << std::endl;
    return table->PutSymbol(idToken->str_ptr, steType, scanner->getLineNum());
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
    // std::cout << "Program parsed, creating AST node" << std::endl;
    AST* programAST = make_ast_node(ast_program, programStatements);
    // std::cout << "AST node created" << std::endl;

    return programAST;
}

ast_list* Parser::parseProgram() {
    currentToken = scanner->Scan();
    ast_list* declList = parseDeclList();
    
    return declList;
}


ast_list* Parser::parseDeclList() {
    ast_list* declList = nullptr;

    if(currentToken->type == lx_eof) {
        std::cout << "End of file reached, returning empty list" << std::endl;
        return declList;
    }
    else{//not lambda, then it is a declaration
        AST* decl = parseDecl();

        match(lx_semicolon);

        declList = cons_ast(decl, parseDeclList());

        return declList;
    }
}

STE_TYPE getSTE_type(j_type typeNode) {
    switch (typeNode) {
        case type_integer:
            return STE_INT;
        case type_string:
            return STE_STRING;
        case type_boolean:
            return STE_BOOLEAN;
        default:
            return STE_NONE;
    }

}

AST* Parser::parseDecl() {
    AST* declNode = nullptr;
    TOKEN* idToken = nullptr;
    j_type typeNode = type_none;
    STEntry* STE = nullptr;
    
    switch (currentToken->type) {
        case kw_var: {
            std::cout << "Found var keyword" << std::endl;
            match(kw_var);
            idToken = match(lx_identifier);
            std::cout << "Found identifier: " << idToken->str_ptr << std::endl;
            match(lx_colon);
            std::cout << "Found colon, parsing type" << std::endl;
            typeNode = parseType();
            
            STE = checkAndAddSymbol(idToken, getSTE_type(typeNode));
            declNode = make_ast_node(ast_var_decl, STE, typeNode);
            break;
        }
        
        case kw_constant: {
            std::cout << "Found constant keyword" << std::endl;
            match(kw_constant);
            idToken = match(lx_identifier);
            std::cout << "Found identifier: " << idToken->str_ptr << std::endl;
            match(lx_eq);
            std::cout << "Found equals sign, parsing expression" << std::endl;
            
            // Check for redeclarations, if not, put it in ST
            STE = checkAndAddSymbol(idToken, STE_INT);
            
            // Parse the expression
            AST* exprNode = parseExpr();
            STE->ConstValue = eval_ast_expr(scanner->fd, exprNode);
            std::cout << "Evaluated constant expression: " << STE->ConstValue << std::endl;
            
            // Create the const_decl node with expression
            declNode = make_ast_node(ast_const_decl, STE, exprNode);
            break;
        }
        
        default: {
            had_error = true;
            std::cerr << "Syntax Error: Expected a declaration but found " 
                      << getTokenTypeName(currentToken->type) 
                      << "." << std::endl;
            break;
        }
    }
    
    return declNode;
}


j_type Parser::parseType() {
    switch (currentToken->type) {
        case kw_integer:
            match(kw_integer);
            return type_integer;

            break;
        case kw_string:
            match(kw_string);
            return type_string;

            break;
        case kw_bool:
            match(kw_bool);
            return type_boolean;
            // break statement not needed here because of the return statement

        default:
            had_error = true;
            std::cerr << "Syntax Error: Expected a type but found " 
                      << getTokenTypeName(currentToken->type) 
                      << "." << std::endl;
    }
    return type_none;
}


AST* Parser::parseExpr() {
    AST* leftNode = parseExpr2();
    return parseExprTail(leftNode);
}

AST* Parser::parseExprTail(AST* leftNode) {
    if (currentToken->type == kw_and || currentToken->type == kw_or) {
        AST_type nodeType;
        
        if (currentToken->type == kw_and) {
            match(kw_and);
            nodeType = ast_and;
        } else {
            match(kw_or);
            nodeType = ast_or;
        }
        
        AST* rightNode = parseExpr2();
        AST* binaryNode = make_ast_node(nodeType, leftNode, rightNode);
        return parseExprTail(binaryNode);
    }
    
    return leftNode;
}

AST* Parser::parseExpr2() {
    AST* leftNode = parseExpr3();
    return parseExpr2Tail(leftNode);
}

AST* Parser::parseExpr2Tail(AST* leftNode) {
    if (currentToken->type == lx_eq || currentToken->type == lx_neq || 
        currentToken->type == lx_lt || currentToken->type == lx_le ||
        currentToken->type == lx_gt || currentToken->type == lx_ge) {
            
        AST_type nodeType;
        
        switch (currentToken->type) {
            case lx_eq:
                match(lx_eq);
                nodeType = ast_eq;
                break;
            case lx_neq:
                match(lx_neq);
                nodeType = ast_neq;
                break;
            case lx_lt:
                match(lx_lt);
                nodeType = ast_lt;
                break;
            case lx_le:
                match(lx_le);
                nodeType = ast_le;
                break;
            case lx_gt:
                match(lx_gt);
                nodeType = ast_gt;
                break;
            case lx_ge:
                match(lx_ge);
                nodeType = ast_ge;
                break;
            default:
                had_error = true;
                std::cerr << "Unexpected relational operator" << std::endl;
                return nullptr;
        }
        
        AST* rightNode = parseExpr3();
        AST* binaryNode = make_ast_node(nodeType, leftNode, rightNode);
        return parseExpr2Tail(binaryNode);
    }
    
    return leftNode;
}

AST* Parser::parseExpr3() {
    AST* leftNode = parseExpr4();
    return parseExpr3Tail(leftNode);
}

AST* Parser::parseExpr3Tail(AST* leftNode) {
    if (currentToken->type == lx_plus || currentToken->type == lx_minus) {
        AST_type nodeType;
        
        if (currentToken->type == lx_plus) {
            match(lx_plus);
            nodeType = ast_plus;
        } else {
            match(lx_minus);
            nodeType = ast_minus;
        }
        
        AST* rightNode = parseExpr4();
        AST* binaryNode = make_ast_node(nodeType, leftNode, rightNode);
        return parseExpr3Tail(binaryNode);
    }
    
    return leftNode;
}

AST* Parser::parseExpr4() {
    AST* leftNode = parseExpr5();
    return parseExpr4Tail(leftNode);
}

AST* Parser::parseExpr4Tail(AST* leftNode) {
    if (currentToken->type == lx_star || currentToken->type == lx_slash) {
        AST_type nodeType;
        
        if (currentToken->type == lx_star) {
            match(lx_star);
            nodeType = ast_times;
        } else {
            match(lx_slash);
            nodeType = ast_divide;
        }
        
        AST* rightNode = parseExpr5();
        AST* binaryNode = make_ast_node(nodeType, leftNode, rightNode);
        return parseExpr4Tail(binaryNode);
    }
    
    return leftNode;
}

AST* Parser::parseExpr5() {
    if (currentToken->type == lx_minus || currentToken->type == kw_not) {
        AST_type nodeType;
        
        if (currentToken->type == lx_minus) {
            match(lx_minus);
            nodeType = ast_uminus;
        } else {
            match(kw_not);
            nodeType = ast_not;
        }
        
        match(lx_lparen);
        AST* exprNode = parseExpr();
        match(lx_rparen);
        return make_ast_node(nodeType, exprNode);
    } else {
        return parsePrimaryExpr();
    }
}

AST* Parser::parsePrimaryExpr() {
    AST* node = nullptr;
    
    switch (currentToken->type) {
        case lx_identifier: {
            TOKEN* idToken = match(lx_identifier);
            STEntry* entry = table->GetSymbolFromScopes(idToken->str_ptr);
            if (!entry) {
                had_error = true;
                std::cerr << "Undefined identifier: " << idToken->str_ptr << std::endl;
                entry = table->PutSymbol(idToken->str_ptr, STE_INT, scanner->getLineNum());
            }
            
            node = make_ast_node(ast_var, entry);
            node = parsePrimaryExprTail(node);
            break;
        }
        
        case lx_integer: {
            TOKEN* intToken = match(lx_integer);
            node = make_ast_node(ast_integer, intToken->value);
            break;
        }
        
        case lx_string: {
            TOKEN* strToken = match(lx_string);
            node = make_ast_node(ast_string, strToken->str_ptr);
            break;
        }
        
        case kw_true: {
            match(kw_true);
            node = make_ast_node(ast_boolean, 1);
            break;
        }
        
        case kw_false: {
            match(kw_false);
            node = make_ast_node(ast_boolean, 0);
            break;
        }
        
        case lx_lparen: {
            match(lx_lparen);
            node = parseExpr();
            match(lx_rparen);
            break;
        }
        
        default: {
            had_error = true;
            std::cerr << "Syntax Error: Expected a primary expression but found " 
                      << getTokenTypeName(currentToken->type) 
                      << "." << std::endl;
            node = make_ast_node(ast_integer, 0);
            break;
        }
    }
    
    return node;
}

AST* Parser::parsePrimaryExprTail(AST* idNode) {
    if (currentToken->type == lx_lparen) {
        if (idNode->type != ast_var) {
            had_error = true;
            std::cerr << "Expected a function name." << std::endl;
            return idNode;
        }
        
        STEntry* functionEntry = idNode->f.a_var.var;
        ast_list* arguments = nullptr;
        
        match(lx_lparen);
        
        if (currentToken->type != lx_rparen) {
            AST* arg = parseExpr();
            arguments = cons_ast(arg, nullptr);
            
            ast_list* current = arguments;
            
            while (currentToken->type == lx_comma) {
                match(lx_comma);
                arg = parseExpr();
                current->tail = cons_ast(arg, nullptr);
                current = current->tail;
            }
        }
        
        match(lx_rparen);
        return make_ast_node(ast_call, functionEntry, arguments);
    }
    
    return idNode;
}

// Global scanner and file descriptor pointers
