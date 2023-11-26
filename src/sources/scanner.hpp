#pragma once

#include <string>
#include <vector>

#include "token.hpp"

namespace halo
{
    class Scanner
    {
        const std::string &m_data;

        size_t m_start;
        size_t m_curr;
        size_t m_offset;
        size_t m_line;

        std::vector<Token> m_tokens;

    public:
        Scanner(const std::string &data) : m_data(data), m_start(0), m_curr(0), m_offset(0), m_line(1) {}

        std::vector<Token> scan();

    private:
        char peek() const;
        char peek_next() const;

        void read_one_symbol_lexeme(TokenType tt);
        void read_two_symbol_lexeme(TokenType tt1, TokenType tt2);
        void read_comment();
        void read_quote();
        void read_number();
        void read_identifier();
    };
}