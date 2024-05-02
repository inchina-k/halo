#pragma once

#include <vector>
#include <memory>
#include <unordered_map>

#include "token.hpp"
#include "expr.hpp"
#include "stmt.hpp"

namespace halo
{
    class Parser
    {
        enum class Scopes
        {
            If,
            While,
            For,
            Fun,
            Init,
            Lambda,
            Class
        };

        const std::vector<Token> m_tokens;
        std::size_t m_curr;
        std::vector<std::unique_ptr<Stmt>> m_stmts;
        std::vector<std::unique_ptr<Expr>> m_nodes;
        bool m_had_errors;
        std::vector<Scopes> m_scopes;

        Stmt *statement(size_t line);
        Stmt *var_statement(size_t line);
        Stmt *assignment_statement(size_t line);
        Stmt *expression_statement(size_t line);
        Stmt *if_statement(size_t line);
        Stmt *while_statement(size_t line);
        Stmt *for_statement(size_t line);
        Stmt *break_statement(size_t line);
        Stmt *continue_statement(size_t line);
        Stmt *fun_statement(size_t line);
        Stmt *return_statement(size_t line);
        Stmt *class_statement(size_t line);

        Expr *expr();
        Expr *or_expr();
        Expr *and_expr();
        Expr *equality();
        Expr *comparison();
        Expr *term();
        Expr *factor();
        Expr *unary();
        Expr *call();
        Expr *primary();
        Expr *lambda();
        Expr *list();

        // Expr *alloc_grouping(Expr *e);
        Expr *alloc_binary_expr(Token t, Expr *l, Expr *r);
        Expr *alloc_logical_expr(Token t, Expr *l, Expr *r);
        Expr *alloc_unary_expr(Token t, Expr *e);
        Expr *alloc_call_expr(Expr *e, const std::vector<Expr *> &p, size_t line);
        Expr *alloc_dot_expr(Expr *e, Token t);
        Expr *alloc_subscript_expr(Expr *e, Expr *index, size_t line);
        Expr *alloc_literal(Token t);
        Expr *alloc_var(Token t);
        Expr *alloc_lambda(const std::vector<Token> &capture, const std::vector<Token> &params, std::vector<std::unique_ptr<Stmt>> body, size_t line);
        Expr *alloc_list(const std::vector<Expr *> &params, size_t line);

        bool match(TokenType t);
        const Token &consume(TokenType t, std::string err);
        const Token &peek() const;
        const Token &advance();
        bool is_in_loop() const;
        bool is_in_callable() const;
        bool is_constructor() const;
        bool is_fun_allowed() const;
        bool is_lambda_allowed() const;
        bool is_class_allowed() const;

    public:
        Parser(const std::vector<Token> &tokens)
            : m_tokens(tokens), m_curr(0), m_had_errors(false)
        {
        }

        bool had_errors() const
        {
            return m_had_errors;
        }

        Expr *parse_expr();
        void parse();

        const std::vector<std::unique_ptr<Stmt>> &statements() const
        {
            return m_stmts;
        }
    };
}