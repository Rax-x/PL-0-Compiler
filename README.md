# 🔨 PL/0 Compiler

This is a compiler for the programming language PL/0 introduced by [Niklaus Wirth](https://en.wikipedia.org/wiki/Niklaus_Wirth) in the book, 
[Algorithms + Data Structures = Programs](https://en.wikipedia.org/wiki/Algorithms_%2B_Data_Structures_%3D_Programs), as an example of how to construct a compiler.
This language is a minimal Pascal, has a really simple grammar and supports all basic features of a programming language, like: constants, variables, procedures, control flow and so on.
Because of its simplicity, I consider it a good starting point to learn how build compilers with [LLVM](https://llvm.org/) as backend.
For more details about this language check out the Wikipedia [article](https://en.wikipedia.org/wiki/PL/0).

> [!IMPORTANT]
> The focus of this project is not to design a good and fast compiler, but just to learn how use the LLVM to develop compilers.
> So all the components of this compiler (lexer, parser, error reporting, etc.) are really basic.

## 👩‍💻 Installation
### 📜 Prerequisites

- `g++` or `clang++` compiler
- LLVM
- `make` 

### ⚒️ How to build the compiler?

In your terminal run the following command to build the compiler:
```bash
make
```

## 🧪 Example

Let's take the following PL/0 program:
```pascal
const ADD = 0, SUB = 1, MULT = 2, DIV = 3;
var firstOperand, secondOperand, operator, done, zeroDivisionError; 

procedure printErrorCode;
const ERROR = 2147483647;
begin
   !(-ERROR) - 1
end;

procedure setZeroDivisionError;
begin
   zeroDivisionError := 1
end;

procedure resetZeroDivisionError;
begin
   zeroDivisionError := 0
end;

procedure add;
begin
   !(firstOperand + secondOperand)
end;

procedure sub;
begin
   !(firstOperand - secondOperand)
end;

procedure mult;
begin
   !(firstOperand * secondOperand)
end;

procedure div;
begin
   if secondOperand # 0 then
      !(firstOperand / secondOperand);

   if secondOperand = 0 then
      call setZeroDivisionError
end;

begin
   while done = 0 do
   begin
      ?operator;

      if operator < 0 then
         operator := -operator;

      if operator > DIV then
         done := 1;

      if operator <= DIV then
      begin

         ?firstOperand;
         ?secondOperand;

         if operator = ADD then
            call add;

         if operator = SUB then
            call sub;

         if operator = MULT then
            call mult;

         if operator = DIV then
         begin
            call div;
            if zeroDivisionError = 1 then
            begin
               call printErrorCode;
               call resetZeroDivisionError
            end
         end

      end
   end
end.
```

To generate the executable  we run:

```bash
./pl0 myprogram.pl0
```

And when the compilation is finished we execute the executable produced:

```bash
./myprogram
```

# 🔭 Resources

- [LLVM Kaleidoscope](https://llvm.org/docs/tutorial/)
- [Clang Source Code](https://github.com/llvm/llvm-project/tree/main/clang/)
- [LLVM Language Reference](https://llvm.org/docs/LangRef.html)
