#include "expr.hpp"
#include "stmt.hpp"

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

Object *Lambda::visit(ExprVisitor *v)
{
    return v->visit_lambda(this);
}

Lambda::Lambda(const std::vector<Token> &capture, const std::vector<Token> &params, std::vector<std::unique_ptr<Stmt>> body)
    : m_capture(capture), m_params(params), m_body(std::move(body))
{
}