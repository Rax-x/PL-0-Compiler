#ifndef _PARSER_HPP_
#define _PARSER_HPP_

#include <vector>

#include "token.hpp"

namespace pl0::parser {

using namespace token;

class Parser final {
public:
    Parser() = default;

    
    

private:

    std::vector<Token> m_tokens;
    std::uint32_t m_curr;

    bool m_panicMode = false;
};


}

#endif
