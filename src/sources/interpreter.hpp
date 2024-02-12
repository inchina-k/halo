#pragma once

#include "expr.hpp"
#include "object.hpp"
#include "gc.hpp"
#include "env.hpp"
#include "stmt.hpp"
#include "gc.hpp"

#include <functional>
#include <memory>
#include <iostream>

namespace halo
{
    class Interpreter : public ExprVisitor, public StmtVisitor
    {
        Environment m_env;
        std::istream &m_in;
        std::ostream &m_out;

        template <typename OpType, ObjectType ObType, typename ResType = OpType, typename Op>
        Object *bin_op(Object *left, Object *right, Op op)
        {
            if (OpType *p_left = dynamic_cast<OpType *>(left))
            {
                if (OpType *p_right = dynamic_cast<OpType *>(right))
                {
                    Object *r = GC::instance().new_object(ObType);
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
                    Object *r = GC::instance().new_object(ObType);
                    static_cast<ResType *>(r)->m_val = op(p_left->m_val, p_right->m_val);
                    return r;
                }
            }
            if (OpType2 *p_left = dynamic_cast<OpType2 *>(left))
            {
                if (OpType1 *p_right = dynamic_cast<OpType1 *>(right))
                {
                    Object *r = GC::instance().new_object(ObType);
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
        Interpreter(std::istream &in = std::cin, std::ostream &out = std::cout);

        std::istream &get_in()
        {
            return m_in;
        }

        std::ostream &get_out()
        {
            return m_out;
        }

        void interpret(Expr *e);
        Object *evaluate(Expr *e);
        void execute(const std::vector<std::unique_ptr<Stmt>> &stmts);
        void execute_stmt(Stmt *stmt);

        Object *visit_grouping(Grouping *e) override;
        Object *visit_binary_expr(BinaryExpr *e) override;
        Object *visit_logical_expr(LogicalExpr *e) override;
        Object *visit_unary_expr(UnaryExpr *e) override;
        Object *visit_call_expr(Call *e) override;
        Object *visit_literal(Literal *e) override;

        Object *visit_var(Var *e) override;

        void visit_var_stmt(VarStmt *e) override;
        void visit_assignment_stmt(AssignmentStmt *e) override;
        void visit_expression_stmt(ExpressionStmt *e) override;
        void visit_if_stmt(IfStmt *e) override;
        void visit_while_stmt(WhileStmt *e) override;
        void visit_break_stmt(BreakStmt *e) override;
        void visit_continue_stmt(ContinueStmt *e) override;
    };
}