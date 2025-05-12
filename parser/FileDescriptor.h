#ifndef FILEDESCRIPTOR_H
#define FILEDESCRIPTOR_H

#define SET 1
#define UNSET 0
#define BUFFER_SIZE 256

#include <stdio.h>
#include <iostream>
#include <cstring>
#include <cstdlib>
using namespace std;

class FileDescriptor {
public:
    FILE *fp;
    int line_number;    // line number in the file
    int char_number;    // character number in the line
    int flag;           // to prevent two ungets in a row
    int buf_size;       // stores the buffer size
    char *buffer;       // buffer to store a line
    char *file;         // file name, allocate memory for this
    int flag2;          // additional flag

    // Constructor for opening a specific file
    FileDescriptor(const char *FileName);

    // Default constructor - opens stdin
    FileDescriptor();

    // Destructor to clean up resources
    ~FileDescriptor();

    // Returns the name of the file
    char* GetFileName();

    // Check if file is open without errors
    bool IsOpen();

    // Returns a pointer to the current line buffer
    char* GetCurrLine();

    // Returns the current line number
    int GetLineNum();

    // Returns the current character number
    int GetCharNum();

    // Closes the file descriptor
    void Close();

    // Gets the current character from the file
    char GetChar();

    // Reports an error with line and character information
    void ReportError(char *msg);

    // Puts back one character - can't do consecutive ungets
    void UngetChar(char c);
};

#endif // FILEDESCRIPTOR_H