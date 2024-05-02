#pragma once

#include "token.hpp"
#include "object.hpp"
#include <sstream>
#include <vector>
#include <memory>

namespace halo
{
    class ExprVisitor;

    struct Expr
    {
        size_t m_line;

        Expr(size_t line = 0)
            : m_line(line)
        {
        }

        virtual ~Expr()
        {
        }

        virtual Object *visit(ExprVisitor *v) = 0;
    };

    struct Grouping : Expr
    {
        Expr *expr;

        Grouping(Expr *e, size_t line)
            : Expr(line), expr(e)
        {
        }

        Object *visit(ExprVisitor *v) override;
    };

    struct BinaryExpr : Expr
    {
        Token m_token;
        Expr *m_left;
        Expr *m_right;

        BinaryExpr(Token t, Expr *l, Expr *r, size_t line)
            : Expr(line), m_token(t), m_left(l), m_right(r)
        {
        }

        Object *visit(ExprVisitor *v) override;
    };

    struct LogicalExpr : Expr
    {
        Token m_token;
        Expr *m_left;
        Expr *m_right;

        LogicalExpr(Token t, Expr *l, Expr *r, size_t line)
            : Expr(line), m_token(t), m_left(l), m_right(r)
        {
        }

        Object *visit(ExprVisitor *v) override;
    };

    struct UnaryExpr : Expr
    {
        Token m_token;
        Expr *m_expr;

        UnaryExpr(Token t, Expr *e, size_t line)
            : Expr(line), m_token(t), m_expr(e)
        {
        }

        Object *visit(ExprVisitor *v) override;
    };

    struct Call : Expr
    {
        Expr *m_expr;
        std::vector<Expr *> m_args;

        Call(Expr *e, const std::vector<Expr *> &a, size_t line)
            : Expr(line), m_expr(e), m_args(a)
        {
        }

        Object *visit(ExprVisitor *v) override;
    };

    struct Dot : Expr
    {
        Expr *m_expr;
        Token m_name;

        Dot(Expr *e, Token t, size_t line)
            : Expr(line), m_expr(e), m_name(t)
        {
        }

        Object *visit(ExprVisitor *v) override;
    };

    struct Subscript : Expr
    {
        Expr *m_expr;
        Expr *m_index;

        Subscript(Expr *e, Expr *index, size_t line)
            : Expr(line), m_expr(e), m_index(index)
        {
        }

        Object *visit(ExprVisitor *v) override;
    };

    struct Literal : Expr
    {
        Token m_token;
        Object *m_val;

        Literal(Token t, size_t line)
            : Expr(line), m_token(t), m_val(nullptr)
        {
        }

        Object *visit(ExprVisitor *v) override;
    };

    struct Var : Expr
    {
        Token m_token;

        Var(Token t, size_t line)
            : Expr(line), m_token(t)
        {
        }

        Object *visit(ExprVisitor *v) override;
    };

    struct Stmt;

    struct Lambda : Expr
    {
        std::vector<Token> m_capture;
        std::vector<Token> m_params;
        std::vector<std::unique_ptr<Stmt>> m_body;

        Lambda(const std::vector<Token> &capture, const std::vector<Token> &params, std::vector<std::unique_ptr<Stmt>> body, size_t line);

        Object *visit(ExprVisitor *v) override;
    };

    struct ListExpr : Expr
    {
        std::vector<Expr *> m_params;

        ListExpr(const std::vector<Expr *> &params, size_t line)
            : Expr(line), m_params(params)
        {
        }

        Object *visit(ExprVisitor *v) override;
    };

    struct ExprVisitor
    {
        virtual Object *visit_grouping(Grouping *e) = 0;
        virtual Object *visit_binary_expr(BinaryExpr *e) = 0;
        virtual Object *visit_logical_expr(LogicalExpr *e) = 0;
        virtual Object *visit_unary_expr(UnaryExpr *e) = 0;
        virtual Object *visit_call_expr(Call *e) = 0;
        virtual Object *visit_dot_expr(Dot *e) = 0;
        virtual Object *visit_subscript_expr(Subscript *e) = 0;
        virtual Object *visit_literal(Literal *e) = 0;
        virtual Object *visit_var(Var *e) = 0;
        virtual Object *visit_lambda(Lambda *e) = 0;
        virtual Object *visit_list(ListExpr *e) = 0;
    };
}