#include "expr.hpp"

using namespace halo;
using namespace std;

Object *Grouping::visit(ExprVisitor *v)
{
    return v->visit_grouping(this);
}

Object *BinaryExpr::visit(ExprVisitor *v)
{
    return v->visit_binary_expr(this);
}

Object *LogicalExpr::visit(ExprVisitor *v)
{
    return v->visit_logical_expr(this);
}

Object *UnaryExpr::visit(ExprVisitor *v)
{
    return v->visit_unary_expr(this);
}

Object *Call::visit(ExprVisitor *v)
{
    return v->visit_call_expr(this);
}

Object *Literal::visit(ExprVisitor *v)
{
    return v->visit_literal(this);
}

Object *Var::visit(ExprVisitor *v)
{
    return v->visit_var(this);
}
