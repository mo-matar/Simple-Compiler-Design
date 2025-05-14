#include "parser.h"
#include <stdarg.h>
#include <vector>
#include <fstream>

Parser::Parser(FileDescriptor* fd) {
    scanner = new Scanner(fd);
    table = new SymbolTable();
    current_scope = table;  // Initialize the global current_scope
    currentToken = new TOKEN();
    programAST = nullptr;
    had_error = false;
    
    // Open error file
    errorFile.open("c:\\Users\\Hp\\Desktop\\Compiler Design\\project\\Simple-Compiler-Design\\parser\\parse_errors.txt", std::ios::out);
    if (!errorFile.is_open()) {
        std::cout << "Warning: Could not open parse_errors.txt for writing." << std::endl;
    }
}

Parser::~Parser() {
    // Close error file
    if (errorFile.is_open()) {
        errorFile.close();
    }
    // Other cleanup as needed
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
        currentToken = scanner->Scan(); // Advance to the next token
        // std::cout << "Next token: " << getTokenTypeName(currentToken->type) << std::endl;
        return matchedToken;
    } else {
        had_error = true;
        errorFile << "Syntax Error: Expected token of type " 
                  << getTokenTypeName(expected) 
                  << " but found " 
                  << getTokenTypeName(currentToken->type) 
                  << "." << "on line:" << scanner->getLineNum() << std::endl;
        exit(1); // Exit on error
        return nullptr;
    }
}

void Parser::checkForRedeclaration(TOKEN* idToken) {
    // Only check in the current scope (not parent scopes)
    STEntry* STE = current_scope->GetEntryCurrentScope(idToken->str_ptr);
    if (STE != nullptr){
        had_error = true;
        char error_msg[] = "Syntax Error: Redeclaration of identifier2";
        scanner->fd->ReportError(error_msg);
        errorFile << error_msg << std::endl;
        return;
    }
    
    // std::cout << "No redeclaration found for identifier: " << idToken->str_ptr << std::endl;
}

STEntry* Parser::checkAndAddSymbol(TOKEN* idToken, STE_TYPE steType) {
    // Only check in the current scope
    STEntry* STE = current_scope->GetEntryCurrentScope(idToken->str_ptr);
    if (STE != nullptr){
        had_error = true;
        char error_msg[] = "Syntax Error: Redeclaration of identifier1";
        scanner->fd->ReportError(error_msg);
        errorFile << error_msg << std::endl;
        return nullptr;
    }
    
    // std::cout << "No redeclaration found for identifier: " << idToken->str_ptr << std::endl;
    return current_scope->PutSymbol(idToken->str_ptr, steType, scanner->getLineNum());
}

void Parser::scan_and_check_illegal_token() {
    currentToken = scanner->Scan();
    // std::cout << "Scanned token type: " << currentToken->type << " (" << getTokenTypeName(currentToken->type) << ")" << std::endl;
    if (currentToken->type == lx_identifier && currentToken->str_ptr) {
        // std::cout << "Identifier value: " << currentToken->str_ptr << std::endl;
    }
    
    if (currentToken->type == illegal_token) {
        had_error = true;
        errorFile << "Error: Illegal token encountered from parser." << std::endl;
    }
    // else std::cout << "no illegal token yet... parser" << std::endl;
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
    match(kw_program);
    ast_list* declList = parseDeclList();
    
    return declList;
}


ast_list* Parser::parseDeclList() {
    ast_list* declList = nullptr;

    if(currentToken->type == lx_eof) {
        // std::cout << "End of file reached, returning empty list" << std::endl;
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
            // std::cout << "Found var keyword" << std::endl;
            match(kw_var);
            idToken = match(lx_identifier);
            // std::cout << "Found identifier: " << idToken->str_ptr << std::endl;
            match(lx_colon);
            // std::cout << "Found colon, parsing type" << std::endl;
            typeNode = parseType();
            
            STE = checkAndAddSymbol(idToken, getSTE_type(typeNode));
            declNode = make_ast_node(ast_var_decl, STE, typeNode);
            break;
        }
        
        case kw_constant: {
            // std::cout << "Found constant keyword" << std::endl;
            match(kw_constant);
            idToken = match(lx_identifier);
            // std::cout << "Found identifier: " << idToken->str_ptr << std::endl;
            match(lx_eq);
            // std::cout << "Found equals sign, parsing expression" << std::endl;
            
            // Check for redeclarations, if not, put it in ST
            STE = checkAndAddSymbol(idToken, STE_INT);
            
            // Parse the expression
            AST* exprNode = parseExpr();
            STE->ConstValue = eval_ast_expr(scanner->fd, exprNode);
            // std::cout << "Evaluated constant expression: " << STE->ConstValue << std::endl;
            
            // Create the const_decl node with expression
            declNode = make_ast_node(ast_const_decl, STE, make_ast_node(ast_integer, STE->ConstValue));
            break;
        }

        case kw_function: {
            // std::cout << "Found function keyword" << std::endl;
            match(kw_function);

            idToken = match(lx_identifier);
            
            // First add the function to the current scope
            STE = checkAndAddSymbol(idToken, STE_ROUTINE);
            
            // Enter a new scope for the function body
            enter_scope();
            
            match(lx_lparen);
            ste_list* formalsNode = parseFormalList();
            match(lx_colon);
            typeNode = parseType();
            
            // Store additional function info
            STE->Formals = formalsNode;
            STE->ResultType = typeNode;
            
            // Here you would parse the function body
            AST* blockNode = parseBlock();
            
            // Exit the function scope
            exit_scope();
            
            declNode = make_ast_node(ast_routine_decl, STE, formalsNode, typeNode, blockNode);
            break;
        }
        
        case kw_procedure: {
            // std::cout << "Found procedure keyword" << std::endl;
            // match(kw_procedure);
            // idToken = match(lx_identifier);
            // std::cout << "Found identifier: " << idToken->str_ptr << std::endl;
            // match(lx_lparen);
            // std::cout << "Found left parenthesis, parsing formals" << std::endl;
            // ste_list* formalsNode = parseFormals();
            // match(lx_rparen);
            // std::cout << "Found right parenthesis, parsing block" << std::endl;
            // AST* blockNode = parseBlock();
            
            // STE = checkAndAddSymbol(idToken, getSTE_type(typeNode));
            // declNode = make_ast_node(ast_routine_decl, STE, formalsNode, blockNode);
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
    // std::cout << "Parsing block..." << std::endl;
    match(kw_begin);
    ste_list* varDeclList = parseVarDeclList();
    ast_list* stmtList = parseStmtList();

    match(kw_end);
    // std::cout << "Block parsed successfully." << std::endl;
    blockNode = make_ast_node(ast_block, varDeclList, stmtList);
    

    return blockNode;
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
        // std::cout << "No formals found, returning empty list" << std::endl;
        return nullptr;
    }
    
    // Collect all parameters first in a vector to maintain order
    std::vector<STEntry*> params;
    
    while (true) {
        TOKEN* idToken = match(lx_identifier);
        // std::cout << "Found identifier: " << idToken->str_ptr << std::endl;
        match(lx_colon);
        j_type typeNode = parseType();
        
        STEntry* STE = checkAndAddSymbol(idToken, getSTE_type(typeNode));
        params.push_back(STE);
        
        if(currentToken->type == lx_rparen) {
            // std::cout << "End of formals list reached" << std::endl;
            match(lx_rparen);
            break;
        }
        match(lx_comma);
    }
    
    // Build the list in the correct order (last to first)
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
// Global scanner and file descriptor pointers

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
    // std::cout << "Found var keyword" << std::endl;

    TOKEN* idToken = match(lx_identifier);
    // std::cout << "Found identifier: " << idToken->str_ptr << std::endl;
    match(lx_colon);
    j_type typeNode = parseType();
    
    STEntry* STE = checkAndAddSymbol(idToken, getSTE_type(typeNode));
    
    return STE;
}



ast_list* Parser::parseStmtList() {
    ast_list* stmtList = nullptr;
    
    if (currentToken->type == kw_end) {
        // std::cout << "End of statement list reached, returning empty list" << std::endl;
        return stmtList;}
    
    
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
            // ast_list* thenStmtNode = parseStmtList();
            // stmtNode = make_ast_node(ast_block, nullptr, thenStmtNode);
            AST* thenStmtNode = parseStmt();

            stmtNode = parseIfTail(condNode, thenStmtNode);
            break;
        }
        case kw_begin: {
            stmtNode = parseBlock();
            // stmtNode = make_ast_node(ast_block, nullptr, stmtList);
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




