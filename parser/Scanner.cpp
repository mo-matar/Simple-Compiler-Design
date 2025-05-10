#include "Scanner.h"
#include "String.h"
#include <cstring>
#include <cctype>
#include <memory>

// Constructor that takes a filename
Scanner::Scanner(char *nameOfFile) {
    fd = new FileDescriptor(nameOfFile);
}

// Constructor that takes a file descriptor
Scanner::Scanner(FileDescriptor *fileDescriptor) {
    fd = fileDescriptor;
}

// Define keywords for the language
const int KEYWORD_COUNT = 30;
const std::string KEYWORDS[KEYWORD_COUNT] = {
        "and", "begin", "bool", "by", "constant",
        "do", "else", "end", "false", "fi", "float", "for", "from",
        "function", "if", "integer", "not", "od", "or", "procedure",
        "program", "read", "return", "string", "then", "to", "true",
        "var", "while", "write"
};

// Map keywords to their corresponding token types
LEXEME_TYPE getKeywordType(const std::string& keyword) {
    if (keyword == "and") return kw_and;
    if (keyword == "begin") return kw_begin;
    if (keyword == "bool") return kw_bool;
    if (keyword == "by") return kw_by;
    if (keyword == "constant") return kw_constant;
    if (keyword == "do") return kw_do;
    if (keyword == "else") return kw_else;
    if (keyword == "end") return kw_end;
    if (keyword == "false") return kw_false;
    if (keyword == "fi") return kw_fi;
    if (keyword == "float") return kw_float;
    if (keyword == "for") return kw_for;
    if (keyword == "from") return kw_from;
    if (keyword == "function") return kw_function;
    if (keyword == "if") return kw_if;
    if (keyword == "integer") return kw_integer;
    if (keyword == "not") return kw_not;
    if (keyword == "od") return kw_od;
    if (keyword == "or") return kw_or;
    if (keyword == "procedure") return kw_procedure;
    if (keyword == "program") return kw_program;
    if (keyword == "read") return kw_read;
    if (keyword == "return") return kw_return;
    if (keyword == "string") return kw_string;
    if (keyword == "then") return kw_then;
    if (keyword == "to") return kw_to;
    if (keyword == "true") return kw_true;
    if (keyword == "var") return kw_var;
    if (keyword == "while") return kw_while;
    if (keyword == "write") return kw_write;

    // Should never reach here if checkKeyword is called first
    return illegal_token;
}

// Scan the input file and identify the next token
TOKEN* Scanner::Scan() {
    std::unique_ptr<TOKEN> token(new TOKEN());
    char currentChar = fd->GetChar();

    // Skip whitespace
    if (isSpace(currentChar)) {
        skipSpaces(currentChar);
        // Recursively call Scan() after skipping spaces
        return Scan();
    }

    // Check for identifiers (variables, keywords)
    if (isStartID(currentChar)) {
        return get_id(currentChar);
    }

    // Check for numeric literals
    if (isdigit(currentChar) != 0) {
        return get_int(currentChar, ' ');
    }

    // Handle possible negative number or minus operator
    if (currentChar == '-') {
        char nextChar = fd->GetChar();
        if (isdigit(nextChar) != 0) {
            return get_int(nextChar, '-');
        } else {
            if (!isSpace(nextChar) && nextChar != EOF) {
                fd->UngetChar(nextChar);
            }
            token->type = lx_minus;
            return token.release();
        }
    }

    // Handle string literals
    if (currentChar == '"') {
        char nextChar = fd->GetChar();
        token.reset(get_string(nextChar));
        if (token->str_ptr == "error") {
            fd->ReportError("Expected end of string literal (closing quote)");
            // Instead of exiting, return an error token
            token->type = illegal_token;
            return token.release();
        }
        return token.release();
    }

    // Handle comments
    if (currentChar == '#') {
        if (fd->GetChar() == '#') {
            skipComments();
            return Scan(); // Skip comments and continue scanning
        } else {
            fd->ReportError("Comments must start with ## (not just #)");
            // Instead of exiting, return an error token
            token->type = illegal_token;
            token->str_ptr = currentChar;
            return token.release();
        }
    }

    // Handle end of file
    if (currentChar == EOF) {
        token->type = lx_eof;
        return token.release();
    }

    // Handle operators and punctuation
    switch (currentChar) {
        case '(': token->type = lx_lparen; break;
        case ')': token->type = lx_rparen; break;
        case '{': token->type = lx_lbracket; break;
        case '}': token->type = lx_rbracket; break;
        case '[': token->type = lx_lsbracket; break;
        case ']': token->type = lx_rsbracket; break;
        case '.': token->type = lx_dot; break;
        case ';': token->type = lx_semicolon; break;
        case ',': token->type = lx_comma; break;
        case '+': token->type = lx_plus; break;
        case '*': token->type = lx_star; break;
        case '/': token->type = lx_slash; break;
        case '=': token->type = lx_eq; break;

            // Multi-character operators
        case ':': {
            char nextChar = fd->GetChar();
            if (nextChar == '=') {
                token->type = lx_colon_eq;
            } else {
                token->type = lx_colon;
                fd->UngetChar(nextChar);
            }
            break;
        }
        case '!': {
            char nextChar = fd->GetChar();
            if (nextChar == '=') {
                token->type = lx_neq;
            } else {
                fd->ReportError("Expected '=' after '!' (for inequality operator)");
                // Instead of exiting, return an error token
                token->type = illegal_token;
                fd->UngetChar(nextChar); // Put back the unexpected character
            }
            break;
        }
        case '<': {
            char nextChar = fd->GetChar();
            if (nextChar == '=') {
                token->type = lx_le;
            } else {
                token->type = lx_lt;
                fd->UngetChar(nextChar);
            }
            break;
        }
        case '>': {
            char nextChar = fd->GetChar();
            if (nextChar == '=') {
                token->type = lx_ge;
            } else {
                token->type = lx_gt;
                fd->UngetChar(nextChar);
            }
            break;
        }
        default:
            token->type = illegal_token;
            token->str_ptr = currentChar;
            fd->ReportError(" illegal token");
    }

    return token.release();
}

// Process identifiers and keywords
TOKEN* Scanner::get_id(char firstChar) {
    const int MAX_ID_LENGTH = 1024;
    std::unique_ptr<char[]> buffer(new char[MAX_ID_LENGTH]);

    // Store the first character
    buffer[0] = firstChar;
    int position = 0;

    // Read remaining identifier characters
    char nextChar = fd->GetChar();
    while (isalpha(nextChar) || nextChar == '_' || isdigit(nextChar)) {
        if (position < MAX_ID_LENGTH - 2) { // Leave room for null terminator
            buffer[++position] = nextChar;
        }
        nextChar = fd->GetChar();
    }

    // Push back non-identifier character
    if (!isSpace(nextChar) && nextChar != EOF) {
        fd->UngetChar(nextChar);
    }

    // Null-terminate the string
    buffer[++position] = '\0';

    // Create and fill the token
    std::unique_ptr<TOKEN> token(new TOKEN());
    token->str_ptr = buffer.get();

    // Check if it's a keyword
    if (checkKeyword(buffer.get())) {
        token->type = getKeywordType(buffer.get());
    } else {
        token->type = lx_identifier;
    }

    return token.release();
}

// Process string literals
TOKEN* Scanner::get_string(char firstChar) {
    const int MAX_STRING_LENGTH = 1024;
    std::unique_ptr<char[]> buffer(new char[MAX_STRING_LENGTH]);

    char currentChar = firstChar;
    int position = -1;

    // Read characters until closing quote or error
    while (currentChar != '"' && currentChar != '\n' && currentChar != EOF) {
        if (position < MAX_STRING_LENGTH - 2) { // Leave room for null terminator
            buffer[++position] = currentChar;
        }
        currentChar = fd->GetChar();
    }

    // Create and fill the token
    std::unique_ptr<TOKEN> token(new TOKEN());

    if (currentChar == '"') {
        // Properly terminated string
        buffer[++position] = '\0';
        token->str_ptr = buffer.get();
        token->type = lx_string;
    } else {
        // Unterminated string
        token->str_ptr = "error";
    }

    return token.release();
}

// Process integer literals, potentially calling get_float
TOKEN* Scanner::get_int(char firstChar, char sign) {
    const int MAX_NUMBER_LENGTH = 1024;
    std::unique_ptr<char[]> buffer(new char[MAX_NUMBER_LENGTH]);
    int position = -1;

    // Handle sign if present
    if (sign != ' ') {
        buffer[++position] = sign;
    }

    char currentChar = firstChar;

    // Read digits
    do {
        if (position < MAX_NUMBER_LENGTH - 2) { // Leave room for null terminator
            buffer[++position] = currentChar;
        }
        currentChar = fd->GetChar();
    } while (isdigit(currentChar) != 0);

    // Check for decimal point (float)
    if (currentChar == '.') {
        return get_float(fd->GetChar(), buffer.get(), position + 1);
    }
        // Check for invalid characters
    else if (isalpha(currentChar)) {
        fd->ReportError("Invalid character in integer literal");
        // Instead of exiting, create and return an error token
        std::unique_ptr<TOKEN> token(new TOKEN());
        token->type = illegal_token;
        // Skip until we find a space or suitable delimiter
        while (!isSpace(currentChar) && currentChar != EOF &&
               !strchr("();,{}[]", currentChar)) {
            currentChar = fd->GetChar();
        }
        fd->UngetChar(currentChar); // Put back the delimiter
        return token.release();
    }
        // Handle valid integer
    else {
        // Push back non-digit character
        if (!isSpace(currentChar) && currentChar != EOF) {
            fd->UngetChar(currentChar);
        }

        // Null-terminate the string
        buffer[++position] = '\0';

        // Create and fill token
        std::unique_ptr<TOKEN> token(new TOKEN());
        token->str_ptr = buffer.get();
        token->type = lx_integer;

        return token.release();
    }
}

// Process floating point literals
TOKEN* Scanner::get_float(char firstChar, char* buffer, int position) {
    // Add decimal point to the buffer
    buffer[position] = '.';

    char currentChar = firstChar;
    int currentPosition = position;

    // Check for at least one digit after decimal point
    if (!isdigit(currentChar)) {
        fd->ReportError("Expected digit after decimal point");
        // Instead of exiting, create and return an error token
        std::unique_ptr<TOKEN> token(new TOKEN());
        token->type = illegal_token;
        // Skip until we find a space or suitable delimiter
        while (!isSpace(currentChar) && currentChar != EOF &&
               !strchr("();,{}[]", currentChar)) {
            currentChar = fd->GetChar();
        }
        fd->UngetChar(currentChar); // Put back the delimiter
        return token.release();
    }

    // Read digits after decimal point
    while (isdigit(currentChar) != 0) {
        buffer[++currentPosition] = currentChar;
        currentChar = fd->GetChar();
    }

    // Check for invalid characters
    if (isalpha(currentChar)) {
        fd->ReportError("Invalid character in float literal");
        // Instead of exiting, create and return an error token
        std::unique_ptr<TOKEN> token(new TOKEN());
        token->type = illegal_token;
        // Skip until we find a space or suitable delimiter
        while (!isSpace(currentChar) && currentChar != EOF &&
               !strchr("();,{}[]", currentChar)) {
            currentChar = fd->GetChar();
        }
        fd->UngetChar(currentChar); // Put back the delimiter
        return token.release();
    }

    // Push back non-digit character
    if (!isSpace(currentChar) && currentChar != EOF) {
        fd->UngetChar(currentChar);
    }

    // Null-terminate the string
    buffer[++currentPosition] = '\0';

    // Create and fill token
    std::unique_ptr<TOKEN> token(new TOKEN());
    token->str_ptr = buffer;
    token->type = lx_float;

    return token.release();
}

// Check if a string is a keyword
bool Scanner::checkKeyword(const char* keyString) {
    for (int i = 0; i < KEYWORD_COUNT; i++) {
        if (strcmp(keyString, KEYWORDS[i].c_str()) == 0) {
            return true;
        }
    }
    return false;
}

// Skip comments in the input
void Scanner::skipComments() {
    char currentChar;

    while (true) {
        currentChar = fd->GetChar();

        // Skip until end of line, end of comment, or end of file
        while (currentChar != '#' && currentChar != '\n' && currentChar != EOF) {
            currentChar = fd->GetChar();
        }

        // End of line or file terminates comment
        if (currentChar == '\n' || currentChar == EOF) {
            return;
        }

        // Possible comment terminator
        if (currentChar == '#') {
            currentChar = fd->GetChar();
            if (currentChar == '#') {
                // Comment properly closed
                return;
            }
            // Otherwise continue skipping
        }
    }
}

// Skip whitespace characters
void Scanner::skipSpaces(char startChar) {
    char c = startChar;

    while (isSpace(c)) {
        c = fd->GetChar();
    }

    // Push back non-space character
    fd->UngetChar(c);
}

// Check if a character is a whitespace
bool Scanner::isSpace(char c) {
    return (c == ' ' || c == '\n' || c == '\t' || c == '\f');
}

// Check if a character can start an identifier
bool Scanner::isStartID(char c) {
    return (c == '_' || isalpha(c));
}

// Destructor - clean up resources
Scanner::~Scanner() {
    // We don
}