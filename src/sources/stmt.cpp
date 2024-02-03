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