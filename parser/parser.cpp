#include "../include/parser.h"
#include <stdarg.h>
#include <vector>
#include <fstream>

Parser::Parser(FileDescriptor* fd) {
    scanner = new Scanner(fd);
    table = new SymbolTable();
    current_scope = table;
    currentToken = new TOKEN();
    programAST = nullptr;
    had_error = false;
    
    errorFile.open("../tests/output/parse_errors.txt", std::ios::out);
    if (!errorFile.is_open()) {
        std::cout << "Warning: Could not open parse_errors.txt for writing." << std::endl;
    }
}

Parser::~Parser() {
    if (errorFile.is_open()) {
        errorFile.close();
    }
    delete scanner;
    delete table;
    delete currentToken;
}

const char* Parser::getTokenTypeName(LEXEME_TYPE type) {
    switch (type) {
        case lx_identifier: return "Identifier";
        case kw_var: return "Keyword var";
        case kw_integer: return "Keyword integer";
        case kw_string: return "Keyword string";
        case kw_bool: return "Keyword bool";
        case kw_if: return "Keyword if";
        case kw_fi: return "Keyword fi";
        case kw_then: return "Keyword then";
        case kw_else: return "Keyword else";
        case kw_read: return "Keyword read";
        case kw_write: return "Keyword write";
        case lx_semicolon: return "Semicolon";
        case lx_colon: return "Colon";
        case lx_eof: return "End of File";
        case lx_lparen: return "Left Parenthesis";
        case lx_rparen: return "Right Parenthesis";
        case lx_lbracket: return "Left Bracket";
        case lx_rbracket: return "Right Bracket";
        case lx_lsbracket: return "Left Square Bracket";
        case lx_rsbracket: return "Right Square Bracket";
        case lx_comma: return "Comma";
        case lx_dot: return "Dot";
        case lx_plus: return "Plus";
        case lx_minus: return "Minus";
        case lx_star: return "Star";
        case lx_slash: return "Slash";
        case lx_eq: return "Equal";
        case lx_neq: return "Not Equal";
        case lx_lt: return "Less Than";
        case lx_le: return "Less Than or Equal";
        case lx_gt: return "Greater Than";
        case lx_ge: return "Greater Than or Equal";
        case kw_and: return "Keyword and";
        case kw_or: return "Keyword or";
        case kw_constant: return "Keyword constant";
        case kw_function: return "Keyword function";
        case kw_procedure: return "Keyword procedure";
        case kw_return: return "Keyword return";
        case kw_true: return "Keyword true";
        case kw_false: return "Keyword false";
        case kw_while: return "Keyword while";
        case kw_do: return "Keyword do";
        case kw_od: return "Keyword od";
        case kw_begin: return "Keyword begin";
        case kw_end: return "Keyword end";
        case kw_from: return "Keyword from";
        case kw_to: return "Keyword to";
        case kw_by: return "Keyword by";
        case kw_for: return "Keyword for";
        case kw_not: return "Keyword not";
        case kw_float: return "Keyword float";
        case illegal_token: return "Illegal Token";
        case type_integer: return "Type Integer";
        case kw_program: return "Keyword program";
        default: return "Unknown Token Type";
    }
}

TOKEN* Parser::match(LEXEME_TYPE expected) {
    if (currentToken->type == expected) {
        TOKEN* matchedToken = currentToken;
        std::cout << "Matched token: " << getTokenTypeName(currentToken->type) << std::endl;
        currentToken = scanner->Scan();
        return matchedToken;
    } else {
        had_error = true;
        errorFile << "Match Syntax Error: Expected token of type " 
                  << getTokenTypeName(expected) 
                  << " but found " 
                  << getTokenTypeName(currentToken->type) 
                  << "." << "on line:" << scanner->getLineNum() << std::endl;
        exit(1);
        return nullptr;
    }
}

void Parser::checkForRedeclaration(TOKEN* idToken) {
    STEntry* STE = current_scope->GetEntryCurrentScope(idToken->str_ptr);
    if (STE != nullptr){
        had_error = true;
        char error_msg[] = "Syntax Error: Redeclaration of identifier2";
        scanner->fd->ReportError(error_msg);
        errorFile << error_msg << std::endl;
        return;
    }
}

STEntry* Parser::checkAndAddSymbol(TOKEN* idToken, STE_TYPE steType) {
    STEntry* STE = current_scope->GetEntryCurrentScope(idToken->str_ptr);
    if (STE != nullptr){
        had_error = true;
        char error_msg[] = "Syntax Error: Redeclaration of identifier1";
        scanner->fd->ReportError(error_msg);
        errorFile << error_msg << std::endl;
        return nullptr;
    }
    return current_scope->PutSymbol(idToken->str_ptr, steType, scanner->getLineNum());
}

void Parser::scan_and_check_illegal_token() {
    currentToken = scanner->Scan();
    
    if (currentToken->type == illegal_token) {
        had_error = true;
        errorFile << "Error: Illegal token encountered from parser." << std::endl;
    }
}

void Parser::printParsedAST(AST* node) {
    if (node == nullptr) {
        std::cout << "Error: Null AST node encountered." << std::endl;
        return;
    }
    FILE* outputFile = fopen("../tests/output/output_program.txt", "w");

    print_ast_node(outputFile, node);
}

AST* Parser::start_parsing() {
    std::cout << "Starting parsing..." << std::endl;
    
    ast_list* programStatements = parseProgram();
    AST* programAST = make_ast_node(ast_program, programStatements);

    return programAST;
}

ast_list* Parser::parseProgram() {
    currentToken = scanner->Scan();
    match(kw_program);
    ast_list* declList = parseDeclList();
    
    return declList;
}


ast_list* Parser::parseDeclList() {
    ast_list* declList = nullptr;

    if(currentToken->type == lx_eof) {
        return declList;
    }
    else {
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
            match(kw_var);
            idToken = match(lx_identifier);
            match(lx_colon);
            typeNode = parseType();
            
            STE = checkAndAddSymbol(idToken, getSTE_type(typeNode));
            declNode = make_ast_node(ast_var_decl, STE, typeNode);
            break;
        }
        
        case kw_constant: {
            match(kw_constant);
            idToken = match(lx_identifier);
            match(lx_eq);
            
            STE = checkAndAddSymbol(idToken, STE_INT);
            
            AST* exprNode = parseExpr();
            STE->ConstValue = eval_ast_expr(scanner->fd, exprNode);
            
            declNode = make_ast_node(ast_const_decl, STE, make_ast_node(ast_integer, STE->ConstValue));
            break;

        }
        case kw_function: {
            match(kw_function);
            idToken = match(lx_identifier);
            
            STE = checkAndAddSymbol(idToken, STE_ROUTINE);
            
            enter_scope();
            
            match(lx_lparen);
            ste_list* formalsNode = parseFormalList();
            match(lx_colon);
            typeNode = parseType();
            
            STE->Formals = formalsNode;
            STE->ResultType = typeNode;
            
            AST* blockNode = parseBlock();
            
            exit_scope();
            
            declNode = make_ast_node(ast_routine_decl, STE, formalsNode, typeNode, blockNode);
            break;
        }
        
        case kw_procedure: {
            match(kw_procedure);
            idToken = match(lx_identifier);
            
            STE = checkAndAddSymbol(idToken, STE_ROUTINE);
            
            enter_scope();
            
            match(lx_lparen);
            ste_list* formalsNode = parseFormalList();
            
            STE->Formals = formalsNode;
            STE->ResultType = type_none;
            
            AST* blockNode = parseBlock();
            
            exit_scope();
            
            declNode = make_ast_node(ast_routine_decl, STE, formalsNode, type_none, blockNode);
            break;
        }        case kw_begin: {
            enter_scope();
            declNode = parseBlock();
            exit_scope();
            break;
        }
        
        default: {
            had_error = true;
            errorFile << "Syntax Error: Expected a declaration but found " 
                      << getTokenTypeName(currentToken->type) 
                      << "." << std::endl;
            break;
        }
    }
    
    return declNode;
}

AST* Parser::parseBlock() {
    AST* blockNode = nullptr;
    match(kw_begin);
    ste_list* varDeclList = parseVarDeclList();
    ast_list* stmtList = parseStmtList();

    match(kw_end);
    blockNode = make_ast_node(ast_block, varDeclList, stmtList);
    
    return blockNode;
}

j_type Parser::parseType() {
    switch (currentToken->type) {
        case kw_integer:
            match(kw_integer);
            return type_integer;
        case kw_string:
            match(kw_string);
            return type_string;
        case kw_bool:
            match(kw_bool);
            return type_boolean;
        default:
            had_error = true;
            errorFile << "Syntax Error: Expected a type but found " 
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
                errorFile << "Unexpected relational operator" << std::endl;
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
            STEntry* entry = current_scope->GetSymbolFromScopes(idToken->str_ptr);
            if (!entry) {
                had_error = true;
                errorFile << "Undefined identifier: " << idToken->str_ptr << std::endl;
                entry = current_scope->PutSymbol(idToken->str_ptr, STE_INT, scanner->getLineNum());
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
            errorFile << "Syntax Error: Expected a primary expression but found " 
                      << getTokenTypeName(currentToken->type) 
                      << "on line:" << scanner->fd->GetLineNum() << std::endl;
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
            errorFile << "Expected a function name." << std::endl;
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

ste_list* Parser::parseFormalList() {
    if (currentToken->type == lx_rparen) {
        match(lx_rparen);
        return nullptr;
    }
    
    std::vector<STEntry*> params;
    
    while (true) {
        TOKEN* idToken = match(lx_identifier);
        match(lx_colon);
        j_type typeNode = parseType();
        
        STEntry* STE = checkAndAddSymbol(idToken, getSTE_type(typeNode));
        params.push_back(STE);
        
        if(currentToken->type == lx_rparen) {
            match(lx_rparen);
            break;
        }
        match(lx_comma);
    }
    
    ste_list* formals = nullptr;
    for (int i = params.size() - 1; i >= 0; i--) {
        formals = cons_ste(params[i], formals);
    }
    
    return formals;
}

ste_list* Parser::parseVarDeclList() {
    if(noVariableDecl()) 
        return nullptr;
    ste_list* varDeclList = nullptr;
    STEntry* varDecl = parseVarDecl();

    match(lx_semicolon);

    if (currentToken->type == kw_var) {
        varDeclList = cons_ste(varDecl, parseVarDeclList());
    } else {
        varDeclList = cons_ste(varDecl, nullptr);
    }
      return varDeclList;
}

bool Parser::noVariableDecl() {
    if (currentToken->type == lx_identifier || currentToken->type == kw_if ||
        currentToken->type == kw_while || currentToken->type == kw_for ||
        currentToken->type == kw_read || currentToken->type == kw_write ||
        currentToken->type == kw_return || currentToken->type == kw_begin) {
        return true;
    }
    return false;
}


STEntry* Parser::parseVarDecl() {
    match(kw_var);
    TOKEN* idToken = match(lx_identifier);
    match(lx_colon);
    j_type typeNode = parseType();
    
    STEntry* STE = checkAndAddSymbol(idToken, getSTE_type(typeNode));
    
    return STE;
}



ast_list* Parser::parseStmtList() {
    ast_list* stmtList = nullptr;
    
    if (currentToken->type == kw_end) {
        return stmtList;
    }
    
    AST* stmtNode = parseStmt();
    match(lx_semicolon);
    
    stmtList = cons_ast(stmtNode, parseStmtList());
    
    return stmtList;
}

AST* Parser::parseStmt() {
    AST* stmtNode = nullptr;
    switch (currentToken->type) {
        case lx_identifier: {
            TOKEN* idToken = match(lx_identifier);
            STEntry* entry = current_scope->GetSymbolFromScopes(idToken->str_ptr);
            if (entry == nullptr) {
                had_error = true;
                errorFile << "Undefined identifier: " << idToken->str_ptr << std::endl;
                scanner->fd->ReportError(" ");
            }

            return parseStmtIdTail(entry);
            break;
        }
        case kw_if : {
            match(kw_if);
            AST* condNode = parseExpr();
            match(kw_then);
            AST* thenStmtNode = parseStmt();

            stmtNode = parseIfTail(condNode, thenStmtNode);
            break;
        }
        case kw_while: {
            match(kw_while);
            AST* condNode = parseExpr();
            match(kw_do);
            AST* bodyNode = parseStmt();
            match(kw_od);
            stmtNode = make_ast_node(ast_while, condNode, bodyNode);
            break;
        }
        case kw_for : {
            match(kw_for);
            TOKEN* idToken = match(lx_identifier);
            STEntry* entry = current_scope->GetSymbolFromScopes(idToken->str_ptr);
            if(entry == nullptr) {
                had_error = true;
                errorFile << "Undefined identifier: " << idToken->str_ptr << "on line: " 
                          << scanner->getLineNum() << std::endl;
            }
            match(lx_colon_eq);
            AST* lowerBoundNode = parseExpr();
            match(kw_to);
            AST* upperBoundNode = parseExpr();
            match(kw_do);
            AST* bodyNode = parseStmt();
            match(kw_od);
            stmtNode = make_ast_node(ast_for, entry, lowerBoundNode, upperBoundNode, bodyNode);
            break;
        }

        case kw_read: {
            match(kw_read);
            match(lx_lparen);
            TOKEN* idToken = match(lx_identifier);
            STEntry* entry = current_scope->GetSymbolFromScopes(idToken->str_ptr);
            if (entry == nullptr) {
                had_error = true;
                errorFile << "Undefined identifier: " << idToken->str_ptr << std::endl;
                scanner->fd->ReportError(" ");
            }
            match(lx_rparen);
            stmtNode = make_ast_node(ast_read, entry);
            break;
        }

        case kw_write: {
            match(kw_write);
            match(lx_lparen);
            TOKEN* idToken = match(lx_identifier);
            STEntry* entry = current_scope->GetSymbolFromScopes(idToken->str_ptr);
            if (entry == nullptr) {
                had_error = true;
                errorFile << "Undefined identifier: " << idToken->str_ptr << std::endl;
                scanner->fd->ReportError(" ");
            }
            match(lx_rparen);
            stmtNode = make_ast_node(ast_write, entry);
            break;
        }
        case kw_return: {
            match(kw_return);
            match(lx_lparen);
            AST* exprNode = parseExpr();
            match(lx_rparen);
            stmtNode = make_ast_node(ast_return, exprNode);
            break;
        }

        case kw_begin: {
            //inner scope inside a block
            enter_scope();
            stmtNode = parseBlock();
            exit_scope();
            break;
        }
        default: {
            had_error = true;
            errorFile << "Syntax Error: Expected a statement but found " 
                      << getTokenTypeName(currentToken->type) 
                      << "." << std::endl;
            break;
        }
     }


    return stmtNode;
}


AST* Parser::parseStmtIdTail(STEntry* entry) {
    AST* stmtNode = nullptr;
    
    if(currentToken->type == lx_colon_eq){
        //assignment statement
        match(lx_colon_eq);
        AST* exprNode = parseExpr();
        stmtNode = make_ast_node(ast_assign, entry, exprNode);

    }
    else if(currentToken->type == lx_lparen){
        //note from grammar that the arg_list is the same as primary_expr_tail
        stmtNode = parsePrimaryExprTail(make_ast_node(ast_var, entry));}
        
    else {
        had_error = true;
        errorFile << "Syntax Error: Expected assignment or function call but found " 
                  << getTokenTypeName(currentToken->type) 
                  << "." << "on line: " << scanner->getLineNum() << std::endl;
        

    }

    return stmtNode;

}


AST* Parser::parseIfTail(AST* condNode, AST* thenStmtNode) {
    AST* stmtNode = nullptr;
    
    if (currentToken->type == kw_else) {
        match(kw_else);
        AST* elseStmtNode = parseStmt();
        stmtNode = make_ast_node(ast_if, condNode, thenStmtNode, elseStmtNode);
        match(kw_fi);
    } else {
        match(kw_fi);
        stmtNode = make_ast_node(ast_if, condNode, thenStmtNode, nullptr);
    }
    
    return stmtNode;
}




