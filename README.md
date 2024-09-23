# PL/0 Compiler

This is a compiler for the programming language PL/0 introduced by [Niklaus Wirth](https://en.wikipedia.org/wiki/Niklaus_Wirth) in the book, 
[Algorithms + Data Structures = Programs](https://en.wikipedia.org/wiki/Algorithms_%2B_Data_Structures_%3D_Programs), as an example of how to construct a compiler.
This language is a minimal Pascal, has a really simple grammar and supports all basic features of a programming language, like: constants, variables, procedures, control flow and so on.
Because of its simplicity, I consider it a good starting point to learn how build compilers with [LLVM](https://llvm.org/) as backend.
For more details about this language check out the Wikipedia [article](https://en.wikipedia.org/wiki/PL/0).

> [!IMPORTANT]
> The focus of this project is not to design a good and fast compiler, but just to learn how use the LLVM to develop compilers backends.
> So all the components of this compiler (lexer, parsers, error reporting, etc.) are really basic.

### Example

```pascal

const MAX = 100;
var start;

procedure countFromStartToMax;
begin
  if start < MAX then
    while start < MAX do
      start := start + 1
end;

begin
  ?start;
  call countFromStartToMax;
  !start
end.
```

> [!WARNING]
> Only global variables are initialized to 0.

**How generate the executable?**

> [!NOTE]
> This compiler generate an object file called `program.o`.

First of all generate the object file with `./pl0 myprogram.pl0` and after link it with `clang` or `gcc`.
```bash
gcc program.o -o myprogram
```

## TODO

- [ ] Code refactor (the codegen code is a little bit unredable).
- [ ] Generate the object file with the same name of the input file.
- [ ] Invoke the `clang` or `gcc` internally.

# Resources

- [LLVM Kaleidoscope](https://llvm.org/docs/tutorial/)
- [Clang Source code](https://github.com/llvm/llvm-project/tree/main/clang/)
- [LLVM Language Reference](https://llvm.org/docs/LangRef.html)



