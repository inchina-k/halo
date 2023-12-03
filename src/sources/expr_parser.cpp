#include "expr_parser.hpp"

using namespace std;
using namespace halo;

Expr *ExprParser::alloc_grouping(Expr *e)
{
    m_nodes.push_back(make_unique<Grouping>(e));
    return m_nodes.back().get();
}

Expr *ExprParser::alloc_binary_expr(Token t, Expr *l, Expr *r)
{
    m_nodes.push_back(make_unique<BinaryExpr>(t, l, r));
    return m_nodes.back().get();
}

Expr *ExprParser::alloc_logical_expr(Token t, Expr *l, Expr *r)
{
    m_nodes.push_back(make_unique<LogicalExpr>(t, l, r));
    return m_nodes.back().get();
}

Expr *ExprParser::alloc_unary_expr(Token t, Expr *e)
{
    m_nodes.push_back(make_unique<UnaryExpr>(t, e));
    return m_nodes.back().get();
}

Expr *ExprParser::alloc_int_literal(Token t)
{
    m_nodes.push_back(make_unique<IntLiteral>(t));
    return m_nodes.back().get();
}

Expr *ExprParser::alloc_float_literal(Token t)
{
    m_nodes.push_back(make_unique<FloatLiteral>(t));
    return m_nodes.back().get();
}

Expr *ExprParser::alloc_bool_literal(Token t)
{
    m_nodes.push_back(make_unique<BoolLiteral>(t));
    return m_nodes.back().get();
}

Expr *ExprParser::alloc_string_literal(Token t)
{
    m_nodes.push_back(make_unique<StringLiteral>(t));
    return m_nodes.back().get();
}

Expr *ExprParser::alloc_null_literal(Token t)
{
    m_nodes.push_back(make_unique<NullLiteral>(t));
    return m_nodes.back().get();
}

Expr *ExprParser::parse()
{
    return m_root = expr();
}

Expr *ExprParser::expr()
{
    return or_expr();
}

Expr *ExprParser::or_expr()
{
    Expr *left = and_expr();

    while (m_tokens[m_curr].m_type == TokenType::Or)
    {
        Token op = m_tokens[m_curr];
        ++m_curr;
        Expr *right = and_expr();
        left = alloc_logical_expr(op, left, right);
    }

    return left;
}

Expr *ExprParser::and_expr()
{
    Expr *left = equality();

    while (m_tokens[m_curr].m_type == TokenType::And)
    {
        Token op = m_tokens[m_curr];
        ++m_curr;
        Expr *right = equality();
        left = alloc_logical_expr(op, left, right);
    }

    return left;
}

Expr *ExprParser::equality()
{
    Expr *left = comparison();

    while (m_tokens[m_curr].m_type == TokenType::EqualEqual ||
           m_tokens[m_curr].m_type == TokenType::BangEqual)
    {
        Token op = m_tokens[m_curr];
        ++m_curr;
        Expr *right = comparison();
        left = alloc_binary_expr(op, left, right);
    }

    return left;
}

Expr *ExprParser::comparison()
{
    Expr *left = term();

    while (m_tokens[m_curr].m_type == TokenType::Greater ||
           m_tokens[m_curr].m_type == TokenType::GreaterEqual ||
           m_tokens[m_curr].m_type == TokenType::Less ||
           m_tokens[m_curr].m_type == TokenType::LessEqual)
    {
        Token op = m_tokens[m_curr];
        ++m_curr;
        Expr *right = term();
        left = alloc_binary_expr(op, left, right);
    }

    return left;
}

Expr *ExprParser::term()
{
    Expr *left = factor();

    while (m_tokens[m_curr].m_type == TokenType::Plus || m_tokens[m_curr].m_type == TokenType::Minus)
    {
        Token op = m_tokens[m_curr];
        ++m_curr;
        Expr *right = factor();
        left = alloc_binary_expr(op, left, right);
    }

    return left;
}

Expr *ExprParser::factor()
{
    Expr *left = unary();

    while (m_tokens[m_curr].m_type == TokenType::Mul || m_tokens[m_curr].m_type == TokenType::Div || m_tokens[m_curr].m_type == TokenType::Mod)
    {
        Token op = m_tokens[m_curr];
        ++m_curr;
        Expr *right = unary();
        left = alloc_binary_expr(op, left, right);
    }

    return left;
}

Expr *ExprParser::unary()
{
    if (m_tokens[m_curr].m_type == TokenType::Minus ||
        m_tokens[m_curr].m_type == TokenType::Not)
    {
        Token op = m_tokens[m_curr];
        ++m_curr;
        Expr *right = unary();
        return alloc_unary_expr(op, right);
    }

    return primary();
}

Expr *ExprParser::primary()
{
    if (m_tokens[m_curr].m_type == TokenType::IntLiteral)
    {
        return alloc_int_literal(m_tokens[m_curr++]);
    }
    else if (m_tokens[m_curr].m_type == TokenType::FloatLiteral)
    {
        return alloc_float_literal(m_tokens[m_curr++]);
    }
    else if (m_tokens[m_curr].m_type == TokenType::True || m_tokens[m_curr].m_type == TokenType::False)
    {
        return alloc_bool_literal(m_tokens[m_curr++]);
    }
    else if (m_tokens[m_curr].m_type == TokenType::StrLiteral)
    {
        return alloc_string_literal(m_tokens[m_curr++]);
    }
    else if (m_tokens[m_curr].m_type == TokenType::Null)
    {
        return alloc_null_literal(m_tokens[m_curr++]);
    }
    else if (m_tokens[m_curr].m_type == TokenType::OpenPar)
    {
        ++m_curr;
        Expr *e = expr();

        if (m_tokens[m_curr].m_type == TokenType::ClosePar)
        {
            ++m_curr;
            return e;
        }

        throw runtime_error("Missing ')' for '('");
    }

    throw runtime_error("What am I, Why am I???");
}
