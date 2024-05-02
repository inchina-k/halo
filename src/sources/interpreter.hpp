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
#include <stdexcept>
#include <string>

namespace halo
{

    class Interpreter : public ExprVisitor, public StmtVisitor
    {
        struct DebugInfo
        {
            size_t m_line;
            std::string m_name;
        };

        struct DebugManager
        {
            Interpreter *m_interp;

            DebugManager(Interpreter *interp)
                : m_interp(interp)
            {
                m_interp->m_debug_info.emplace_back();
            }

            ~DebugManager()
            {
                m_interp->m_debug_info.pop_back();
            }
        };

        std::vector<DebugInfo> m_debug_info;

        Environment m_env;
        std::istream &m_in;
        std::ostream &m_out;
        int m_fun_scope_counter;
        int m_max_fun_depth;

        template <typename OpType, ObjectType ObType, typename ResType = OpType, typename Op>
        Object *bin_op(Object *left, Object *right, Op op)
        {
            if (OpType *p_left = dynamic_cast<OpType *>(left))
            {
                if (OpType *p_right = dynamic_cast<OpType *>(right))
                {
                    Object *r = GC::instance().new_object(ObType);
                    dynamic_cast<ResType *>(r)->m_val = op(p_left->m_val, p_right->m_val);
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

        size_t get_curr_error_line();

        std::string get_curr_error_element();

    public:
        Interpreter(std::istream &in = std::cin, std::ostream &out = std::cout);

        std::string report_error(std::string desc);

        Environment &get_env()
        {
            return m_env;
        }

        std::istream &get_in()
        {
            return m_in;
        }

        std::ostream &get_out()
        {
            return m_out;
        }

        int get_recursion_depth() const
        {
            return m_max_fun_depth;
        }

        void set_recursion_depth(int depth)
        {
            m_max_fun_depth = depth;
        }

        void inc_fun_scope_counter()
        {
            ++m_fun_scope_counter;

            if (m_fun_scope_counter > m_max_fun_depth)
            {
                throw std::runtime_error(report_error("max function depth exceeded '" + std::to_string(m_max_fun_depth) + "'"));
            }
        }

        void dec_fun_scope_counter()
        {
            --m_fun_scope_counter;
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
        Object *visit_dot_expr(Dot *e) override;
        Object *visit_subscript_expr(Subscript *e) override;
        Object *visit_literal(Literal *e) override;
        Object *visit_var(Var *e) override;
        Object *visit_lambda(Lambda *e) override;
        Object *visit_list(ListExpr *e) override;

        void visit_var_stmt(VarStmt *e) override;
        void visit_assignment_stmt(AssignmentStmt *e) override;
        void visit_expression_stmt(ExpressionStmt *e) override;
        void visit_if_stmt(IfStmt *e) override;
        void visit_while_stmt(WhileStmt *e) override;
        void visit_for_stmt(ForStmt *e) override;
        void visit_break_stmt(BreakStmt *e) override;
        void visit_continue_stmt(ContinueStmt *e) override;
        void visit_fun_stmt(FunStmt *e) override;
        void visit_return_stmt(ReturnStmt *e) override;
        void visit_class_stmt(ClassStmt *e) override;
    };
}