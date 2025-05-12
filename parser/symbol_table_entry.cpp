#include "symbol_table_entry.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


char *STE_TYPE_STR[TYPE_SIZE] = {"None","int","char*","float","char","boolean","function"};
char str[128];


/**
 * @brief STEntry::STEntry : Default constructor, initialize Next to NULL, Type to STE_NONE and Name to empty string
 */
STEntry::STEntry() {
    Next = NULL;
    Type = STE_NONE;
    Name[0] = 0; // empty String
    Size = 0; // size in bytes
    Line = 0;
    
    // Initialize new fields
    ConstValue = 0;
    VarType = type_none;
    ResultType = type_none;
    Formals = NULL;
    IsConstant = 0;
}

/**
 * @brief STEntry::STEntry : Constructor with name and type
 * @param name : Name of the entry
 * @param type : Type of the entry
 */
STEntry::STEntry(const char* name, STE_TYPE type, int line) {
    Next = NULL;
    Type = type;
    strcpy(Name, name);
    Size = getTypeSize(type);
    Line = line;

    
    // Initialize new fields
    ConstValue = 0;
    VarType = steTypeToJType(type);
    ResultType = type_none;
    Formals = NULL;
    IsConstant = 0;
}

/**
 * @brief STEntry::toString : Convert entry to string representation
 * @return : String representation of the entry
 */
char* STEntry::toString() {
    if ((Type < STE_NONE) || Type > STE_ROUTINE) Type = STE_NONE;
    
    const char* varTypeName = "none";
    if (VarType == type_integer) varTypeName = "integer";
    else if (VarType == type_float) varTypeName = "float";
    else if (VarType == type_boolean) varTypeName = "boolean";
    else if (VarType == type_string) varTypeName = "string";
    
    if (IsConstant) {
        sprintf(str, "(%s,%s,const:%d)", Name, STE_TYPE_STR[Type], ConstValue);
    } else if (Type == STE_ROUTINE) {
        sprintf(str, "(%s,function,return:%s)", Name, varTypeName);
    } else {
        sprintf(str, "(%s,%s,type:%s,size:%d)", Name, STE_TYPE_STR[Type], varTypeName, Size);
    }
    
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
        case STE_CHAR: return sizeof(char);
        case STE_BOOLEAN: return sizeof(bool);
        default: return 0; // Unknown type
    }
}

/**
 * @brief STEntry::steTypeToJType : Convert STE_TYPE to j_type
 * @param type : STE_TYPE to convert
 * @return : Corresponding j_type
 */
j_type STEntry::steTypeToJType(STE_TYPE type) {
    switch (type) {
        case STE_INT: return type_integer;
        case STE_STRING: return type_string;
        case STE_FLOAT: return type_float;
        case STE_BOOLEAN: return type_boolean;
        default: return type_none;
    }
}

/**
 * @brief STEntry::jTypeToSteType : Convert j_type to STE_TYPE
 * @param type : j_type to convert
 * @return : Corresponding STE_TYPE
 */
STE_TYPE STEntry::jTypeToSteType(j_type type) {
    switch (type) {
        case type_integer: return STE_INT;
        case type_string: return STE_STRING;
        case type_float: return STE_FLOAT;
        case type_boolean: return STE_BOOLEAN;
        default: return STE_NONE;
    }
}



