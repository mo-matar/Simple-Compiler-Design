//testing the symbol table
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include "symbol.h"
#include "symbol_table_entry.h"
#include "list.h"

/**
 * @brief getIdentifier: reads the next Identifier, and its type from the file
 * @param fp
 * @param str    : Identifier is stored here
 * @param type   : stores the type read
 * @return
 */
int getIdentifier(FILE* fp, char *str, STE_TYPE& type)
{
    char s1[64],s2[64],s3[64];
    int n = fscanf(fp,"%s%s%s", s1,s2,s3);

    //printf ("(n= %d,%s,%s,%s)\n", n, s1, s2, s3);

    strcpy(str,s2);
    type = STEntry::getType(s1);
   // printf ("-(%d,%s)\n",(int)type, str); to make sure it is reading correctly
    return n;
}

/**
 * @brief getRandChar Returns a random char
 * @param isFirst: isFirst= true character returned is a random char in 'A' --- 'Z'
 *                 else returns a 'A' ---'Z' or '0' -- '9'
 * @return
 */
char getRandChar(bool isFirst=false)
{
    int index = 0;
    char ch;
    if (isFirst) index = rand()%26;
    else index = rand()%(26+10); // 10 for 0--9
    if(index < 26) ch = 'A' + index;
    else ch = '0' + (index - 26);
    return ch;
}

/**
 * @brief getRandIdentifier: generates a random 8-char variable name and random attributes
 * @param str: Stores the generated string
 * @param type: Type of variable generated, random from the types defined in STEntry.h
 */
void getRandIdentifier(char *str, STE_TYPE& type)
{
    int i;
    str[0] = getRandChar(true);
    for (i = 1; i < 8; i++)
    {
        str[i] = getRandChar();
    }
    str[i]=0;
    type = (STE_TYPE)(rand()%TYPE_SIZE);
}

int main(int argc, char *argv[])
{
    FILE *fp;
    SymbolTable ST;

    STE_TYPE type;
    char str[64];
    fp = fopen("t.txt","r");
    if(!fp) {
        return 1;
    }
    FILE *fout = fopen("out.txt","w");   //set fout = stdout for screen output
    if(!fout) {
        printf("Error: Could not open output file out.txt\n");
        fclose(fp);
        return 1;
    }

    FILE *statsFile = fopen("table_stats.txt", "w");
    if (!statsFile) {
        printf("Error: Could not open stats file table_stats.txt\n");
        fclose(fp);
        fclose(fout);
        return 1;
    }

    fprintf(fout, "\n**(Test 1)**: Reading from the file t.txt and creating a symbol table with size = 19(default)\n\n");
    int n;
    bool Added;
    while (!feof(fp))
    {
        n = getIdentifier(fp,str,type);
        if(n == 3)
        {
           Added = ST.AddEntry(str,type);
           if(!Added) 
           {
               fprintf(fout,"Entry %s Already Exists\n",str);
           }
           else
           {
               // Set additional parameters for some entries to demonstrate the extended output
               STEntry *entry = ST.GetSymbol(str);
               if (entry) {
                   // Set sizes based on type
                   entry->setSize(STEntry::getTypeSize(entry->Type));
                   
                   // Add some sample data for specific entries
                   if (strcmp(str, "variable1") == 0) {
                       entry->setInitialized(true);
                       entry->setUsed(true);
                   }
                   else if (strcmp(str, "x12") == 0) {
                       entry->setDimensions(2);  // 2D array
                       entry->setSize(entry->Size * 10);  //size for 10 elements
                   }
                   else if (strcmp(str, "counter") == 0) {
                       entry->setInitialized(true);
                   }
                   else if (strcmp(str, "PI") == 0) {
                       entry->setInitialized(true);
                       entry->setUsed(true);
                   }
                   else if (strcmp(str, "grade") == 0) {
                       entry->setScope(1);  //in a function scope
                   }
                   else if (strcmp(str, "name") == 0) {
                       entry->setScope(0);  //in the global scope
                   }
                   
               }
           }
        }
    }
    fclose(fp);

    fprintf(fout,"\n==============\n");
    ST.PrintAll(fout);
    fprintf(fout,"\n==============\n");

    fprintf(fout, "\n\n**(Test 2)**: Searching for some Variables in the Symbol Table Created\n\n");
    ST.FindAndPrintEntry("variable1111", fout);
    ST.FindAndPrintEntry("jklm", fout);
    ST.FindAndPrintEntry("x12", fout);
    ST.FindAndPrintEntry("xx12", fout);

    fprintf(fout, "\n\n**(Test 3)**: Creating 8 Symbol Tables with different Slot Size and Different Sizes\n");
    fprintf(fout, "            : Random Variable Names are generated\n");
    srand(time(0));

    int k, i;
    int Sizes[10]         = {29, 29, 17, 23, 37, 31, 101, 101, 101, 101};
    int ElementsCount[10] = {100, 100, 100, 100, 100, 150, 300, 100, 200, 50};

    SymbolTable ST2(Sizes[0], 0); //initialize table with 29 elements and fold_case=0
    
    for(k = 0; k < 10; k++)
    {
        if(k != 0) ST2.Reset(Sizes[k]);
        for(i=0; i < ElementsCount[k]; i++) // create different sized tables
        {
            getRandIdentifier(str, type);
            Added = ST2.AddEntry(str, type);
            if(!Added) {
                fprintf(fout,"Entry %s Already Exists\n", str);
            } else {
                //set random attributes for the new entry
                STEntry *entry = ST2.GetSymbol(str);
                if (entry) {
                    //set size based on type
                    entry->setSize(STEntry::getTypeSize(entry->Type));
                    
                    // random dimension (0-3, where 0 means not an array)
                    int dim = rand() % 4;
                    entry->setDimensions(dim);
                    
                    // random scope (0-2, where 0 is global)
                    int scope = rand() % 3;
                    entry->setScope(scope);
                    
                    //random initialization
                    entry->setInitialized(rand() % 2 == 0);
                    
                    //random usage (30% chance)
                    entry->setUsed(rand() % 10 < 3);
                }
            }
        }
        fprintf(fout,"\n===== Table (%d) =======\n", k);
        ST2.PrintSymbolStats(statsFile); // Print stats to table_stats.txt
        ST2.PrintAll(fout); // Print all entries with detailed parameters
    }
    
    fclose(statsFile);
    fprintf(fout,"\n------------------done-------------------\n");
    fclose(fout);
    
    printf("Symbol table tests completed. Results written to out.txt\n");
    return 0;
}