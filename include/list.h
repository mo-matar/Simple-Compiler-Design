#ifndef LIST_H
#define LIST_H
#include "symbol_table_entry.h"

class STList
{
private:
    int Counter;  // counts the Node n the List
    STEntry *Head; // Should be Initialized to Null in the Constructor
public:

    STList();
    STEntry *FindEntry(char *name); // return NULL if Not found
    bool AddEntry(char *name, STE_TYPE type, int line);//Adds an entry if the Node Does Not exist
    void PrintAll(FILE *fp);
    int Count();
    void Clear();
    ~STList();
};

#endif // STLIST_H