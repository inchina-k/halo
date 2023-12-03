#pragma once

#include "token.hpp"
#include "halo_object.hpp"
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

    struct IntLiteral : Expr
    {
        Token m_token;
        long long m_val;

        IntLiteral(Token t)
            : m_token(t), m_val(std::stoll(t.m_lexeme))
        {
        }

        Object *visit(ExprVisitor *v) override;
    };

    struct FloatLiteral : Expr
    {
        Token m_token;
        double m_val;

        FloatLiteral(Token t)
            : m_token(t), m_val(std::stod(t.m_lexeme))
        {
        }

        Object *visit(ExprVisitor *v) override;
    };

    struct BoolLiteral : Expr
    {
        Token m_token;
        bool m_val;

        BoolLiteral(Token t)
            : m_token(t), m_val(t.m_type == TokenType::True)
        {
        }

        Object *visit(ExprVisitor *v) override;
    };

    struct StringLiteral : Expr
    {
        Token m_token;
        std::string m_val;

        StringLiteral(Token t)
            : m_token(t), m_val(t.m_lexeme)
        {
        }

        Object *visit(ExprVisitor *v) override;
    };

    struct NullLiteral : Expr
    {
        Token m_token;

        NullLiteral(Token t)
            : m_token(t)
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
        virtual Object *visit_int_literal(IntLiteral *e) = 0;
        virtual Object *visit_float_literal(FloatLiteral *e) = 0;
        virtual Object *visit_bool_literal(BoolLiteral *e) = 0;
        virtual Object *visit_string_literal(StringLiteral *e) = 0;
        virtual Object *visit_null_literal(NullLiteral *e) = 0;
    };

    struct ExprPrinter : ExprVisitor
    {
        std::ostringstream m_data;

        Object *visit_grouping(Grouping *e) override
        {
            m_data << "(";
            e->expr->visit(this);
            m_data << ")";

            return nullptr;
        }

        Object *visit_binary_expr(BinaryExpr *e) override
        {
            m_data << "(";
            e->m_left->visit(this);
            m_data << e->m_token.m_lexeme;
            e->m_right->visit(this);
            m_data << ")";

            return nullptr;
        }

        Object *visit_logical_expr(LogicalExpr *e) override
        {
            m_data << "(";
            e->m_left->visit(this);
            m_data << e->m_token.m_lexeme;
            e->m_right->visit(this);
            m_data << ")";

            return nullptr;
        }

        Object *visit_unary_expr(UnaryExpr *e) override
        {
            m_data << "(";
            m_data << e->m_token.m_lexeme;
            e->m_expr->visit(this);
            m_data << ")";

            return nullptr;
        }

        Object *visit_int_literal(IntLiteral *e) override
        {
            m_data << e->m_token.m_lexeme;

            return nullptr;
        }

        Object *visit_float_literal(FloatLiteral *e) override
        {
            m_data << e->m_token.m_lexeme;

            return nullptr;
        }

        Object *visit_bool_literal(BoolLiteral *e) override
        {
            m_data << e->m_token.m_lexeme;

            return nullptr;
        }

        Object *visit_string_literal(StringLiteral *e) override
        {
            m_data << e->m_token.m_lexeme;

            return nullptr;
        }

        Object *visit_null_literal(NullLiteral *e) override
        {
            m_data << e->m_token.m_lexeme;

            return nullptr;
        }
    };
}