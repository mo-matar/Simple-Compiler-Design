#include <stdio.h>
#include "ast.h"
#include "symbol_table_entry.h"
#include "FileDescriptor.h"

int main() {
    // Create an output file
    FILE *output_file = fopen("ast_test.txt", "w");
    if (!output_file) {
        printf("Error: Could not open ast_test.txt for writing\n");
        return 1;
    }
    
    // Create a FileDescriptor for error reporting
    FileDescriptor fd;
    
    // Create symbol table entries for our simple test
    STEntry *x_entry = new STEntry("x", STE_INT);
    STEntry *y_entry = new STEntry("y", STE_INT);
    
    // Create a simple if statement: if x > y then read x else read y
    
    // Create the comparison expression: x > y
    AST *comparison = make_ast_node(ast_gt, 
        make_ast_node(ast_var, x_entry),
        make_ast_node(ast_var, y_entry));
    
    // Create the statements for true and false branches
    AST *read_x = make_ast_node(ast_read, x_entry);
    AST *read_y = make_ast_node(ast_read, y_entry);
    
    // Create the if statement
    AST *if_stmt = make_ast_node(ast_if, comparison, read_x, read_y);
    
    // Create a block that contains just the if statement and variable declarations
    ste_list *vars = cons_ste(x_entry, cons_ste(y_entry, NULL));
    ast_list *block_stmts = cons_ast(if_stmt, NULL);
    AST *block = make_ast_node(ast_block, vars, block_stmts);
    
    // Write header to the output file
    fprintf(output_file, "AST TEST OUTPUT\n");
    fprintf(output_file, "==============\n\n");
    fprintf(output_file, "Simple if statement: if x > y then read x else read y\n\n");
    
    // Print the AST to the output file
    print_ast_node(output_file, block);
    fprintf(output_file, "\n");
    
    // Close the output file
    fclose(output_file);
    
    printf("AST has been written to ast_test.txt\n");
    
    // Clean up (in a real program, we would need to free all the allocated memory)
    
    return 0;
}
