#include "parser.h"

// Global scanner and file descriptor pointers
Scanner* scanner;
FileDescriptor* fd;

/**
 * @brief Match and consume expected token
 * @param scanner: Scanner object to get tokens from
 * @param expected: Expected token type
 * @return: The consumed token if it matches, otherwise exits with error
 */
TOKEN* match_token(Scanner* scanner, LEXEME_TYPE expected) {
    TOKEN* token = scanner->Scan();
    if (token->type != expected) {
        fd->ReportError((char*)"Expected token type");
        fprintf(stderr, "Expected: %d, Found: %d\n", expected, token->type);
        exit(1);
    }
    return token;
}

Parser::Parser() {
    // Constructor implementation
    lookahead = nullptr;
    had_error = false;  // Initialize error flag
}

Parser::~Parser() {
    // Destructor implementation
    if (lookahead) {
        delete lookahead;
    }
}

/**
 * @brief Get the next token without consuming it
 * @return The next token
 */
TOKEN* Parser::peek_token() {
    if (!lookahead) {
        lookahead = scanner->Scan();
    }
    return lookahead;
}

/**
 * @brief Consume the current token and return it
 * @return The consumed token
 */
TOKEN* Parser::consume_token() {
    TOKEN* token = peek_token();
    lookahead = nullptr;
    return token;
}

/**
 * @brief Match the current token with expected type and consume it
 * @param expected The expected token type
 * @return The consumed token
 */
TOKEN* Parser::match(LEXEME_TYPE expected) {
    TOKEN* token = peek_token();
    if (token->type != expected) {
        char errorMsg[256];
        const char* expectedStr = getTokenTypeName(expected);
        const char* foundStr = getTokenTypeName(token->type);
        
        snprintf(errorMsg, sizeof(errorMsg), "Syntax error: Expected %s, found %s", 
                expectedStr, foundStr);
                
        fd->ReportError(errorMsg);
        
        // Don't exit on error, try to recover
        // For now, we'll just consume the token and continue
        consume_token();
        return token; // Return the mismatched token
    }
    return consume_token();
}

/**
 * @brief Helper function to get a readable name for a token type
 * @param type The token type
 * @return A string representation of the token type
 */
const char* Parser::getTokenTypeName(LEXEME_TYPE type) {
    switch (type) {
        case lx_identifier: return "identifier";
        case lx_integer: return "integer";
        case lx_string: return "string";
        case lx_float: return "float";
        case kw_program: return "keyword 'program'";
        case kw_var: return "keyword 'var'";
        case kw_constant: return "keyword 'constant'";
        case kw_integer: return "keyword 'integer'";
        case kw_bool: return "keyword 'boolean'";
        case kw_string: return "keyword 'string'";
        case kw_if: return "keyword 'if'";
        case kw_fi: return "keyword 'fi'";
        case kw_then: return "keyword 'then'";
        case kw_else: return "keyword 'else'";
        case kw_while: return "keyword 'while'";
        case kw_do: return "keyword 'do'";
        case kw_od: return "keyword 'od'";
        case kw_for: return "keyword 'for'";
        case kw_function: return "keyword 'function'";
        case kw_procedure: return "keyword 'procedure'";
        case kw_return: return "keyword 'return'";
        case kw_begin: return "keyword 'begin'";
        case kw_end: return "keyword 'end'";
        case lx_lparen: return "(";
        case lx_rparen: return ")";
        case lx_colon: return ":";
        case lx_semicolon: return ";";
        case lx_comma: return ",";
        case lx_colon_eq: return ":=";
        case lx_plus: return "+";
        case lx_minus: return "-";
        case lx_star: return "*";
        case lx_slash: return "/";
        case lx_eq: return "=";
        case lx_neq: return "!=";
        case lx_lt: return "<";
        case lx_le: return "<=";
        case lx_gt: return ">";
        case lx_ge: return ">=";
        case lx_eof: return "end of file";
        default: return "unknown token";
    }
}

/**
 * @brief Parse the input file and build AST
 * @param fd: File descriptor to read from
 * @param scanner: Scanner to tokenize the input
 * @return AST: The abstract syntax tree representing the program
 */
AST* Parser::parse(FileDescriptor* fd_arg, Scanner* scanner_arg) {
    // Set up global pointers
    fd = fd_arg;
    scanner = scanner_arg;
    lookahead = nullptr;
    
    // Start the parsing process from the program rule
    return parseProgram();
}

/**
 * @brief Parse a complete program
 * @return AST: Root node of the program's AST
 * 
 * Grammar: program = decl_list ;
 */
AST* Parser::parseProgram() {
    // According to grammar: program = decl_list
    
    // Parse the declaration list which forms the complete program
    AST* declListNode = parseDeclList();
    
    // Program should terminate with EOF
    TOKEN* token = consume_token();
    if (token->type != lx_eof) {
        fd->ReportError((char*)"Expected end of file");
    }
    
    return declListNode;
}

/**
 * @brief Parse a list of declarations
 * @return AST: Node representing the declaration list
 * 
 * Grammar: decl_list = decl ";" decl_list | "λ"
 */
AST* Parser::parseDeclList() {
    // According to grammar: decl_list = decl ";" decl_list | "λ"
    
    // Peek at the next token
    TOKEN* token = peek_token();
    
    // If we're at the end or not at a declaration start, return null (empty list)
    if (token->type == lx_eof || 
        (token->type != kw_var && 
         token->type != kw_constant && 
         token->type != kw_function && 
         token->type != kw_procedure)) {
        
        // No need to put back the token since we only peeked at it
        return nullptr; // Empty declaration list
    }
    
    // Parse a declaration
    AST* declNode = parseDecl();
    
    // Match semicolon separator
    match(lx_semicolon);
    
    // Parse the rest of the declarations recursively
    AST* restDeclList = parseDeclList();
    
    // Create a list of declarations (using cons_ast to build list)
    ast_list* declList = cons_ast(declNode, nullptr);
    
    // If there are more declarations, append them
    if (restDeclList != nullptr) {
        declList->tail = (ast_list*)restDeclList;
    }
    
    // Return the declaration list
    return (AST*)declList;
}

/**
 * @brief Parse a single declaration
 * @return AST: Node representing the declaration
 * 
 * Grammar: decl = "var" id ":" type 
 *               | "constant" id "=" expr 
 *               | "function" id formal_list ":" type block 
 *               | "procedure" id formal_list block
 */
AST* Parser::parseDecl() {
    // Peek at the next token to determine declaration type
    TOKEN* token = consume_token();
    AST* declNode = nullptr;
    
    // If we're at the end or not at a declaration start, return null (empty list)
    if (token->type == kw_var) {
        // Variable declaration: "var" id ":" type
        TOKEN* idToken = match(lx_identifier);
        
        // Add the identifier to symbol table
        STEntry* varEntry = current_scope->PutSymbol(const_cast<char*>(idToken->str_ptr.c_str()));
        
        // Match colon
        match(lx_colon);
        
        // Parse the type
        AST* typeNode = parseType();
        
        // Set the variable type in the symbol table entry
        j_type varType = (j_type)typeNode->type;
        varEntry->VarType = varType;
        
        // Create AST node for variable declaration
        declNode = make_ast_node(ast_var_decl, varEntry, varType);
    }
    else if (token->type == kw_constant) {
        // Constant declaration: "constant" id "=" expr
        TOKEN* idToken = match(lx_identifier);
        
        // Add the identifier to symbol table
        STEntry* constEntry = current_scope->PutSymbol(const_cast<char*>(idToken->str_ptr.c_str()));
        constEntry->IsConstant = 1; // Mark as constant
        
        // Match equals
        match(lx_eq);
        
        // Parse the expression
        AST* exprNode = parseExpr();
        
        // Evaluate the constant expression
        int constValue = eval_ast_expr(fd, exprNode);
        constEntry->ConstValue = constValue;
        
        // Create AST node for constant declaration
        declNode = make_ast_node(ast_const_decl, constEntry, constValue);
    }
    else if (token->type == kw_function) {
        // Function declaration: "function" id formal_list ":" type block
        TOKEN* idToken = match(lx_identifier);
        
        // Add the function to symbol table
        STEntry* funcEntry = current_scope->PutSymbol(const_cast<char*>(idToken->str_ptr.c_str()), STE_ROUTINE);
        
        // Enter a new scope for function parameters
        enter_scope();
        
        // Parse formal parameters
        AST* formalListNode = parseFormalList();
        ste_list* formalList = (ste_list*)formalListNode;
        
        // Match colon and parse return type
        match(lx_colon);
        AST* returnTypeNode = parseType();
        j_type returnType = (j_type)returnTypeNode->type;
        
        // Set function entry attributes
        funcEntry->VarType = type_none; // Not a variable
        funcEntry->ResultType = returnType; // Set return type
        funcEntry->Formals = formalList; // Set parameters
        
        // Parse function body
        AST* bodyNode = parseBlock();
        
        // Exit function scope
        exit_scope();
        
        // Create AST node for function declaration
        declNode = make_ast_node(ast_routine_decl, funcEntry, formalList, returnType, bodyNode);
    }
    else if (token->type == kw_procedure) {
        // Procedure declaration: "procedure" id formal_list block
        TOKEN* idToken = match(lx_identifier);
        
        // Add the procedure to symbol table
        STEntry* procEntry = current_scope->PutSymbol(const_cast<char*>(idToken->str_ptr.c_str()), STE_ROUTINE);
        
        // Enter a new scope for procedure parameters
        enter_scope();
        
        // Parse formal parameters
        AST* formalListNode = parseFormalList();
        ste_list* formalList = (ste_list*)formalListNode;
        
        // Set procedure entry attributes
        procEntry->VarType = type_none; // Not a variable
        procEntry->ResultType = type_none; // No return type for procedures
        procEntry->Formals = formalList; // Set parameters
        
        // Parse procedure body
        AST* bodyNode = parseBlock();
        
        // Exit procedure scope
        exit_scope();
        
        // Create AST node for procedure declaration
        declNode = make_ast_node(ast_routine_decl, procEntry, formalList, type_none, bodyNode);
    }
    else {
        fd->ReportError((char*)"Expected declaration keyword (var, constant, function, procedure)");
        return nullptr;
    }
    
    return declNode;
}

/**
 * @brief Parse a type specification
 * @return AST: Node representing the type
 * 
 * Grammar: type = "integer" | "boolean" | "string"
 */
AST* Parser::parseType() {
    TOKEN* token = consume_token();
    AST* typeNode = nullptr;
    
    if (token->type == kw_integer) {
        typeNode = make_ast_node(ast_integer);
    }
    else if (token->type == kw_bool) {
        typeNode = make_ast_node(ast_boolean);
    }
    else if (token->type == kw_string) {
        typeNode = make_ast_node(ast_string);
    }
    else {
        fd->ReportError((char*)"Expected type specifier (integer, boolean, string)");
    }
    
    return typeNode;
}

/**
 * @brief Parse a formal parameter list
 * @return AST: Node representing the formal parameter list
 * 
 * Grammar: formal_list = "()" | "(" formals ")"
 */
AST* Parser::parseFormalList() {
    // Match opening parenthesis
    match(lx_lparen);
    
    // Check for empty parameter list
    TOKEN* token = consume_token();
    if (token->type == lx_rparen) {
        // Empty parameter list
        return nullptr;
    }
    
    // Put back the token for parsing formals
    lookahead = token;
    
    // Parse the formal parameters
    AST* formalsNode = parseFormals();
    
    // Match closing parenthesis
    match(lx_rparen);
    
    return formalsNode;
}

/**
 * @brief Parse formal parameters
 * @return AST: Node representing the formal parameters
 * 
 * Grammar: formals = id ":" type formals_tail
 */
AST* Parser::parseFormals() {
    // Parse an identifier
    TOKEN* idToken = match(lx_identifier);
    
    // Add parameter to the current scope's symbol table
    STEntry* paramEntry = current_scope->PutSymbol(const_cast<char*>(idToken->str_ptr.c_str()));
    
    // Match colon
    match(lx_colon);
    
    // Parse the type
    AST* typeNode = parseType();
    j_type paramType = (j_type)typeNode->type;
    
    // Set parameter type in symbol table
    paramEntry->VarType = paramType;
    
    // Create list with this parameter
    ste_list* paramList = cons_ste(paramEntry, nullptr);
    
    // Parse additional parameters if any
    AST* moreFormalsTail = parseFormalsTail();
    
    // If there are more parameters, link them to the current one
    if (moreFormalsTail != nullptr) {
        paramList->tail = (ste_list*)moreFormalsTail;
    }
    
    return (AST*)paramList;
}

/**
 * @brief Parse the tail of formal parameters list
 * @return AST: Node representing additional formal parameters
 * 
 * Grammar: formals_tail = "," id ":" type formals_tail | "λ"
 */
AST* Parser::parseFormalsTail() {
    // Check if there are more parameters (comma separator)
    TOKEN* token = consume_token();
    if (token->type != lx_comma) {
        // No more parameters, put back token
        lookahead = token;
        return nullptr;
    }
    
    // Parse an identifier
    TOKEN* idToken = match(lx_identifier);
    
    // Add parameter to the current scope's symbol table
    STEntry* paramEntry = current_scope->PutSymbol(const_cast<char*>(idToken->str_ptr.c_str()));
    
    // Match colon
    match(lx_colon);
    
    // Parse the type
    AST* typeNode = parseType();
    j_type paramType = (j_type)typeNode->type;
    
    // Set parameter type in symbol table
    paramEntry->VarType = paramType;
    
    // Create list with this parameter
    ste_list* paramList = cons_ste(paramEntry, nullptr);
    
    // Parse additional parameters if any
    AST* moreFormalsTail = parseFormalsTail();
    
    // If there are more parameters, link them to the current one
    if (moreFormalsTail != nullptr) {
        paramList->tail = (ste_list*)moreFormalsTail;
    }
    
    return (AST*)paramList;
}

/**
 * @brief Parse a block of code
 * @return AST: Node representing the block
 * 
 * Grammar: block = "begin" var_decl_list stmt_list "end"
 */
AST* Parser::parseBlock() {
    // Match 'begin' keyword
    match(kw_begin);
    
    // Enter a new scope for the block
    enter_scope();
    
    // Parse variable declarations within the block
    AST* varDeclListNode = parseVarDeclList();
    ste_list* varList = (ste_list*)varDeclListNode;
    
    // Parse statements within the block
    AST* stmtListNode = parseStmtList();
    ast_list* stmtList = (ast_list*)stmtListNode;
    
    // Match 'end' keyword
    match(kw_end);
    
    // Exit the block's scope
    exit_scope();
    
    // Create AST node for the block
    return make_ast_node(ast_block, varList, stmtList);
}

/**
 * @brief Parse a list of variable declarations
 * @return AST: Node representing the variable declaration list
 * 
 * Grammar: var_decl_list = var_decl ";" var_decl_list | "λ"
 */
AST* Parser::parseVarDeclList() {
    // Peek at the next token to check if it starts a variable declaration
    TOKEN* token = peek_token();
    
    // If not 'var' keyword, empty list (epsilon production)
    if (token->type != kw_var) {
        return nullptr; // Empty variable declaration list
    }
    
    // Parse one variable declaration
    AST* varDeclNode = parseVarDecl();
    
    // Match semicolon separator
    match(lx_semicolon);
    
    // Recursively parse the rest of the variable declarations
    AST* restVarDeclList = parseVarDeclList();
    
    // Create a list of variable declarations (using cons_ste for symbol table entries)
    STEntry* varEntry = (STEntry*)varDeclNode;
    ste_list* varDeclList = cons_ste(varEntry, nullptr);
    
    // If there are more variable declarations, link them
    if (restVarDeclList != nullptr) {
        varDeclList->tail = (ste_list*)restVarDeclList;
    }
    
    // Return the variable declaration list
    return (AST*)varDeclList;
}

/**
 * @brief Parse a variable declaration
 * @return AST: Node representing the variable declaration
 * 
 * Grammar: var_decl = "var" id ":" type
 */
AST* Parser::parseVarDecl() {
    // Match 'var' keyword
    match(kw_var);
    
    // Match identifier
    TOKEN* idToken = match(lx_identifier);
    
    // Add variable to symbol table
    STEntry* varEntry = current_scope->PutSymbol(const_cast<char*>(idToken->str_ptr.c_str()));
    
    // Match colon
    match(lx_colon);
    
    // Parse type
    AST* typeNode = parseType();
    
    // Set variable type in symbol table
    j_type varType = (j_type)typeNode->type;
    varEntry->VarType = varType;
    
    // Return the symbol table entry for the variable
    return (AST*)varEntry;
}

/**
 * @brief Parse a list of statements
 * @return AST: Node representing the statement list
 * 
 * Grammar: stmt_list = stmt ";" stmt_list | "λ"
 */
AST* Parser::parseStmtList() {
    // Peek at the next token to check if it could start a statement
    TOKEN* token = peek_token();
    
    // Check for tokens that can't start a statement (end of statement list)
    if (token->type == kw_end || token->type == kw_else || 
        token->type == kw_fi || token->type == kw_od || token->type == lx_eof) {
        return nullptr; // Empty statement list
    }
    
    // Parse one statement
    AST* stmtNode = parseStmt();
    
    // Match semicolon separator
    match(lx_semicolon);
    
    // Recursively parse the rest of the statements
    AST* restStmtList = parseStmtList();
    
    // Create a list of statements
    ast_list* stmtList = cons_ast(stmtNode, nullptr);
    
    // If there are more statements, link them
    if (restStmtList != nullptr) {
        stmtList->tail = (ast_list*)restStmtList;
    }
    
    // Return the statement list
    return (AST*)stmtList;
}

/**
 * @brief Parse a statement
 * @return AST: Node representing the statement
 * 
 * Grammar: stmt = id stmt_id_tail
 *               | "if" expr "then" stmt if_tail
 *               | "while" expr "do" stmt "od"
 *               | "for" id ":=" expr "to" expr "do" stmt "od"
 *               | "read" "(" id ")"
 *               | "write" "(" id ")"
 *               | "return" "(" expr ")"
 *               | block
 */
AST* Parser::parseStmt() {
    TOKEN* token = peek_token();
    AST* stmtNode = nullptr;
    
    if (token->type == lx_identifier) {
        // id stmt_id_tail
        token = consume_token();
        
        // Look up the identifier in symbol table
        STEntry* entry = current_scope->GetSymbol(const_cast<char*>(token->str_ptr.c_str()));
        if (!entry) {
            fd->ReportError((char*)"Undeclared identifier");
            fprintf(stderr, "Identifier: %s\n", token->str_ptr.c_str());
            exit(1);
        }
          // Create AST node for the identifier
        AST* idNode = make_ast_node(ast_var, entry);
        
        // Parse the rest of the statement that starts with id
        stmtNode = parseStmtIdTail(idNode);
    }
    else if (token->type == kw_if) {
        // "if" expr "then" stmt if_tail
        consume_token(); // consume 'if'
        
        // Parse condition expression
        AST* condNode = parseExpr();
        
        // Match 'then' keyword
        match(kw_then);
        
        // Parse 'then' statement
        AST* thenStmtNode = parseStmt();
        
        // Parse 'else' part or just 'fi'
        stmtNode = parseIfTail(condNode, thenStmtNode);
    }
    else if (token->type == kw_while) {
        // "while" expr "do" stmt "od"
        consume_token(); // consume 'while'
        
        // Parse condition expression
        AST* condNode = parseExpr();
        
        // Match 'do' keyword
        match(kw_do);
        
        // Parse loop body statement
        AST* bodyNode = parseStmt();
        
        // Match 'od' keyword
        match(kw_od);
        
        // Create while loop AST node
        stmtNode = make_ast_node(ast_while, condNode, bodyNode);
    }
    else if (token->type == kw_for) {
        // "for" id ":=" expr "to" expr "do" stmt "od"
        consume_token(); // consume 'for'
        
        // Parse loop variable
        TOKEN* idToken = match(lx_identifier);
        STEntry* loopVarEntry = current_scope->GetSymbol(const_cast<char*>(idToken->str_ptr.c_str()));
        if (!loopVarEntry) {
            fd->ReportError((char*)"Undeclared loop variable");
            fprintf(stderr, "Identifier: %s\n", idToken->str_ptr.c_str());
            exit(1);
        }
          // Create AST node for loop variable
        AST* loopVarNode = make_ast_node(ast_var, loopVarEntry);
        
        // Match assignment operator
        match(lx_colon_eq);
        
        // Parse start expression
        AST* startExprNode = parseExpr();
        
        // Match 'to' keyword
        match(kw_to);
        
        // Parse end expression
        AST* endExprNode = parseExpr();
        
        // Match 'do' keyword
        match(kw_do);
        
        // Parse loop body statement
        AST* bodyNode = parseStmt();
        
        // Match 'od' keyword
        match(kw_od);
        
        // Create for loop AST node
        stmtNode = make_ast_node(ast_for, loopVarNode, startExprNode, endExprNode, bodyNode);
    }
    else if (token->type == kw_read) {
        // "read" "(" id ")"
        consume_token(); // consume 'read'
        
        // Match opening parenthesis
        match(lx_lparen);
        
        // Parse identifier
        TOKEN* idToken = match(lx_identifier);
        STEntry* varEntry = current_scope->GetSymbol(const_cast<char*>(idToken->str_ptr.c_str()));
        if (!varEntry) {
            fd->ReportError((char*)"Undeclared variable for read");
            fprintf(stderr, "Identifier: %s\n", idToken->str_ptr.c_str());
            exit(1);
        }
          // Create AST node for variable to read
        AST* varNode = make_ast_node(ast_var, varEntry);
        
        // Match closing parenthesis
        match(lx_rparen);
        
        // Create read statement AST node
        stmtNode = make_ast_node(ast_read, varNode);
    }
    else if (token->type == kw_write) {
        // "write" "(" id ")"
        consume_token(); // consume 'write'
        
        // Match opening parenthesis
        match(lx_lparen);
        
        // Parse identifier
        TOKEN* idToken = match(lx_identifier);
        STEntry* varEntry = current_scope->GetSymbol(const_cast<char*>(idToken->str_ptr.c_str()));
        if (!varEntry) {
            fd->ReportError((char*)"Undeclared variable for write");
            fprintf(stderr, "Identifier: %s\n", idToken->str_ptr.c_str());
            exit(1);
        }
          // Create AST node for variable to write
        AST* varNode = make_ast_node(ast_var, varEntry);
        
        // Match closing parenthesis
        match(lx_rparen);
        
        // Create write statement AST node
        stmtNode = make_ast_node(ast_write, varNode);
    }
    else if (token->type == kw_return) {
        // "return" "(" expr ")"
        consume_token(); // consume 'return'
        
        // Match opening parenthesis
        match(lx_lparen);
        
        // Parse return expression
        AST* exprNode = parseExpr();
        
        // Match closing parenthesis
        match(lx_rparen);
        
        // Create return statement AST node
        stmtNode = make_ast_node(ast_return, exprNode);
    }
    else if (token->type == kw_begin) {
        // block
        stmtNode = parseBlock();
    }
    else {
        fd->ReportError((char*)"Expected statement");
        fprintf(stderr, "Found token type: %d\n", token->type);
        exit(1);
    }
    
    return stmtNode;
}

/**
 * @brief Parse the tail part of a statement that starts with an identifier
 * @param idNode AST node representing the identifier
 * @return AST: Node representing the complete statement
 * 
 * Grammar: stmt_id_tail = ":=" expr | arg_list
 */
AST* Parser::parseStmtIdTail(AST* idNode) {
    TOKEN* token = peek_token();
    AST* stmtNode = nullptr;
    
    if (token->type == lx_colon_eq) {
        // Assignment statement: id ":=" expr
        consume_token(); // consume ':='
        
        // Parse right-hand side expression
        AST* exprNode = parseExpr();
        
        // Create assignment statement AST node
        stmtNode = make_ast_node(ast_assign, idNode, exprNode);
    }
    else {
        // Function/procedure call: id arg_list
        AST* argListNode = parseArgList();
        ast_list* argList = (ast_list*)argListNode;
        
        // Create function call AST node
        stmtNode = make_ast_node(ast_call, idNode, argList);
    }
    
    return stmtNode;
}

/**
 * @brief Parse the tail part of an if statement
 * @param condNode AST node representing the condition
 * @param thenStmtNode AST node representing the 'then' part
 * @return AST: Node representing the complete if statement
 * 
 * Grammar: if_tail = "fi" | "else" stmt "fi"
 */
AST* Parser::parseIfTail(AST* condNode, AST* thenStmtNode) {
    TOKEN* token = peek_token();
    AST* elseStmtNode = nullptr;
    
    if (token->type == kw_fi) {
        // Simple if-then: "if" expr "then" stmt "fi"
        consume_token(); // consume 'fi'
        
        // Create if-then AST node with null else part
        return make_ast_node(ast_if, condNode, thenStmtNode, nullptr);
    }
    else if (token->type == kw_else) {
        // If-then-else: "if" expr "then" stmt "else" stmt "fi"
        consume_token(); // consume 'else'
        
        // Parse 'else' statement
        elseStmtNode = parseStmt();
        
        // Match 'fi' keyword
        match(kw_fi);
        
        // Create if-then-else AST node
        return make_ast_node(ast_if, condNode, thenStmtNode, elseStmtNode);
    }
    else {
        fd->ReportError((char*)"Expected 'fi' or 'else' after if-then statement");
        fprintf(stderr, "Found token type: %d\n", token->type);
        exit(1);
    }
    
    // Should never reach here
    return nullptr;
}

/**
 * @brief Parse an argument list for function/procedure calls
 * @return AST: Node representing the argument list
 * 
 * Grammar: arg_list = "()" | "(" args ")"
 */
AST* Parser::parseArgList() {
    // Match opening parenthesis
    match(lx_lparen);
    
    // Check for empty argument list
    TOKEN* token = peek_token();
    if (token->type == lx_rparen) {
        consume_token(); // consume ')'
        return nullptr; // Empty argument list
    }
    
    // Parse arguments
    AST* argsNode = parseArgs();
    
    // Match closing parenthesis
    match(lx_rparen);
    
    return argsNode;
}

/**
 * @brief Parse function arguments
 * @return AST: Node representing the arguments
 * 
 * Grammar: args = expr args_tail
 */
AST* Parser::parseArgs() {
    // Parse first expression
    AST* exprNode = parseExpr();
    
    // Create list with this argument
    ast_list* argList = cons_ast(exprNode, nullptr);
    
    // Parse additional arguments if any
    AST* moreArgsTail = parseArgsTail();
    
    // If there are more arguments, link them
    if (moreArgsTail != nullptr) {
        argList->tail = (ast_list*)moreArgsTail;
    }
    
    return (AST*)argList;
}

/**
 * @brief Parse additional function arguments
 * @return AST: Node representing additional arguments
 * 
 * Grammar: args_tail = "," expr args_tail | "λ"
 */
AST* Parser::parseArgsTail() {
    // Check if there are more arguments (comma separator)
    TOKEN* token = peek_token();
    if (token->type != lx_comma) {
        // No more arguments
        return nullptr;
    }
    
    // Consume the comma
    consume_token();
    
    // Parse next expression
    AST* exprNode = parseExpr();
    
    // Create list with this argument
    ast_list* argList = cons_ast(exprNode, nullptr);
    
    // Parse additional arguments if any
    AST* moreArgsTail = parseArgsTail();
    
    // If there are more arguments, link them
    if (moreArgsTail != nullptr) {
        argList->tail = (ast_list*)moreArgsTail;
    }
    
    return (AST*)argList;
}

/**
 * @brief Parse an expression
 * @return AST: Node representing the expression
 * 
 * Grammar: expr = expr2 expr_tail
 */
AST* Parser::parseExpr() {
    // Parse term
    AST* leftNode = parseExpr2();
    
    // Parse possible further operations
    return parseExprTail(leftNode);
}

/**
 * @brief Parse the tail part of an expression
 * @param leftNode AST node representing the left operand
 * @return AST: Node representing the complete expression
 * 
 * Grammar: expr_tail = rel_conj expr2 expr_tail | "λ"
 */
AST* Parser::parseExprTail(AST* leftNode) {
    TOKEN* token = peek_token();
    
    // Check if token is a relational conjunction (and, or)
    if (token->type != kw_and && token->type != kw_or) {
        return leftNode; // No more operations, return the left operand
    }
    
    // Parse relational conjunction
    AST* opNode = parseRelConj();
    AST_type opType = opNode->type; // Get operation type (ast_and, ast_or)
    
    // Parse right operand
    AST* rightNode = parseExpr2();
    
    // Create binary operation AST node
    AST* exprNode = make_ast_node(opType, leftNode, rightNode);
    
    // Parse possible further operations
    return parseExprTail(exprNode);
}

/**
 * @brief Parse a relational conjunction
 * @return AST: Node representing the relational conjunction
 * 
 * Grammar: rel_conj = "and" | "or"
 */
AST* Parser::parseRelConj() {
    TOKEN* token = consume_token();
    AST* opNode = nullptr;
    
    if (token->type == kw_and) {
        opNode = make_ast_node(ast_and);
    }
    else if (token->type == kw_or) {
        opNode = make_ast_node(ast_or);
    }
    else {
        fd->ReportError((char*)"Expected relational conjunction (and, or)");
        fprintf(stderr, "Found token type: %d\n", token->type);
        exit(1);
    }
    
    return opNode;
}

/**
 * @brief Parse a level 2 expression (term)
 * @return AST: Node representing the level 2 expression
 * 
 * Grammar: expr2 = expr3 expr2_tail
 */
AST* Parser::parseExpr2() {
    // Parse factor
    AST* leftNode = parseExpr3();
    
    // Parse possible further operations
    return parseExpr2Tail(leftNode);
}

/**
 * @brief Parse the tail part of a level 2 expression
 * @param leftNode AST node representing the left operand
 * @return AST: Node representing the complete level 2 expression
 * 
 * Grammar: expr2_tail = rel_op expr3 | "λ"
 */
AST* Parser::parseExpr2Tail(AST* leftNode) {
    TOKEN* token = peek_token();
    
    // Check if token is a relational operator
    if (token->type != lx_eq && token->type != lx_neq &&
        token->type != lx_lt && token->type != lx_le &&
        token->type != lx_gt && token->type != lx_ge) {
        return leftNode; // No more operations, return the left operand
    }
    
    // Parse relational operator
    AST* opNode = parseRelOp();
    AST_type opType = opNode->type; // Get operation type
    
    // Parse right operand
    AST* rightNode = parseExpr3();
    
    // Create binary operation AST node
    return make_ast_node(opType, leftNode, rightNode);
}

/**
 * @brief Parse a relational operator
 * @return AST: Node representing the relational operator
 * 
 * Grammar: rel_op = "=" | "!=" | "<" | "<=" | ">" | ">="
 */
AST* Parser::parseRelOp() {
    TOKEN* token = consume_token();
    AST* opNode = nullptr;
    
    if (token->type == lx_eq) {
        opNode = make_ast_node(ast_eq);
    }
    else if (token->type == lx_neq) {
        opNode = make_ast_node(ast_neq);
    }
    else if (token->type == lx_lt) {
        opNode = make_ast_node(ast_lt);
    }
    else if (token->type == lx_le) {
        opNode = make_ast_node(ast_le);
    }
    else if (token->type == lx_gt) {
        opNode = make_ast_node(ast_gt);
    }
    else if (token->type == lx_ge) {
        opNode = make_ast_node(ast_ge);
    }
    else {
        fd->ReportError((char*)"Expected relational operator (=, !=, <, <=, >, >=)");
        fprintf(stderr, "Found token type: %d\n", token->type);
        exit(1);
    }
    
    return opNode;
}

/**
 * @brief Parse a level 3 expression (add/subtract terms)
 * @return AST: Node representing the level 3 expression
 * 
 * Grammar: expr3 = expr4 expr3_tail
 */
AST* Parser::parseExpr3() {
    // Parse term
    AST* leftNode = parseExpr4();
    
    // Parse possible further operations
    return parseExpr3Tail(leftNode);
}

/**
 * @brief Parse the tail part of a level 3 expression
 * @param leftNode AST node representing the left operand
 * @return AST: Node representing the complete level 3 expression
 * 
 * Grammar: expr3_tail = "+" expr4 expr3_tail | "-" expr4 expr3_tail | "λ"
 */
AST* Parser::parseExpr3Tail(AST* leftNode) {
    TOKEN* token = peek_token();
    
    // Check if token is an addition or subtraction operator
    if (token->type != lx_plus && token->type != lx_minus) {
        return leftNode; // No more operations, return the left operand
    }
    
    // Consume the operator
    token = consume_token();
    AST_type opType;
    
    if (token->type == lx_plus) {
        opType = ast_plus;
    }
    else { // token->type == lx_minus
        opType = ast_minus;
    }
    
    // Parse right operand
    AST* rightNode = parseExpr4();
    
    // Create binary operation AST node
    AST* exprNode = make_ast_node(opType, leftNode, rightNode);
    
    // Parse possible further operations
    return parseExpr3Tail(exprNode);
}

/**
 * @brief Parse a level 4 expression (multiply/divide terms)
 * @return AST: Node representing the level 4 expression
 * 
 * Grammar: expr4 = expr5 expr4_tail
 */
AST* Parser::parseExpr4() {
    // Parse factor
    AST* leftNode = parseExpr5();
    
    // Parse possible further operations
    return parseExpr4Tail(leftNode);
}

/**
 * @brief Parse the tail part of a level 4 expression
 * @param leftNode AST node representing the left operand
 * @return AST: Node representing the complete level 4 expression
 * 
 * Grammar: expr4_tail = "*" expr5 expr4_tail | "/" expr5 expr4_tail | "λ"
 */
AST* Parser::parseExpr4Tail(AST* leftNode) {
    TOKEN* token = peek_token();
    
    // Check if token is a multiplication or division operator
    if (token->type != lx_star && token->type != lx_slash) {
        return leftNode; // No more operations, return the left operand
    }
    
    // Consume the operator
    token = consume_token();
    AST_type opType;
    
    if (token->type == lx_star) {
        opType = ast_times;
    }
    else { // token->type == lx_slash
        opType = ast_divide;
    }
    
    // Parse right operand
    AST* rightNode = parseExpr5();
    
    // Create binary operation AST node
    AST* exprNode = make_ast_node(opType, leftNode, rightNode);
    
    // Parse possible further operations
    return parseExpr4Tail(exprNode);
}

/**
 * @brief Parse a level 5 expression (unary operations or primary expression)
 * @return AST: Node representing the level 5 expression
 * 
 * Grammar: expr5 = unary_op expr5 | primary_expr
 */
AST* Parser::parseExpr5() {
    TOKEN* token = peek_token();
    
    // Check if token is a unary operator (-, not)
    if (token->type == lx_minus || token->type == kw_not) {
        // Parse unary operator
        AST* opNode = parseUnaryOp();
        AST_type opType = opNode->type; // Get operation type
        
        // Parse operand
        AST* operandNode = parseExpr5();
        
        // Create unary operation AST node
        return make_ast_node(opType, operandNode);
    }
    else {
        // Parse primary expression
        return parsePrimaryExpr();
    }
}

/**
 * @brief Parse a unary operator
 * @return AST: Node representing the unary operator
 * 
 * Grammar: unary_op = "-" | "not"
 */
AST* Parser::parseUnaryOp() {
    TOKEN* token = consume_token();
    AST* opNode = nullptr;
    
    if (token->type == lx_minus) {
        opNode = make_ast_node(ast_uminus);
    }
    else if (token->type == kw_not) {
        opNode = make_ast_node(ast_not);
    }
    else {
        fd->ReportError((char*)"Expected unary operator (-, not)");
        fprintf(stderr, "Found token type: %d\n", token->type);
        exit(1);
    }
    
    return opNode;
}

/**
 * @brief Parse a primary expression
 * @return AST: Node representing the primary expression
 * 
 * Grammar: primary_expr = "(" expr ")" | id primary_expr_tail | INTEGER | STRING | "true" | "false"
 */
AST* Parser::parsePrimaryExpr() {
    TOKEN* token = peek_token();
    AST* exprNode = nullptr;
    
    if (token->type == lx_lparen) {
        // "(" expr ")"
        consume_token(); // consume '('
        
        // Parse expression
        exprNode = parseExpr();
        
        // Match closing parenthesis
        match(lx_rparen);
    }
    else if (token->type == lx_identifier) {
        // id primary_expr_tail
        token = consume_token();
        
        // Look up the identifier in symbol table
        STEntry* entry = current_scope->GetSymbol(const_cast<char*>(token->str_ptr.c_str()));
        if (!entry) {
            fd->ReportError((char*)"Undeclared identifier in expression");
            fprintf(stderr, "Identifier: %s\n", token->str_ptr.c_str());
            exit(1);
        }
        
        // Create AST node for the identifier
        AST* idNode = make_ast_node(ast_var, entry);
        
        // Parse possible function call
        exprNode = parsePrimaryExprTail(idNode);
    }
    else if (token->type == lx_integer) {
        // INTEGER literal
        token = consume_token();
        int value = std::stoi(token->str_ptr);
        exprNode = make_ast_node(ast_integer, value);
    }
    else if (token->type == lx_string) {
        // STRING literal
        token = consume_token();
        exprNode = make_ast_node(ast_string, const_cast<char*>(token->str_ptr.c_str()));
    }
    else if (token->type == kw_true) {
        // boolean literal "true"
        consume_token();
        exprNode = make_ast_node(ast_boolean, 1);
    }
    else if (token->type == kw_false) {
        // boolean literal "false"
        consume_token();
        exprNode = make_ast_node(ast_boolean, 0);
    }
    else {
        fd->ReportError((char*)"Expected primary expression");
        fprintf(stderr, "Found token type: %d\n", token->type);
        exit(1);
    }
    
    return exprNode;
}

/**
 * @brief Parse the tail part of a primary expression that starts with an identifier
 * @param idNode AST node representing the identifier
 * @return AST: Node representing the complete primary expression
 * 
 * Grammar: primary_expr_tail = arg_list | "λ"
 */
AST* Parser::parsePrimaryExprTail(AST* idNode) {
    TOKEN* token = peek_token();
    
    if (token->type == lx_lparen) {
        // Function call: id arg_list
        AST* argListNode = parseArgList();
        ast_list* argList = (ast_list*)argListNode;
        
        // Create function call AST node
        return make_ast_node(ast_call, idNode, argList);
    }
    else {
        // Simple variable reference
        return idNode;
    }
}