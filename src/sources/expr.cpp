#include "expr.hpp"

using namespace halo;
using namespace std;

Literal::Literal(Token t)
    : m_token(t), m_val(nullptr)
{
}

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

Object *Literal::visit(ExprVisitor *v)
{
    return v->visit_literal(this);
}
