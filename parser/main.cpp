#include <iostream>
#include "parser.h"
#include "ast.h"
#include "Scanner.h"
#include "FileDescriptor.h"
#include "symbol.h"

int main(int argc, char *argv[]) {
    const char* inputFile = "c:\\Users\\Hp\\Desktop\\Compiler Design\\project\\Simple-Compiler-Design\\parser\\example_program.txt";
    const char* outputFile = "c:\\Users\\Hp\\Desktop\\Compiler Design\\project\\Simple-Compiler-Design\\parser\\output_program.txt";
    
    // Allow command line arguments to override default file paths
    if (argc > 1) {
        inputFile = argv[1];
    }
    if (argc > 2) {
        outputFile = argv[2];
    }
    
    // Initialize global scope
    enter_scope();
    
    // Create file descriptor and scanner
    FileDescriptor* fd = new FileDescriptor(inputFile);
    if (!fd->IsOpen()) {
        std::cerr << "Error: Could not open input file: " << inputFile << std::endl;
        delete fd;
        return 1;
    }
    
    // Create scanner and parser
    Scanner* scanner = new Scanner(fd);
    Parser* parser = new Parser();
    
    // Parse the input file
    std::cout << "Parsing file: " << inputFile << std::endl;
    AST* ast = parser->parse(fd, scanner);
    
    // Check if parsing was successful
    if (ast == nullptr) {
        std::cerr << "Error: Parsing failed or empty program." << std::endl;
        delete parser;
        delete scanner;
        delete fd;
        exit_scope();
        return 1;
    }
    
    // Output the AST
    FILE* outFile = fopen(outputFile, "w");
    if (outFile != nullptr) {
        print_ast_node(outFile, ast);
        fclose(outFile);
        std::cout << "AST written to " << outputFile << std::endl;
    } else {
        std::cerr << "Error: Could not open output file." << std::endl;
    }
    
    // Clean up
    delete parser;
    delete scanner;
    delete fd;
    exit_scope();
    
    std::cout << "Parser test completed successfully." << std::endl;
    return 0;
}
