#ifndef SYMBOL_TABLE_ENTRY_H
#define SYMBOL_TABLE_ENTRY_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Define the enum for symbol table entry types
enum STE_TYPE {
    STE_NONE,
    STE_INT,
    STE_STRING,
    STE_FLOAT,
    STE_CHAR,
    STE_BOOLEAN,
    STE_ROUTINE,
    TYPE_SIZE  //this should be the last element to represent the size of enum
};

// Forward declaration for j_type
typedef enum J_type_enum {
    type_none,
    type_integer,
    type_float,
    type_boolean,
    type_string
} j_type;

// Forward declaration for parameters
struct ste_list_cell;
typedef struct ste_list_cell ste_list;

class STEntry {
public:
    // Member variables
    STEntry* Next;      // Pointer to next entry in symbol table (for chaining)
    STE_TYPE Type;      // Type of the symbol
    char Name[128];     // Name of the symbol
    int Size;           // Size in bytes
    int Line;          // Line number in source code
    // Additional fields for language features
    int ConstValue;     // Value for constants
    j_type VarType;     // Variable type for type checking
    j_type ResultType;  // Return type for functions
    ste_list* Formals;  // Formal parameters for functions
    int IsConstant;     // Flag indicating if entry is a constant
    
    STEntry();
    STEntry(const char* name, STE_TYPE type, int line = 0);
    char* toString(); // Convert entry to string representation
    void print(FILE* fp); // Print entry to file
    static STE_TYPE getType(const char* str); // Get type from string
    static const char* getTypeString(STE_TYPE type); // Get string representation of type
    static int getTypeSize(STE_TYPE type); // Get size of type in bytes
    
    // Helper functions for type conversion
    static j_type steTypeToJType(STE_TYPE type); // Convert STE_TYPE to j_type
    static STE_TYPE jTypeToSteType(j_type type); // Convert j_type to STE_TYPE
};

// External declarations for global variables
extern char* STE_TYPE_STR[TYPE_SIZE];
extern char str[128];

#endif // SYMBOL_TABLE_ENTRY_H
