#pragma once

#include "expr.hpp"

#include <vector>
#include <memory>

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
        Token m_token;
        Expr *m_expr;

        AssignmentStmt(Token t, Expr *e)
            : m_token(t), m_expr(e)
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
        Expr *m_cond;
        std::vector<std::unique_ptr<Stmt>> m_then_branch;
        std::vector<std::unique_ptr<Stmt>> m_else_branch;

        IfStmt(Expr *cond, std::vector<std::unique_ptr<Stmt>> then_branch, std::vector<std::unique_ptr<Stmt>> else_branch)
            : m_cond(cond), m_then_branch(std::move(then_branch)), m_else_branch(std::move(else_branch))
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
    };
}