//
// Created by momo on 3/3/2025.
//

#ifndef TEST_SCANNER_H
#define TEST_SCANNER_H




#include "FileDescriptor.h"
#include <string>

// Token types enumeration - Organized more systematically
typedef enum {
    /* Literals */
    lx_identifier,  // Variable names, function names, etc.
    lx_integer,     // Integer constants like 123
    lx_string,      // String literals like "hello"
    lx_float,       // Float constants like 3.14

    /* Keywords */
    kw_program,     // 'program' keyword
    kw_var,         // 'var' keyword
    kw_constant,    // 'constant' keyword
    kw_integer,     // 'integer' type keyword
    kw_bool,        // 'bool' type keyword
    kw_string,      // 'string' type keyword
    kw_float,       // 'float' type keyword
    kw_true,        // 'true' boolean literal
    kw_false,       // 'false' boolean literal
    kw_if,          // 'if' conditional
    kw_fi,          // 'fi' end of conditional
    kw_then,        // 'then' clause
    kw_else,        // 'else' clause
    kw_while,       // 'while' loop
    kw_do,          // 'do' block
    kw_od,          // 'od' end of do block
    kw_and,         // 'and' logical operator
    kw_or,          // 'or' logical operator
    kw_read,        // 'read' input operator
    kw_write,       // 'write' output operator
    kw_for,         // 'for' loop
    kw_from,        // 'from' range specification
    kw_to,          // 'to' range specification
    kw_by,          // 'by' step specification
    kw_function,    // 'function' declaration
    kw_procedure,   // 'procedure' declaration
    kw_return,      // 'return' statement
    kw_not,         // 'not' logical operator
    kw_begin,       // 'begin' block
    kw_end,         // 'end' block

    /* Operators and Punctuation */
    lx_lparen,      // Left parenthesis '('
    lx_rparen,      // Right parenthesis ')'
    lx_lbracket,    // Left brace '{'
    lx_rbracket,    // Right brace '}'
    lx_lsbracket,   // Left square bracket '['
    lx_rsbracket,   // Right square bracket ']'
    lx_colon,       // Colon ':'
    lx_dot,         // Dot '.'
    lx_semicolon,   // Semicolon ';'
    lx_comma,       // Comma ','
    lx_colon_eq,    // Assignment ':='
    lx_plus,        // Addition '+'
    lx_minus,       // Subtraction '-'
    lx_star,        // Multiplication '*'
    lx_slash,       // Division '/'
    lx_eq,          // Equal '='
    lx_neq,         // Not equal '!='
    lx_lt,          // Less than '<'
    lx_le,          // Less than or equal '<='
    lx_gt,          // Greater than '>'
    lx_ge,          // Greater than or equal '>='
    lx_eof,         // End of file

    illegal_token   // Invalid token
} LEXEME_TYPE;

/**
 * @class TOKEN
 * @brief Represents a token in the language
 */
class TOKEN {
public:
    LEXEME_TYPE type;  // Type of the token
    std::string str_ptr; // Token's lexeme (textual representation)

    // Constructor
    TOKEN() : type(illegal_token), str_ptr("") {}
};

/**
 * @class Scanner
 * @brief Lexical analyzer for the programming language
 */
class Scanner {
public:
    // Constructors and destructor
    Scanner(char *nameOfFile);
    Scanner(FileDescriptor *fileDescriptor);
    ~Scanner();

    // Main scanning function
    TOKEN* Scan();

    // Token recognition methods
    TOKEN* get_id(char firstChar);
    TOKEN* get_string(char firstChar);
    TOKEN* get_int(char firstChar, char sign);
    TOKEN* get_float(char firstChar, char* buffer, int position);

    // Helper methods
    bool checkKeyword(const char* keyString);
    void skipComments();
    void skipSpaces(char startChar);
    bool isSpace(char c);
    bool isStartID(char c);

    // File handling
    FileDescriptor *fd;

private:
    // Private data and methods can be added here
};

#endif //TEST_SCANNER_H