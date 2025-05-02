#include "FileDescriptor.h"

// Constructor for opening a specific file
FileDescriptor::FileDescriptor(const char *FileName) {
    line_number = 1;
    char_number = 0;
    flag = UNSET;
    flag2 = UNSET;
    buf_size = BUFFER_SIZE;
    buffer = new char[buf_size];
    buffer[0] = '\0';

    if (FileName == nullptr) {
        fp = stdin;
        file = nullptr;
    } else {
        fp = fopen(FileName, "r");
        if (fp == nullptr) {
            std::cerr << "Error: Could not open file " << FileName << std::endl;
            file = nullptr;
        } else {
            file = new char[strlen(FileName) + 1];
            strcpy(file, FileName);
        }
    }
}

// Default constructor - opens stdin
FileDescriptor::FileDescriptor() {
    fp = stdin;
    file = nullptr;
    line_number = 1;
    char_number = 0;
    flag = UNSET;
    flag2 = UNSET;
    buf_size = BUFFER_SIZE;
    buffer = new char[buf_size];
    buffer[0] = '\0';
}

// Destructor to clean up resources
FileDescriptor::~FileDescriptor() {
    Close();
    if (file != nullptr) {
        delete[] file;
        file = nullptr;
    }
    if (buffer != nullptr) {
        delete[] buffer;
        buffer = nullptr;
    }
}

// Returns the name of the file
char* FileDescriptor::GetFileName() {
    return file;
}

// Check if file is open without errors
bool FileDescriptor::IsOpen() {
    return (fp != nullptr && !ferror(fp));
}

// Returns a pointer to the current line buffer
char* FileDescriptor::GetCurrLine() {
    if (fp == nullptr || feof(fp)) {
        return nullptr;
    }
    return buffer;
}

// Returns the current line number
int FileDescriptor::GetLineNum() {
    return line_number;
}

// Returns the current character number
int FileDescriptor::GetCharNum() {
    return char_number;
}

// Closes the file descriptor
void FileDescriptor::Close() {
    if (fp != nullptr && fp != stdin) {
        fclose(fp);
        fp = nullptr;
    }
}

// Gets the current character from the file
char FileDescriptor::GetChar() {
    // If there's a previous unget, handle it
    if (flag == SET) {
        flag = UNSET;
        char_number++;
        return buffer[char_number - 1];
    }

    // Check if we need to read a new line
    if (buffer[char_number] == '\0') {
        // Reached end of line, read next line
        if (fp == nullptr || feof(fp)) {
            return EOF;
        }

        char *result = nullptr;
        int current_size = buf_size;

        // Try reading a line, doubling buffer size if needed
        while (true) {
            result = fgets(buffer, current_size, fp);

            // Handle EOF or error
            if (result == nullptr) {
                if (feof(fp)) {
                    buffer[0] = '\0';
                    return EOF;
                }
                ReportError((char*)"Error reading from file");
                return EOF;
            }

            // Check if we got the entire line
            size_t len = strlen(buffer);
            if (len > 0 && len < current_size - 1) {
                break; // Got the whole line
            }

            // If buffer isn't big enough, double it and try again
            current_size *= 2;
            char *new_buffer = new char[current_size];
            strcpy(new_buffer, buffer);
            delete[] buffer;
            buffer = new_buffer;
            buf_size = current_size;

            // Continue reading from where we left off
            if (fseek(fp, -len, SEEK_CUR) != 0) {
                ReportError((char*)"Error repositioning in file");
                return EOF;
            }
        }

        line_number++;
        char_number = 0;
    }

    // Return the current character and move to the next
    return buffer[char_number++];
}

// Reports an error with line and character information
void FileDescriptor::ReportError(char *msg) {
    cout << msg << " on line: " << line_number << '\n';
    cout << buffer ;//<< '\n';

    // Print spaces or tabs until the caret position
    for (int i = 0; i < char_number - 1; i++)
    {
        cout << (buffer[i] == '\t' ? '\t' : ' ');
    }

    // Print the caret symbol '^' under the current character
    cout << "^\n";
}

// Puts back one character - can't do consecutive ungets
void FileDescriptor::UngetChar(char c) {
    if (flag == SET) {
        ReportError((char*)"Multiple UngetChar calls are not allowed");
        return;
    }

    if (char_number <= 0) {
        ReportError((char*)"Cannot UngetChar at beginning of line");
        return;
    }

    char_number--;
    buffer[char_number] = c;
    flag = SET;
}