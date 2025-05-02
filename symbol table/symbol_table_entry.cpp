#include "symbol_table_entry.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


char *STE_TYPE_STR[TYPE_SIZE] = {"None","int","char*","float","double","char","boolean","function"};
char str[128];


/**
 * @brief STEntry::STEntry : Default constructor, initialize Next to NULL, Type to STE_NONE and Name to empty string
 */
STEntry::STEntry() {
    Next = NULL;
    Type = STE_NONE;
    Name[0] = 0; // empty String
    Dimensions = 0; // for arrays, 0 for non arrays
    Scope = 0; // for functions, 0 for non functions
    Size = 0; // size in bytes
    IsInitialized = false; // for variables, 0 for non initialized, 1 for initialized
    IsUsed = false; // for variables, 0 for unused, 1 for used
}

/**
 * @brief STEntry::STEntry : Constructor with name and type, initialize Next to NULL, Type to STE_NONE and Name to empty string
 * @param name : Name of the entry
 * @param type : Type of the entry
 */
STEntry::STEntry(const char* name, STE_TYPE type) {
    Next = NULL;
    Type = type;
    strcpy(Name, name);
    Dimensions = 0; // for arrays, 0 for non arrays
    Scope = 0; // for functions, 0 for non functions
    Size = 0; // size in bytes
    IsInitialized = false; // for variables, 0 for non initialized, 1 for initialized
    IsUsed = false; // for variables, 0 for unused, 1 for used
}

/**
 * @brief STEntry::toString : Convert entry to string representation
 * @return : String representation of the entry
 */
char* STEntry::toString() {
    if ((Type < STE_NONE) || Type > STE_CHAR) Type = STE_NONE;
    sprintf(str, "(%s,%s,dim:%d,scope:%d,size:%d,init:%s,used:%s)", 
            Name, 
            STE_TYPE_STR[Type],
            Dimensions,
            Scope,
            Size,
            IsInitialized ? "true" : "false",
            IsUsed ? "true" : "false");
    return str;
}

/**
 * @brief STEntry::print : Print entry to file
 * @param fp : File pointer to print to
 */
void STEntry::print(FILE* fp) {
    fprintf(fp, "%s ", toString());
}
/**
 * @brief STEntry::getType : Get type from string
 * @param str : String representation of the type
 * @return : STE_TYPE corresponding to the string
 */
STE_TYPE STEntry::getType(const char* str) {
    int i;
    for (i = 0; i < TYPE_SIZE; i++) {
        if (strcmp(STE_TYPE_STR[i], str) == 0) return ((STE_TYPE)i);
    }
    return STE_NONE;
}
/**
 * @brief STEntry::getTypeString : Get string representation of type
 * @param type : STE_TYPE to get string representation for
 * @return : String representation of the type
 */
const char* STEntry::getTypeString(STE_TYPE type) {
    if ((type < STE_NONE) || type > STE_CHAR) type = STE_NONE;
    return STE_TYPE_STR[type];
}
/**
 * @brief STEntry::getTypeSize : Get size of type in bytes
 * @param type : STE_TYPE to get size for
 * @return : Size of the type in bytes
 */
int STEntry::getTypeSize(STE_TYPE type) {
    switch (type) {
        case STE_INT: return sizeof(int);
        case STE_STRING: return sizeof(char*);
        case STE_FLOAT: return sizeof(float);
        case STE_DOUBLE: return sizeof(double);
        case STE_CHAR: return sizeof(char);
        case STE_BOOLEAN: return sizeof(bool);
        default: return 0; // Unknown type
    }
}
/**
 * @brief STEntry::getDimensions : Get number of dimensions for arrays
 * @param type : STE_TYPE to get dimensions for
 * @return : Number of dimensions for the type
 */
int STEntry::getDimensions(){

    return Dimensions;
}

/**
 * @brief STEntry::getScope : Get scope level for functions
 * @param type : STE_TYPE to get scope for
 * @return : Scope level for the type
 */
int STEntry::getScope() {
    return Scope;
}   

/**
 * @brief STEntry::isInitialized : Check if variable is initialized
 * @return : true if initialized, false otherwise
 */
bool STEntry::isInitialized() {
    return IsInitialized;
}
/**
 * @brief STEntry::isUsed : Check if variable is used
 * @return : true if used, false otherwise
 */
bool STEntry::isUsed() {
    return IsUsed;
}
/**
 * @brief STEntry::setInitialized : Set variable as initialized
 * @param initialized : true if initialized, false otherwise
 */
void STEntry::setInitialized(bool initialized) {
    IsInitialized = initialized;
}
/**
 * @brief STEntry::setUsed : Set variable as used
 * @param used : true if used, false otherwise
 */
void STEntry::setUsed(bool used) {
    IsUsed = used;
}
/**
 * @brief STEntry::setDimensions : Set number of dimensions for arrays
 * @param dimensions : Number of dimensions for the array
 */
void STEntry::setDimensions(int dimensions) {
    Dimensions = dimensions;
}
/**
 * @brief STEntry::setScope : Set scope level for functions
 * @param scope : Scope level for the function
 */
void STEntry::setScope(int scope) {
    Scope = scope;
}
/**
 * @brief STEntry::setSize : Set size in bytes
 * @param size : Size in bytes
 */
void STEntry::setSize(int size) {
    Size = size;
}


