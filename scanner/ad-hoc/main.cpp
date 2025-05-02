#include <iostream>
#include <iomanip>
#include <memory>
#include "Scanner.h"
#include "FileDescriptor.h"

// Current Date: 2025-03-03 13:28:07
// User: momo252598

int main() {
    // Hard-coded file path as requested
    const char* filePath = "../test_scanner.txt";

    // Create the file descriptor and scanner
    std::unique_ptr<FileDescriptor> fd(new FileDescriptor(filePath));
    std::unique_ptr<Scanner> scanner(new Scanner(fd.get()));

    // Define token type names array that corresponds to the LEXEME_TYPE enum
    const std::string tokenNames[] = {
            // Literals
            "lx_identifier", "lx_integer", "lx_string", "lx_float",

            // Keywords
            "kw_program", "kw_var", "kw_constant", "kw_integer", "kw_bool", "kw_string", "kw_float",
            "kw_true", "kw_false", "kw_if", "kw_fi", "kw_then", "kw_else", "kw_while", "kw_do", "kw_od",
            "kw_and", "kw_or", "kw_read", "kw_write", "kw_for", "kw_from", "kw_to", "kw_by",
            "kw_function", "kw_procedure", "kw_return", "kw_not", "kw_begin", "kw_end",

            // Operators and Punctuation
            "lx_lparen", "lx_rparen", "lx_lbracket", "lx_rbracket", "lx_lsbracket", "lx_rsbracket",
            "lx_colon", "lx_dot", "lx_semicolon", "lx_comma", "lx_colon_eq",
            "lx_plus", "lx_minus", "lx_star", "lx_slash",
            "lx_eq", "lx_neq", "lx_lt", "lx_le", "lx_gt", "lx_ge", "lx_eof",
            "illegal_token"
    };

    // Find the maximum length of token type names for formatting
    size_t maxTokenNameLength = 0;
    for (const auto& name : tokenNames) {
        maxTokenNameLength = std::max(maxTokenNameLength, name.length());
    }

    // Print header
    std::cout << "Lexical Analysis of file: " << filePath << std::endl;
    std::cout << "Timestamp: 2025-03-03 13:28:07 UTC" << std::endl;
    std::cout << "User: momo252598" << std::endl;
    std::cout << std::string(60, '-') << std::endl;

    // Print column headers
    std::cout << std::left << std::setw(5) << "NO."
              << std::setw(maxTokenNameLength + 5) << "TYPE"
              << "VALUE" << std::endl;
    std::cout << std::string(maxTokenNameLength + 30, '-') << std::endl;

    // Process all tokens
    int tokenCount = 0;
    TOKEN* token = nullptr;

    do {
        // Get next token and manage its memory
        token = scanner->Scan();
        tokenCount++;

        // Print token information
        std::cout << std::left << std::setw(5) << tokenCount
                  << std::setw(maxTokenNameLength + 5) << tokenNames[token->type];

        // Print token value if it exists
        if (!token->str_ptr.empty()) {
            std::cout << "\"" << token->str_ptr << "\"";
        }
        std::cout << std::endl;

        // Check for end of file
        if (token->type == lx_eof) {
            break;
        }

        // Properly delete the token to prevent memory leaks
        delete token;

    } while (true);

    // Delete the last token
    delete token;

    // Print summary
    std::cout << std::string(maxTokenNameLength + 30, '-') << std::endl;
    std::cout << "Total tokens found: " << tokenCount << std::endl;

    return 0;
}