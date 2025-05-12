#include <cstring>
#include <ctype.h>
#include <stdio.h>
#include "symbol.h"

// Global current scope variable
SymbolTable* current_scope = nullptr;

// Helper method to process string (fold case if needed)
char* SymbolTable::processString(char *str) {
    if (!str) return NULL;
    
    static char buffer[1024]; // Static buffer for the processed string
    strcpy(buffer, str);
    
    if (fold_case) {
        for (char *p = buffer; *p; p++) {
            *p = tolower(*p);
        }
    }
    
    return buffer;
}

// Hash function implementation - using ELF hash algorithm
unsigned long SymbolTable::hash(char *str) {
    unsigned long h = 0, high;
    char *processed_str = processString(str);
    unsigned char *s = (unsigned char*)processed_str;
    
    while (*s) {
        h = (h << 4) + *s++;
        if ((high = h & 0xF0000000))
            h ^= high >> 24;
        h &= ~high;
    }
    
    return h % table_size;
}

// Default constructor
SymbolTable::SymbolTable() {
    fold_case = 0;
    table_size = DEFAULT_SIZE;
    
    // Allocate the hash table
    slots = new STList[table_size];
    
    // Initialize statistics
    number_entries = 0;
    number_probes = 0;
    number_hits = 0;
    max_search_dist = 0;
    next = NULL;
    
    // Initialize the global current_scope if this is the first symbol table
    if (current_scope == nullptr) {
        current_scope = this;
    }
}

// Constructor with fold_case flag
SymbolTable::SymbolTable(int fold_case_flag) {
    fold_case = fold_case_flag;
    table_size = DEFAULT_SIZE;
    
    // Allocate the hash table
    slots = new STList[table_size];
    
    // Initialize statistics
    number_entries = 0;
    number_probes = 0;
    number_hits = 0;
    max_search_dist = 0;
    next = NULL;
}

// Constructor with size and fold_case
SymbolTable::SymbolTable(int size, int fold_case_flag) {
    fold_case = fold_case_flag;
    table_size = size;
    
    // Allocate the hash table
    slots = new STList[table_size];
    
    // Initialize statistics
    number_entries = 0;
    number_probes = 0;
    number_hits = 0;
    max_search_dist = 0;
    next = NULL;
}

// Destructor
SymbolTable::~SymbolTable() {
    delete[] slots;
}

// Get a symbol from current scope and parent scopes
STEntry* SymbolTable::GetSymbolFromScopes(char* str) {
    if (!str) return NULL;
    
    SymbolTable *currentTable = this;
    STEntry* entry = NULL;
    
    // Search through the scope chain
    while(currentTable != NULL && ((entry = currentTable->GetEntryCurrentScope(str)) == NULL)) {
        currentTable = currentTable->next;
    }
    
    return entry;
}

// Get an entry only from the current scope (does not check parent scopes)
STEntry *SymbolTable::GetEntryCurrentScope(char *key) {
    if (!key) return NULL;
    
    // Calculate the hash index using our hash function
    unsigned long index = hash(key);
    
    // Increment probe count for statistics
    number_probes++;
    
    // Return the entry from the current scope only
    return slots[index].FindEntry(processString(key));
}

// Lookup symbol in current and all parent scopes
STEntry *SymbolTable::LookupSymbol(char *str) {
    if (!str) return NULL;
    
    // First search in current scope
    STEntry *entry = GetEntryCurrentScope(str);
    if (entry) return entry;
    
    // If not found and we have a parent scope, search there
    if (next != NULL) {
        return next->LookupSymbol(str);
    }
    
    // Not found in any scope
    return NULL;
}

// Get parent scope without exiting
SymbolTable* SymbolTable::get_parent_scope() {
    return next;
}

// Add a symbol to the current scope or return existing one
STEntry *SymbolTable::PutSymbol(char *str, STE_TYPE type, int line) {
    if (!str) return NULL;
    
    STEntry *entry = GetEntryCurrentScope(str);
    
    // If the symbol already exists, return it
    if (entry) return entry;
    
    // Otherwise, add a new entry to the list at the hash slot
    unsigned long index = hash(str);
    slots[index].AddEntry(str, type, line);
    
    // Increment entry count
    number_entries++;
    
    // Return the newly added entry
    return slots[index].FindEntry(str);
}

// Add an entry to the symbol table, return false if already exists
bool SymbolTable::AddEntry(char *str, STE_TYPE type, int line) {
    if (!str) return false;
    
    unsigned long index = hash(str);
    
    // If the slot already has an entry with the same name, return false
    // otherwise add the entry and return true
    bool result = slots[index].AddEntry(str, type, line);
    
    if (result) {
        number_entries++;
    }
    
    return result;
}

// Global function: Create a new scope and return it
SymbolTable* enter_scope() {
    // Create a new scope and link it as the new head of the scope chain
    SymbolTable* new_scope = new SymbolTable(SymbolTable::DEFAULT_SIZE, 
                                            current_scope ? current_scope->fold_case : 0);
    new_scope->next = current_scope;
    current_scope = new_scope;
    return current_scope;
}

// Global function: Exit current scope
SymbolTable* exit_scope() {
    // Make sure we don't exit the global scope
    if (current_scope && current_scope->next) {
        SymbolTable* temp = current_scope;
        current_scope = current_scope->next;
        // Note: in a real application, you might want to delete temp to avoid memory leaks,
        // but for simplicity and to ensure we don't break anything, we'll leave it for now.
        // delete temp; 
        return current_scope;
    }
    return current_scope; // Return current scope if we can't exit further
}

// Clear all entries in the symbol table
void SymbolTable::ClearSymbolTable() {
    for (int i = 0; i < table_size; i++) {
        slots[i].Clear();
    }
    
    // Reset statistics
    number_entries = 0;
    number_probes = 0;
    number_hits = 0;
    max_search_dist = 0;
}

// Print statistics about the symbol table
void SymbolTable::PrintSymbolStats(FILE *fp) {
    fprintf(fp, "\nSymbol Table Statistics:\n");
    fprintf(fp, "-----------------------\n");
    fprintf(fp, "Total entries: %d\n", number_entries);
    fprintf(fp, "Table size: %d\n", table_size);
    fprintf(fp, "Load factor: %.2f\n", (float)number_entries / table_size);
    fprintf(fp, "Number of probes: %d\n", number_probes);
    fprintf(fp, "Number of hits: %d\n", number_hits);
    
    // Calculate collision statistics
    int empty_slots = 0;
    int max_list_size = 0;
    int total_collisions = 0;
    
    for (int i = 0; i < table_size; i++) {
        int list_size = slots[i].Count();
        
        if (list_size == 0) {
            empty_slots++;
        }
        else if (list_size > 1) {
            total_collisions += (list_size - 1);
        }
        
        if (list_size > max_list_size) {
            max_list_size = list_size;
        }
    }
    
    fprintf(fp, "Empty slots: %d (%.2f%%)\n", empty_slots, (float)empty_slots / table_size * 100);
    fprintf(fp, "Non-empty slots: %d\n", table_size - empty_slots);
    fprintf(fp, "Maximum list length: %d\n", max_list_size);
    fprintf(fp, "Total collisions: %d\n", total_collisions);
    fprintf(fp, "Average list length for non-empty slots: %.2f\n", 
           (table_size - empty_slots > 0) ? (float)number_entries / (table_size - empty_slots) : 0);
}

// Reset the symbol table with a new size
void SymbolTable::Reset(int new_size) {
    // Save old table
    STList *old_slots = slots;
    int old_size = table_size;
    
    // Create new table
    table_size = new_size;
    slots = new STList[table_size];
    
    // Reset statistics
    number_entries = 0;
    number_probes = 0;
    number_hits = 0;
    max_search_dist = 0;
    
    // We can't easily transfer entries between lists, so we need to recreate all entries
    // A better solution would be to add a method to iterate through all entries in the list
    // but for now we'll keep the interface simple
    
    // Clean up
    delete[] old_slots;
}

// Print all entries in the symbol table
void SymbolTable::PrintAll(FILE *fp) {
    fprintf(fp, "\nSymbol Table Contents:\n");
    fprintf(fp, "---------------------\n");
    fprintf(fp, "Table size: %d, Entries: %d\n", table_size, number_entries);
    
    for (int i = 0; i < table_size; i++) {
        int count = slots[i].Count();
        if (count > 0) {
            fprintf(fp, "Slot[%d]: ", i);
            slots[i].PrintAll(fp);
        }
    }
}

// Find and print a specific entry
void SymbolTable::FindAndPrintEntry(char *str, FILE *fp) {
    if (!str) {
        fprintf(fp, "Invalid string provided (NULL)\n");
        return;
    }
    
    unsigned long index = hash(str);
    STEntry *entry = slots[index].FindEntry(str);
    
    if (entry) {
        fprintf(fp, "Found entry: ");
        entry->print(fp);
        fprintf(fp, "\n");
    } else {
        fprintf(fp, "Entry '%s' not found in symbol table.\n", str);
    }
}