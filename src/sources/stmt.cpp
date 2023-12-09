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