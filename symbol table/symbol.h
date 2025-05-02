#ifndef SYMBOL_H
#define SYMBOL_H

#include "symbol_table_entry.h"
#include "list.h"

// Symbol Table class
class SymbolTable {
private:
    static const int DEFAULT_SIZE = 19;
    
    STList *slots;            // Pointer to array of STList objects
    int fold_case;            // Non-zero => fold upper to lower case
    int table_size;           // Size of the hash table
    
    // Statistics on hash table effectiveness
    int number_entries;       // Number of entries in table
    int number_probes;        // Number of probes into table
    int number_hits;          // Number of probes that immediately found entry
    int max_search_dist;      // Maximum entries searched
    SymbolTable *next;        // To be used to create a stack of symbol table
    
    // Hash function
    unsigned long hash(char *str);
    
    // Helper method to fold case if needed
    char* processString(char *str);

public:
    SymbolTable();            // Default constructor with fold_case = false
    SymbolTable(int fold_case_flag);
    SymbolTable(int size, int fold_case_flag = 0); // Constructor with table size and optional fold_case
    ~SymbolTable();
    
    void ClearSymbolTable();
    STEntry *GetSymbol(char *str);
    STEntry *PutSymbol(char *str, STE_TYPE type = STE_NONE);
    bool AddEntry(char *str, STE_TYPE type); // Similar to PutSymbol but returns bool
    void PrintSymbolStats(FILE *fp);
    void Reset(int new_size);  // Reset the symbol table with a new size
    
    // Additional helper functions
    void PrintAll(FILE *fp);
    void FindAndPrintEntry(char *str, FILE *fp); // Find and print a specific entry
};

#endif // SYMBOL_H
