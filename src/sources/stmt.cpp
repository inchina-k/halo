#include "stmt.hpp"

using namespace std;
using namespace halo;

void VarStmt::visit(StmtVisitor *v)
{
    v->visit_var_stmt(this);
}

void AssignmentStmt::visit(StmtVisitor *v)
{
    v->visit_assignment_stmt(this);
}

void ExpressionStmt::visit(StmtVisitor *v)
{
    v->visit_expression_stmt(this);
}

void IfStmt::visit(StmtVisitor *v)
{
    v->visit_if_stmt(this);
}

void WhileStmt::visit(StmtVisitor *v)
{
    v->visit_while_stmt(this);
}

void ForStmt::visit(StmtVisitor *v)
{
    v->visit_for_stmt(this);
}

void BreakStmt::visit(StmtVisitor *v)
{
    v->visit_break_stmt(this);
}

void ContinueStmt::visit(StmtVisitor *v)
{
    v->visit_continue_stmt(this);
}

void FunStmt::visit(StmtVisitor *v)
{
    v->visit_fun_stmt(this);
}

void ReturnStmt::visit(StmtVisitor *v)
{
    v->visit_return_stmt(this);
}

void ClassStmt::visit(StmtVisitor *v)
{
    v->visit_class_stmt(this);
}
