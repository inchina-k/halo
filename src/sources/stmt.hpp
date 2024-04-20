#pragma once

#include "expr.hpp"

#include <vector>
#include <memory>
#include <string>
#include <set>

namespace halo
{
    class StmtVisitor;

    struct Stmt
    {
        virtual ~Stmt() {}

        virtual void visit(StmtVisitor *v) = 0;
    };

    struct VarStmt : Stmt
    {
        Token m_token;
        Expr *m_expr;

        VarStmt(Token t, Expr *e)
            : m_token(t), m_expr(e)
        {
        }

        void visit(StmtVisitor *v) override;
    };

    struct AssignmentStmt : Stmt
    {
        Expr *m_lval;
        Expr *m_expr;

        AssignmentStmt(Expr *lv, Expr *e)
            : m_lval(lv), m_expr(e)
        {
        }

        void visit(StmtVisitor *v) override;
    };

    struct ExpressionStmt : Stmt
    {
        Expr *m_expr;

        ExpressionStmt(Expr *e)
            : m_expr(e)
        {
        }

        void visit(StmtVisitor *v) override;
    };

    struct IfStmt : Stmt
    {
        std::vector<Expr *> m_conds;
        std::vector<std::vector<std::unique_ptr<Stmt>>> m_then_branches;
        std::vector<std::unique_ptr<Stmt>> m_else_branch;

        IfStmt(std::vector<Expr *> conds, std::vector<std::vector<std::unique_ptr<Stmt>>> then_branches, std::vector<std::unique_ptr<Stmt>> else_branch)
            : m_conds(conds), m_then_branches(std::move(then_branches)), m_else_branch(std::move(else_branch))
        {
        }

        void visit(StmtVisitor *v) override;
    };

    struct WhileStmt : Stmt
    {
        Expr *m_cond;
        std::vector<std::unique_ptr<Stmt>> m_do_branch;

        WhileStmt(Expr *cond, std::vector<std::unique_ptr<Stmt>> do_branch)
            : m_cond(cond), m_do_branch(std::move(do_branch))
        {
        }

        void visit(StmtVisitor *v) override;
    };

    struct ForStmt : Stmt
    {
        Token m_identifier;
        Expr *m_begin;
        Expr *m_end;
        Expr *m_step;
        Expr *m_iterable;
        std::vector<std::unique_ptr<Stmt>> m_do_branch;

        ForStmt(Token identifier, Expr *begin, Expr *end, Expr *step, Expr *iterable, std::vector<std::unique_ptr<Stmt>> do_branch)
            : m_identifier(identifier), m_begin(begin), m_end(end), m_step(step), m_iterable(iterable), m_do_branch(std::move(do_branch))
        {
        }

        void visit(StmtVisitor *v) override;
    };

    struct BreakStmt : Stmt
    {
        void visit(StmtVisitor *v) override;
    };

    struct ContinueStmt : Stmt
    {
        void visit(StmtVisitor *v) override;
    };

    struct FunStmt : Stmt
    {
        Token m_name;
        std::vector<Token> m_params;
        std::vector<std::unique_ptr<Stmt>> m_body;

        FunStmt(Token name, const std::vector<Token> &params, std::vector<std::unique_ptr<Stmt>> body)
            : m_name(name), m_params(params), m_body(std::move(body))
        {
        }

        void visit(StmtVisitor *v) override;
    };

    struct ReturnStmt : Stmt
    {
        Expr *m_expr;

        ReturnStmt(Expr *expr)
            : m_expr(expr)
        {
        }

        void visit(StmtVisitor *v) override;
    };

    struct ClassStmt : Stmt
    {
        Token m_name;
        std::set<std::string> m_fields;
        std::vector<std::unique_ptr<FunStmt>> m_methods;

        ClassStmt(Token name, const std::set<std::string> &fields, std::vector<std::unique_ptr<FunStmt>> methods)
            : m_name(name), m_fields(fields), m_methods(std::move(methods))
        {
        }

        void visit(StmtVisitor *v) override;
    };

    struct StmtVisitor
    {
        virtual void visit_var_stmt(VarStmt *e) = 0;
        virtual void visit_assignment_stmt(AssignmentStmt *e) = 0;
        virtual void visit_expression_stmt(ExpressionStmt *e) = 0;
        virtual void visit_if_stmt(IfStmt *e) = 0;
        virtual void visit_while_stmt(WhileStmt *e) = 0;
        virtual void visit_for_stmt(ForStmt *e) = 0;
        virtual void visit_break_stmt([[maybe_unused]] BreakStmt *e) = 0;
        virtual void visit_continue_stmt([[maybe_unused]] ContinueStmt *e) = 0;
        virtual void visit_fun_stmt(FunStmt *e) = 0;
        virtual void visit_return_stmt(ReturnStmt *e) = 0;
        virtual void visit_class_stmt(ClassStmt *e) = 0;
    };
}