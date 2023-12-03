#pragma once

#include "expr.hpp"

namespace halo
{
    struct Printer : ExprVisitor
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

        Object *visit_literal(Literal *e) override
        {
            m_data << e->m_token.m_lexeme;

            return nullptr;
        }
    };
}
