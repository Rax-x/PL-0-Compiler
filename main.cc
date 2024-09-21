#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <ostream>

#include "ast.hpp"
#include "tokenizer.hpp"
#include "parser.hpp"
#include "codegen.hpp"

/*

# means not equal.
! prints a value.
? gets a value from inputs.

program = block "." ;

block = [ "const" ident "=" number {"," ident "=" number} ";"]
        [ "var" ident {"," ident} ";"]
        { "procedure" ident ";" block ";" } statement ;
statement = [ ident ":=" expression 
              | "call" ident
              | "?" ident | "!" expression
              | "begin" statement {";" statement } "end"
              | "if" condition "then" statement
              | "while" condition "do" statement ];

condition = "odd" expression |
            expression ("="|"#"|"<"|"<="|">"|">=") expression ;

expression = [ "+"|"-"] term { ("+"|"-") term};
term = factor {("*"|"/") factor};
factor = ident | number | "(" expression ")";

*/

using pl0::tokenizer::Tokenizer;
using pl0::parser::Parser;
using pl0::ast::AstPrinter;
using pl0::codegen::CodeGenerator;

static auto readFile(const char* path) -> std::string {
    std::ifstream stream(path);

    const std::size_t size = stream.seekg(0, std::ios::end).tellg();
    stream.seekg(0, std::ios::beg);

    std::string buffer(size, '\0');
    stream.read(buffer.data(), size);

    return buffer;
}


auto main(int argc, char** argv) -> int {

    if(argc < 2){
        std::cerr << "Usage: " << argv[0] << " <path>" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto source = readFile(argv[1]);
    Tokenizer tokenizer = Tokenizer(source);
    
    auto tokens = tokenizer.tokenize();

    Parser parser(tokens);
    auto ast = parser.parseProgram();

    if(parser.hadError()) {
        for(const auto& error : parser.errors()){
            std::cout << error << '\n';
        }
    } else {
        AstPrinter printer;
        printer.print(ast);

        CodeGenerator codegen;
        codegen.dump(ast);
    }

    return 0;
}
