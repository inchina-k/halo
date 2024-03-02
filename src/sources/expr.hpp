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
        virtual ~Expr() {}

        virtual Object *visit(ExprVisitor *v) = 0;
    };

    struct Grouping : Expr
    {
        Expr *expr;

        Grouping(Expr *e)
            : expr(e)
        {
        }

        Object *visit(ExprVisitor *v) override;
    };

    struct BinaryExpr : Expr
    {
        Token m_token;
        Expr *m_left;
        Expr *m_right;

        BinaryExpr(Token t, Expr *l, Expr *r)
            : m_token(t), m_left(l), m_right(r)
        {
        }

        Object *visit(ExprVisitor *v) override;
    };

    struct LogicalExpr : Expr
    {
        Token m_token;
        Expr *m_left;
        Expr *m_right;

        LogicalExpr(Token t, Expr *l, Expr *r)
            : m_token(t), m_left(l), m_right(r)
        {
        }

        Object *visit(ExprVisitor *v) override;
    };

    struct UnaryExpr : Expr
    {
        Token m_token;
        Expr *m_expr;

        UnaryExpr(Token t, Expr *e)
            : m_token(t), m_expr(e)
        {
        }

        Object *visit(ExprVisitor *v) override;
    };

    struct Call : Expr
    {
        Expr *m_expr;
        std::vector<Expr *> m_args;

        Call(Expr *e, const std::vector<Expr *> &a)
            : m_expr(e), m_args(a)
        {
        }

        Object *visit(ExprVisitor *v) override;
    };

    struct Literal : Expr
    {
        Token m_token;
        Object *m_val;

        Literal(Token t)
            : m_token(t), m_val(nullptr)
        {
        }

        Object *visit(ExprVisitor *v) override;
    };

    struct Var : Expr
    {
        Token m_token;

        Var(Token t)
            : m_token(t)
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

        Lambda(const std::vector<Token> &capture, const std::vector<Token> &params, std::vector<std::unique_ptr<Stmt>> body);

        Object *visit(ExprVisitor *v) override;
    };

    struct ExprVisitor
    {
        virtual Object *visit_grouping(Grouping *e) = 0;
        virtual Object *visit_binary_expr(BinaryExpr *e) = 0;
        virtual Object *visit_logical_expr(LogicalExpr *e) = 0;
        virtual Object *visit_unary_expr(UnaryExpr *e) = 0;
        virtual Object *visit_call_expr(Call *e) = 0;
        virtual Object *visit_literal(Literal *e) = 0;
        virtual Object *visit_var(Var *e) = 0;
        virtual Object *visit_lambda(Lambda *e) = 0;
    };
}