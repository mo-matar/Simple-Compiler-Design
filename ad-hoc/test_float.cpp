#include "Scanner.h"
#include <iostream>
#include <fstream>
#include <string>

// Create a test file with floating point numbers
void createTestFile(const char* filename) {
    std::ofstream outfile(filename);
    outfile << "3.14\n";
    outfile << "42\n";
    outfile << "5.0\n";
    outfile.close();
}

// Test the scanner with floating point numbers
int main() {
    // Create a test file
    const char* filename = "float_test.txt";
    createTestFile(filename);
    
    // Initialize the scanner
    Scanner* scanner = new Scanner((char*)filename);
    
    // Scan tokens and print their types
    TOKEN* token;
    bool done = false;
    
    while (!done) {
        token = scanner->Scan();
        
        std::cout << "Token type: " << token->type << std::endl;
        
        if (token->type == lx_integer) {
            std::cout << "Found integer: " << token->str_ptr << std::endl;
        } 
        else if (token->type == lx_float) {
            std::cout << "Found float: " << token->str_ptr << std::endl;
        }
        else if (token->type == lx_string) {
            std::cout << "Found string: " << token->str_ptr << std::endl;
        }
        
        // Check for EOF before deleting token
        if (token->type == lx_eof) {
            done = true;
        }
        
        delete token;
    }
    
    delete scanner;
    return 0;
}
