#pragma once

#include "expr.hpp"
#include "object.hpp"
#include "gc.hpp"

#include <functional>

namespace halo
{
    class Interpreter : public ExprVisitor
    {
        GC m_gc;

        template <typename OpType, ObjectType ObType, typename ResType = OpType, typename Op>
        Object *bin_op(Object *left, Object *right, Op op)
        {
            if (OpType *p_left = dynamic_cast<OpType *>(left))
            {
                if (OpType *p_right = dynamic_cast<OpType *>(right))
                {
                    Object *r = m_gc.new_object(ObType);
                    static_cast<ResType *>(r)->m_val = op(p_left->m_val, p_right->m_val);
                    return r;
                }
            }

            return nullptr;
        }

        template <typename OpType1, typename OpType2, ObjectType ObType, typename ResType = OpType1, typename Op>
        Object *bin_op_conv(Object *left, Object *right, Op op)
        {
            if (OpType1 *p_left = dynamic_cast<OpType1 *>(left))
            {
                if (OpType2 *p_right = dynamic_cast<OpType2 *>(right))
                {
                    Object *r = m_gc.new_object(ObType);
                    static_cast<ResType *>(r)->m_val = op(p_left->m_val, p_right->m_val);
                    return r;
                }
            }
            if (OpType2 *p_left = dynamic_cast<OpType2 *>(left))
            {
                if (OpType1 *p_right = dynamic_cast<OpType1 *>(right))
                {
                    Object *r = m_gc.new_object(ObType);
                    static_cast<ResType *>(r)->m_val = op(p_left->m_val, p_right->m_val);
                    return r;
                }
            }

            return nullptr;
        }

        bool equals(Object *o1, Object *o2)
        {
            if (o1 == nullptr && o2 == nullptr)
            {
                return true;
            }

            if (o2 == nullptr)
            {
                return false;
            }

            return o1->equals(o2);
        }

        static bool is_true(Object *o);

    public:
        void interpret(Expr *e);
        Object *evaluate(Expr *e);

        Object *visit_grouping(Grouping *e) override;
        Object *visit_binary_expr(BinaryExpr *e) override;
        Object *visit_logical_expr(LogicalExpr *e) override;
        Object *visit_unary_expr(UnaryExpr *e) override;
        Object *visit_literal(Literal *e) override;
    };
}