#pragma once

#include "expr.hpp"

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

    struct StmtVisitor
    {
        virtual void visit_var_stmt(VarStmt *e) = 0;
        virtual void visit_assignment_stmt(AssignmentStmt *e) = 0;
    };
}