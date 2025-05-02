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
    STE_DOUBLE,
    STE_CHAR,
    STE_BOOLEAN,
    STE_FUNCTION,
    TYPE_SIZE  //this should be the last element to represent the size of enum
};

class STEntry {
public:
     
    
    // Member variables
    STEntry* Next;      // Pointer to next entry in symbol table (for chaining)
    STE_TYPE Type;      // Type of the symbol
    char Name[128];     // Name of the symbol
    int Dimensions;     // For arrays, number of dimensions (0 for non-arrays)
    int Scope;          // Scope level (for functions, 0 for non-functions)
    int Size;           // Size in bytes
    bool IsInitialized; // Whether the variable is initialized
    bool IsUsed;        // Whether the variable is used

    STEntry();
    STEntry(const char* name, STE_TYPE type);
    char* toString(); // Convert entry to string representation
    void print(FILE* fp); // Print entry to file
    static STE_TYPE getType(const char* str); // Get type from string
    static const char* getTypeString(STE_TYPE type); // Get string representation of type
    static int getTypeSize(STE_TYPE type); // Get size of type in bytes
    int getDimensions(); // Get number of dimensions for arrays
     int getScope(); // Get scope level for functions
     bool isInitialized(); // Check if variable is initialized
     bool isUsed(); // Check if variable is used
     void setInitialized( bool initialized); // Set variable as initialized
    void setUsed( bool used); // Set variable as used
    void setDimensions( int dimensions); // Set number of dimensions for arrays
    void setScope( int scope); // Set scope level for functions
    void setSize( int size); // Set size in bytes
};

// External declarations for global variables
extern char* STE_TYPE_STR[TYPE_SIZE];
extern char str[128];

#endif // SYMBOL_TABLE_ENTRY_H
