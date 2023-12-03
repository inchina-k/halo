#pragma once

#include <vector>
#include <vector>
#include <memory>

#include "token.hpp"
#include "expr.hpp"

namespace halo
{
    class Parser
    {
        const std::vector<Token> m_tokens;
        std::size_t m_curr;
        Expr *m_root;
        std::vector<std::unique_ptr<Expr>> m_nodes;

        Expr *expr();
        Expr *or_expr();
        Expr *and_expr();
        Expr *equality();
        Expr *comparison();
        Expr *term();
        Expr *factor();
        Expr *unary();
        Expr *primary();

        Expr *alloc_grouping(Expr *e);
        Expr *alloc_binary_expr(Token t, Expr *l, Expr *r);
        Expr *alloc_logical_expr(Token t, Expr *l, Expr *r);
        Expr *alloc_unary_expr(Token t, Expr *e);
        Expr *alloc_literal(Token t);

    public:
        Parser(const std::vector<Token> &tokens)
            : m_tokens(tokens), m_curr(0)
        {
        }

        Expr *parse();
    };
}