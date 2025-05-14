#include "../include/list.h"

/**
 * @brief STList::STList
 * default constructor, initialize head and Counter
 */
STList::STList()
{
    Counter = 0;
    Head = NULL;
}
/**
 * @brief STList::FindEntry: search (linear search) the list and compare the name to the ones in the list
 * @param name : Given name to find
 * @return : If name is found found return NULL otherwise reaturn a pointer to the Node
 */

STEntry* STList::FindEntry(char *name)
{
    STEntry *ste = Head;
    while (ste != NULL)
    {
        if( strcmp(name,ste->Name) == 0) return ste;
        ste = ste->Next;
    }
    return NULL;

}
/**
 * @brief STList::AddEntry : Call FindEntry, if name is alread in table return false, otherwise add it to the list
 *                           Add it as the first Entry, like a stack which is fastest. Update Counter and Head
 * @param name : Name of Entry (variable)
 * @param type : Type of variable
 * @return : True if the node is added and False if the Entry Already exists in the Table
 */
bool STList::AddEntry(char *name, STE_TYPE type, int line)
{
  STEntry *ste = FindEntry(name);
  bool added = false;
  if(ste)
  {
      printf("Entry Already exist, nothing Added\n");
  }
  else
  {
      ste = new STEntry(name, type, line) ;
      ste->Next = Head;
      Head = ste;
      added = true;

      Counter++;
  }
  return added;
}

/**
 * @brief STList::PrintAll : Prints All nodes in the list, use the print in the STEntry.
 * @param fp : File Pointer where to write
 */
void STList::PrintAll(FILE *fp)
{
    STEntry *ste = Head;
    while (ste != NULL)
    {
        ste->print(fp);
        ste = ste->Next;
    }
}
/**
 * @brief STList::Count : return the number of Nodes in the List
 * @return : Number of Nodes in the List
 */
int STList::Count()
{
    return Counter;
}

/**
 * @brief STList::Clear : Delete All Elements, use a loop and call the Table[i].clear. Then Delete the Array Table
 */
void STList::Clear()
{
    STEntry *ste = Head;
    while (ste != NULL)
    {
        STEntry *temp = ste;
        ste = ste->Next;
        delete temp;
    }
    Head = NULL;
    Counter = 0;
}
/**
 * @brief STList::~STList : Destructor, call clear to delete all elements
 */
STList::~STList()
{
    Clear();
}
