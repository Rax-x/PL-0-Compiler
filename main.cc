#include <cstdlib>
#include <fstream>
#include <iostream>
#include <ostream>

#include "token.hpp"
#include "tokenizer.hpp"

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
using pl0::token::TokenType;

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

    for(const auto& token : tokens){
        const auto& [type, lexeme, line] = token;

        if(type == TokenType::UnexpectedCharacter){
            std::cout << "Unexpected character: '" << lexeme << "' at line " << line << '\n';
            break;
        }

        std::cout << "Token: '" << lexeme << "' Ln: " << line << '\n';
    }

    return 0;
}
