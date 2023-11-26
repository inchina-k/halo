#pragma once

#include "expr.hpp"
#include "halo_object.hpp"
#include "gc.hpp"

namespace halo
{
    class Interpreter : public ExprVisitor
    {
        Object *m_res;
        GC m_gc;

    public:
        Interpreter()
            : m_res(nullptr)
        {
        }

        void interpret(Expr *e);
        Object *evaluate(Expr *e);

        Object *visit_grouping(Grouping *e) override;
        Object *visit_binary_expr(BinaryExpr *e) override;
        Object *visit_unary_expr(UnaryExpr *e) override;
        Object *visit_int_literal(IntLiteral *e) override;
        Object *visit_float_literal(FloatLiteral *e) override;
        Object *visit_bool_literal(BoolLiteral *e) override;
        Object *visit_string_literal(StringLiteral *e) override;
    };
}