# N23 Compiler Design Project

## Introduction

N23 is a high-level programming language designed with syntax elements inspired by Ada and Pascal. This project implements a compiler for the N23 language with a focus on modular design and comprehensive error handling. The compiler is structured into several components: scanner, symbol table, parser, and abstract syntax tree (AST) generation. This implementation follows a top-down recursive descent parsing approach to process N23 programs.

## Language Overview

N23 supports standard programming constructs including:

- Variable and constant declarations
- Functions and procedures with parameters
- Control structures (if-then-else, while loops, for loops)
- Basic I/O operations (read, write)
- Scope management with begin-end blocks
- Data types: integer, boolean, and string
- Expressions with proper operator precedence and associativity

### Example N23 Program

```pascal
program

  var x : integer;
  var y : integer;
  constant MAX = 100;

  function factorial(n : integer) : integer
  begin
    var result : integer;
    program
var num : integer;
var result : boolean;

function isEven(x : integer) : boolean
begin
    var remainder : integer;

    remainder := x / 2 * 2;
    return(remainder = x);
end;

begin
    var msg : string;
    read(num);
    result := isEven(num);

    if result then
    begin
        msg := "number is even";
        write(num);
    end
    else
    begin
        msg := "number is odd";
        write(num);
    end
    fi;
end;
```

## Grammar and LL(1) Conversion

The N23 language is defined by a context-free grammar, initially presented in a form that is not LL(1). Converting this grammar to an LL(1) form was necessary to enable efficient recursive descent parsing.

### Major Transformations for LL(1) Conversion:

1. **Left Recursion Elimination**: Expressions with left recursion (like `expr = expr rel_conj expr2 | expr2`) were rewritten in a right-recursive form with tail recursion (`expr = expr2 expr_tail; expr_tail = rel_conj expr2 expr_tail | λ`).

2. **Left Factoring**: Productions with common prefixes were rewritten to share the common parts.

3. **Operator Precedence Enforcement**: The grammar was restructured to enforce proper operator precedence (from highest to lowest):
   - Unary operators (`not`, `-`)
   - Multiplicative operators (`*`, `/`)
   - Additive operators (`+`, `-`)
   - Relational operators (`=`, `!=`, `<`, `<=`, `>`, `>=`)   - Logical operators (`and`, `or`)

4. **Statement Simplification**: Complex statements were decomposed into simpler forms with tail rules.

The conversion ensures that the grammar can be parsed without backtracking, optimizing the parsing process while maintaining the language's intended semantics.

The usual [Markdown Cheatsheet](https://github.com/adam-p/markdown-here/wiki/Markdown-Cheatsheet)
does not cover some of the more advanced Markdown tricks, but here
is one. You can combine verbatim HTML with your Markdown. 
This is particularly useful for tables.
Notice that with **empty separating lines** we can use Markdown inside HTML:

<table>
<tr>
<th>Regular Grammar</th>
<th>LL(1) Grammar</th>
</tr>
<tr>
<td>

```ebnf
program = decl_list ;

decl_list = decl ";" decl_list | "λ" ;

decl = "var" id ":" type 
     | "constant" id "=" expr 
     | "function" id formal_list ":" type block 
     | "procedure" id formal_list block ;

type = "integer" | "boolean" | "string" ;

formal_list = "()" | "(" formals ")" ;


stmt = id ":=" expr
     | "if" expr "then" stmt "fi"
     | "if" expr "then" stmt "else" stmt "fi"
     | "while" expr "do" stmt "od"
     | "for" id ":=" expr "to" expr "do" stmt "od"
     | "read" "(" id ")"
     | "write" "(" id ")"
     | id arg_list
     | "return" "(" expr ")"
     | block ;

block = "begin" var_decl_list stmt_list "end" ;

var_decl_list = var_decl ";" var_decl_list | " λ" ;

var_decl = "var" id ":" type ;

stmt_list = stmt ";" stmt_list | " λ" ;

arg_list = "()" | "(" args ")" ;

args = expr | expr "," args ;

expr = id
     | id arg_list
     | integer_constant
     | string_constant
     | expr arith_op expr
     | "true"
     | "false"
     | expr rel_op expr
     | expr rel_conj expr
     | unary_op "(" expr ")"
     | "(" expr ")" ;

arith_op = "*" | "/" | "+" | "-" ;

rel_op = "=" | "!=" | "<" | "<=" | ">" | ">=" ;

rel_conj = "and" | "or" ;

unary_op = "-" | "not" ;
```

</td>
<td>

```ebnf
program = decl_list ;

decl_list = decl ";" decl_list | "λ" ;

decl = "var" id ":" type 
     | "constant" id "=" expr 
     | "function" id formal_list ":" type block 
     | "procedure" id formal_list block ;
     | block ; # added for main block

type = "integer" | "boolean" | "string" ;

formal_list = "()" | "(" formals ")" ;

formals = id ":" type formals_tail ;
formals_tail = "," id ":" type formals_tail | "λ" ;



stmt_list = stmt ";" stmt_list | "λ" ;

stmt = id stmt_id_tail
     | "if" expr "then" stmt if_tail
     | "while" expr "do" stmt "od"
     | "for" id ":=" expr "to" expr "do" stmt "od"
     | "read" "(" id ")"
     | "write" "(" id ")"
     | "return" "(" expr ")"
     | block ;

stmt_id_tail = ":=" expr | arg_list ;

if_tail = "fi" | "else" stmt "fi" ;

block = "begin" var_decl_list stmt_list "end" ;

var_decl_list = var_decl ";" var_decl_list | "λ" ;

var_decl = "var" id ":" type ;


arg_list = "()" | "(" args ")" ;

args = expr args_tail ;
args_tail = "," expr args_tail | "λ" ;



rel_op = "=" | "!=" | "<" | "<=" | ">" | ">=" ;

rel_conj = "and" | "or" ;

unary_op = "-" | "not" ;

add_sub_op = "+" | "-" ;
mult_div_op = "*" | "/" ;

primary_expr_tail = arg_list | "λ" ;

expr = expr2 expr_tail ;
expr_tail = matarrel_conj expr2 expr_tail | "λ" ;

expr2 = expr3 expr2_tail ;
expr2_tail = rel_op expr3 expr2_tail | "λ" ;

expr3 = expr4 expr3_tail ;
expr3_tail = add_sub_op expr4 expr3_tail | "λ" ;

expr4 = expr5 expr4_tail ;
expr4_tail = mult_div_op expr5 expr4_tail | "λ" ;

expr5 = unary_op "(" expr ")" | primary_expr ;

primary_expr = id primary_expr_tail
             | integer_constant
             | string_constant
             | "true"
             | "false"
             | "(" expr ")" ;
```

</td>
</tr>
</table>

## Scanner and File Descriptor

The scanner is responsible for lexical analysis, converting the source code from a sequence of characters into a stream of tokens.

### File Descriptor

The `FileDescriptor` class handles file operations and maintains file state:

- Tracks the current position in the file (line number, character position)
- Provides methods for reading characters and reporting errors
- Manages file opening, closing, and buffering

### Scanner Implementation

The `Scanner` class identifies and categorizes tokens from the source code:

- Recognizes language keywords, identifiers, literals, operators, and separators
- Skips whitespace and comments
- Detects and reports lexical errors
- Provides methods for token lookahead and consumption

The scanner implements a state machine approach that transitions based on the current character and context. It identifies various token types including:

- **Keywords**: `program`, `var`, `constant`, `function`, `procedure`, `if`, `then`, `else`, etc.
- **Literals**: Identifiers, integers, strings
- **Operators**: Arithmetic, relational, and logical
- **Separators**: Parentheses, semicolons, etc.

## Symbol Table

The symbol table maintains information about all identifiers in the program and supports scope management similar to the C language.

### Key Features

- **Hash Table Implementation**: Efficient symbol lookup using a hash function
- **Scope Management**: Multiple scopes with proper nesting
- **Symbol Information**: Stores name, type, value, and scope level for each symbol
- **Error Detection**: Helps identify redeclarations and undefined references

### Scope Handling

N23 uses lexical scoping with the following rules:

- The outermost scope corresponds to global variables defined outside any routine
- Each procedure, function parameter list, and begin-end block introduces a new scope
- Variable references use the variable from the closest enclosing scope
- Multiple declarations in a scope are detected as errors

### Symbol Table Entry Types

Each symbol table entry stores:

- **Name**: The identifier string
- **Type**: For variables (integer, boolean, string)
- **Value**: For constants
- **Parameter Information**: For functions and procedures
- **Return Type**: For functions
- **Scope Level**: Indicating nesting depth

## Parser and AST Generation

The parser is the core component of the compiler, responsible for syntax analysis and AST construction.

### Recursive Descent Parser

The parser uses a recursive descent approach based on the LL(1) grammar:

- Each non-terminal in the grammar has a corresponding parse method
- The parser reads tokens from the scanner and matches them against expected patterns
- Productions with alternatives use lookahead to determine the correct rule to apply
- Upon successful parsing, the parser builds corresponding AST nodes
- Error detection and recovery mechanisms are implemented throughout

### Abstract Syntax Tree (AST)

The AST represents the program structure in a hierarchical form:

- **Node Types**: Correspond to different language constructs (declarations, statements, expressions)
- **Children**: Represent sub-expressions, operands, or nested statements
- **Symbol Table Integration**: AST nodes reference symbol table entries for identifiers
- **Tree Traversal**: Enables structured processing for later compiler phases

### Error Management

The parser handles various errors:

- **Syntax Errors**: Detection and reporting of invalid token sequences
- **Error Messages**: Descriptive messages with line numbers and error contexts

### AST Printing and Evaluation

The implementation includes utilities for:

- **AST Printing**: Converting the AST back to a readable program representation (unparsing)
- **Constant Expression Evaluation**: Evaluating constant expressions at compile time
- **Type Checking**: Basic type compatibility verification

## Testing and Validation

The project includes several test cases that demonstrate different aspects of the language:

1. **Test1_isEven**: A function to check if a number is even
2. **Test2_redeclaration**: Tests error detection for redeclared variables
3. **Test3_outer_scope**: Demonstrates variable resolution in outer scopes
4. **Test4_inner_scope**: Tests local variable declarations and scope nesting
5. **Test5_all_operators**: Validates the implementation of all operators and precedence rules

## Error Handling

The compiler implements robust error handling:

- **Lexical Errors**: Detected by the scanner (e.g., invalid characters, malformed tokens)
- **Syntax Errors**: Identified by the parser (e.g., missing semicolons, unbalanced blocks)
- **Error Reporting**: Includes line numbers and descriptive messages
- **Error Output**: Errors are logged to a separate file for reference

## Implementation Notes

1. **Modularity**: The compiler is designed with separate components that can be developed and tested independently.
2. **Error Recovery**: The system attempts to continue parsing after errors to identify multiple issues in a single pass.
3. **Memory Management**: Care is taken to properly allocate and deallocate resources for AST nodes and symbol table entries.
4. **Testing Infrastructure**: Includes utilities for validating the correctness of each compiler phase.
5. **Extensibility**: The design allows for future extensions such as semantic analysis and code generation.
