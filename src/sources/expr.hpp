#pragma once

#include "token.hpp"
#include "object.hpp"
#include <sstream>

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

    struct Literal : Expr
    {
        Token m_token;
        Object *m_val;

        Literal(Token t);

        Object *visit(ExprVisitor *v) override;
    };

    struct ExprVisitor
    {
        virtual Object *visit_grouping(Grouping *e) = 0;
        virtual Object *visit_binary_expr(BinaryExpr *e) = 0;
        virtual Object *visit_logical_expr(LogicalExpr *e) = 0;
        virtual Object *visit_unary_expr(UnaryExpr *e) = 0;
        virtual Object *visit_literal(Literal *e) = 0;
    };
}