#include "scanner.hpp"
#include <cctype>
#include <unordered_map>

using namespace std;
using namespace halo;

std::vector<Token> Scanner::scan()
{
    while (m_start != m_data.size())
    {
        switch (m_data[m_start])
        {
        case ' ':
            ++m_start;
            ++m_curr;
            ++m_offset;
            break;
        case '\n':
        case '\r':
            ++m_start;
            ++m_curr;
            if (m_data[m_curr - 1] == '\n')
            {
                ++m_line;
            }
            m_offset = 0;
            break;
        case '.':
            read_one_symbol_lexeme(TokenType::Dot);
            break;
        case ',':
            read_one_symbol_lexeme(TokenType::Comma);
            break;
        case ':':
            read_one_symbol_lexeme(TokenType::Colon);
            break;
        case ';':
            read_one_symbol_lexeme(TokenType::Semicolon);
            break;
        case '(':
            read_one_symbol_lexeme(TokenType::OpenPar);
            break;
        case ')':
            read_one_symbol_lexeme(TokenType::ClosePar);
            break;
        case '[':
            read_one_symbol_lexeme(TokenType::OpenBracket);
            break;
        case ']':
            read_one_symbol_lexeme(TokenType::CloseBracket);
            break;
        case '+':
            read_two_symbol_lexeme(TokenType::Plus, TokenType::PlusEqual);
            break;
        case '-':
            read_two_symbol_lexeme(TokenType::Minus, TokenType::MinusEqual);
            break;
        case '*':
            read_two_symbol_lexeme(TokenType::Mul, TokenType::MulEqual);
            break;
        case '/':
            read_two_symbol_lexeme(TokenType::Div, TokenType::DivEqual);
            break;
        case '%':
            read_two_symbol_lexeme(TokenType::Mod, TokenType::ModEqual);
            break;
        case '<':
            read_two_symbol_lexeme(TokenType::Less, TokenType::LessEqual);
            break;
        case '>':
            read_two_symbol_lexeme(TokenType::Greater, TokenType::GreaterEqual);
            break;
        case '=':
            read_two_symbol_lexeme(TokenType::Equal, TokenType::EqualEqual);
            break;
        case '!':
            read_two_symbol_lexeme(TokenType::Bad, TokenType::BangEqual);
            break;
        case '#':
            read_comment();
            break;
        case '"':
            read_quote();
            break;
        default:
            if (isdigit(peek()))
            {
                read_number();
            }
            else if (isalpha(peek()) || peek() == '_')
            {
                read_identifier();
            }
            else
            {
                m_tokens.emplace_back(TokenType::Bad, std::string(1, m_data[m_start]), m_line, m_offset);
                ++m_start;
                ++m_curr;
                ++m_offset;
            }
            break;
        }
    }

    m_tokens.emplace_back(TokenType::Eof, "eof", m_line, m_offset);

    return m_tokens;
}

char Scanner::peek() const
{
    if (m_curr < m_data.size())
    {
        return m_data[m_curr];
    }

    return 0;
}

char Scanner::peek_next() const
{
    if (m_curr + 1 < m_data.size())
    {
        return m_data[m_curr + 1];
    }

    return 0;
}

void Scanner::read_one_symbol_lexeme(TokenType tt)
{
    m_tokens.emplace_back(tt, m_data.substr(m_start, 1), m_line, m_offset);

    ++m_start;
    ++m_curr;
    ++m_offset;
}

void Scanner::read_two_symbol_lexeme(TokenType tt1, TokenType tt2)
{
    TokenType tt = (peek_next() == '=') ? tt2 : tt1;

    if (tt == tt1)
    {
        m_tokens.emplace_back(tt1, m_data.substr(m_start, 1), m_line, m_offset);

        ++m_start;
        ++m_curr;
        ++m_offset;
    }
    else
    {
        m_tokens.emplace_back(tt2, m_data.substr(m_start, 2), m_line, m_offset);

        m_start += 2;
        m_curr += 2;
        m_offset += 2;
    }
}

void Scanner::read_comment()
{
    while (m_start < m_data.size() && peek() != '\n')
    {
        ++m_start;
        ++m_curr;
        ++m_offset;
    }

    if (peek() == '\n')
    {
        ++m_start;
        ++m_curr;
        ++m_line;

        m_offset = 0;
    }
}

void Scanner::read_quote()
{
    do
    {
        ++m_curr;
        ++m_offset;

    } while (m_curr < m_data.size() && peek() != '"' && peek() != '\n');

    if (m_curr == m_data.size() || peek() == '\n')
    {
        m_tokens.emplace_back(TokenType::Bad, m_data.substr(m_start, m_curr - m_start), m_line, m_offset);

        if (peek() == '\n')
        {
            ++m_curr;
            ++m_line;
            m_offset = 0;
        }

        m_start = m_curr;
        return;
    }

    m_tokens.emplace_back(TokenType::StrLiteral, m_data.substr(m_start + 1, m_curr - m_start - 1), m_line, m_offset);

    ++m_curr;
    ++m_offset;
    m_start = m_curr;
}

void Scanner::read_number()
{
    int new_offset = m_offset;
    ++m_curr;
    ++new_offset;
    bool is_float = false;

    while (m_curr < m_data.size() && isdigit(peek()))
    {
        ++m_curr;
        ++new_offset;
    }

    if (m_curr < m_data.size() && peek() == '.')
    {
        is_float = true;

        if (isdigit(peek_next()))
        {
            ++m_curr;
            ++new_offset;

            while (m_curr < m_data.size() && isdigit(peek()))
            {
                ++m_curr;
                ++new_offset;
            }
        }
        else
        {
            m_tokens.emplace_back(TokenType::Bad, m_data.substr(m_start, m_curr - m_start + 1), m_line, m_offset);
            ++m_curr;
            m_start = m_curr;
            return;
        }
    }

    string t = m_data.substr(m_start, m_curr - m_start);

    m_tokens.emplace_back(is_float ? TokenType::FloatLiteral : TokenType::IntLiteral, t, m_line, m_offset);
    m_start = m_curr;
    m_offset = new_offset;
}

namespace
{
    unordered_map<string, TokenType> keywords = {
        {"not", TokenType::Not},
        {"and", TokenType::And},
        {"or", TokenType::Or},
        {"var", TokenType::Var},
        {"let", TokenType::Let},
        {"true", TokenType::True},
        {"false", TokenType::False},
        {"null", TokenType::Null},
        {"if", TokenType::If},
        {"elif", TokenType::Elif},
        {"else", TokenType::Else},
        {"for", TokenType::For},
        {"while", TokenType::While},
        {"end", TokenType::End},
        {"break", TokenType::Break},
        {"continue", TokenType::Continue},
        {"throw", TokenType::Throw},
        {"try", TokenType::Try},
        {"catch", TokenType::Catch},
        {"finally", TokenType::Finally},
        {"fun", TokenType::Fun},
        {"return", TokenType::Return},
        {"class", TokenType::Class},
        {"lambda", TokenType::Lambda},
        {"import", TokenType::Import},
    };
}

void Scanner::read_identifier()
{
    ++m_curr;
    ++m_offset;

    while (m_curr < m_data.size() && (isalnum(peek()) || peek() == '_'))
    {
        ++m_curr;
        ++m_offset;
    }

    string t = m_data.substr(m_start, m_curr - m_start);

    auto p = keywords.find(t);

    if (p == keywords.end())
    {
        m_tokens.emplace_back(TokenType::Identifier, t, m_line, m_offset);
    }
    else
    {
        m_tokens.emplace_back(p->second, t, m_line, m_offset);
    }

    m_start = m_curr;
}
