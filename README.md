# Simple-Compiler-Design

The first few phases of designing a compiler (lexical analyser, symbol table, and parser)

## Language Overview

This compiler project implements a Pascal/Ada-inspired language with structured programming features. The language includes strong typing, control structures, and modular programming concepts.

## Sample Code

```
## This is a sample program demonstrating the language syntax ##

program SampleProgram

## Variable declarations ##
var
    x, y: integer;
    pi: constant float := 3.14;
    name: string;
    isValid: bool;

## Function declaration ##
function calculateArea(radius: float): float
begin
    return pi * radius * radius;
end

## Procedure declaration ##
procedure printInfo(message: string)
begin
    write(message);
end

## Main program ##
begin
    ## Variable initialization ##
    x := 10;
    y := 20;
    isValid := true;
    name := "Sample Program";

    ## Conditional statement ##
    if x < y then
        write("y is greater than x");
    else
        write("x is greater than or equal to y");
    fi

    ## Loop examples ##
    ## While loop ##
    while x > 0 do
        write(x);
        x := x - 1;
    od

    ## For loop ##
    for i from 1 to 10 by 1 do
        y := y + i;
    od

    ## Function call with result ##
    write("Area of circle with radius 5: ");
    write(calculateArea(5.0));

    ## Procedure call ##
    printInfo("Program completed successfully!");

    ## Boolean operations ##
    isValid := (x = 0) and (y > 15) or not false;

    ## Read input ##
    write("Enter your name: ");
    read(name);
    write("Hello, ");
    write(name);
end
```

## Language Features

1. **Data Types**

   - `integer`: Whole numbers
   - `float`: Floating-point numbers
   - `string`: Text strings enclosed in double quotes
   - `bool`: Boolean values (`true` or `false`)

2. **Control Structures**

   - `if-then-else-fi`: Conditional execution
   - `while-do-od`: Pre-test loop
   - `for-from-to-by-do-od`: Counted loop with step size

3. **Functions and Procedures**

   - Functions return values
   - Procedures perform actions without returning values

4. **Input/Output**

   - `read`: Read input from user
   - `write`: Display output

5. **Comments**

   - Comments are enclosed between `##` pairs

6. **Operators**
   - Assignment: `:=`
   - Arithmetic: `+`, `-`, `*`, `/`
   - Comparison: `=`, `!=`, `<`, `<=`, `>`, `>=`
   - Logical: `and`, `or`, `not`

---

# MORE LATER!!
