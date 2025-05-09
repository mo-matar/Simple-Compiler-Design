# Simple-Compiler-Design

The first few phases of designing a compiler (lexical analyser, symbol table, and parser)

## Language Overview

This compiler project implements a Pascal/Ada-inspired language with structured programming features. The language includes strong typing, control structures, and modular programming concepts.

## Sample Code

```pascal
program;

  var x : integer;
  var y : integer;
  constant MAX = 100;
  
  function factorial(n : integer) : integer
  begin
    var result : integer;
    result := 1;
    for i := 1 to n do
      result := result * i
    od;
    return(result)
  end;
  
  procedure printNumber(num : integer)
  begin
    write(num)
  end;
  
  function isEven(n : integer) : boolean
  begin
    return(n / 2 * 2 = n)
  end;
  
  begin
    read(x);
    if x > MAX then
      x := MAX
    fi;
    
    y := factorial(x);
    
    if isEven(y) then
      write(y)
    else
      printNumber(y);
      write(x)
    fi
  end;
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
