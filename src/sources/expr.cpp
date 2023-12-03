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

Object *IntLiteral::visit(ExprVisitor *v)
{
    return v->visit_int_literal(this);
}

Object *FloatLiteral::visit(ExprVisitor *v)
{
    return v->visit_float_literal(this);
}

Object *BoolLiteral::visit(ExprVisitor *v)
{
    return v->visit_bool_literal(this);
}

Object *StringLiteral::visit(ExprVisitor *v)
{
    return v->visit_string_literal(this);
}

Object *NullLiteral::visit(ExprVisitor *v)
{
    return v->visit_null_literal(this);
}
