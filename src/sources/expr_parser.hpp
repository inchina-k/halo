#pragma once

#include <vector>
#include <vector>
#include <memory>

#include "token.hpp"
#include "expr.hpp"

namespace halo
{
    class ExprParser
    {
        const std::vector<Token> m_tokens;
        std::size_t m_curr;
        Expr *m_root;
        std::vector<std::unique_ptr<Expr>> m_nodes;

        Expr *expr();
        Expr *equality();
        Expr *comparison();
        Expr *term();
        Expr *factor();
        Expr *unary();
        Expr *primary();

        Expr *alloc_grouping(Expr *e);
        Expr *alloc_binary_expr(Token t, Expr *l, Expr *r);
        Expr *alloc_unary_expr(Token t, Expr *e);
        Expr *alloc_int_literal(Token t);
        Expr *alloc_float_literal(Token t);
        Expr *alloc_bool_literal(Token t);
        Expr *alloc_string_literal(Token t);

    public:
        ExprParser(const std::vector<Token> &tokens)
            : m_tokens(tokens), m_curr(0)
        {
        }

        Expr *parse();
    };
}