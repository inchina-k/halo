#pragma once

#include <ostream>
#include <string>

#include "token_type.hpp"

namespace halo
{
    struct Token
    {
        const TokenType m_type;
        const std::string m_lexeme;
        const size_t m_line;
        const size_t m_offset;

        Token(TokenType type, const std::string &lexeme, size_t line, size_t offset)
            : m_type(type), m_lexeme(lexeme), m_line(line), m_offset(offset)
        {
        }
    };
}