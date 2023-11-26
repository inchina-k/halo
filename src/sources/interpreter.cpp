#include "Interpreter.hpp"

#include <iostream>

using namespace std;
using namespace halo;

void Interpreter::interpret(Expr *e)
{
    m_res = evaluate(e);

    cout << (m_res != nullptr ? m_res->to_str() : "null") << endl;
}

Object *Interpreter::evaluate(Expr *e)
{
    return e->visit(this);
}

Object *Interpreter::visit_grouping(Grouping *e)
{
    return evaluate(e);
}

Object *Interpreter::visit_binary_expr(BinaryExpr *e)
{
    Object *o1 = evaluate(e->m_left);
    Object *o2 = evaluate(e->m_right);

    switch (e->m_token.m_type)
    {
    case TokenType::Plus:
        if (Int *i1 = dynamic_cast<Int *>(o1))
        {
            if (Int *i2 = dynamic_cast<Int *>(o2))
            {
                Object *r = m_gc.new_object(ObjectType::Int);
                static_cast<Int *>(r)->m_val = i1->m_val + i2->m_val;
                return r;
            }
        }
        if (Float *f1 = dynamic_cast<Float *>(o1))
        {
            if (Float *f2 = dynamic_cast<Float *>(o2))
            {
                Object *r = m_gc.new_object(ObjectType::Float);
                static_cast<Float *>(r)->m_val = f1->m_val + f2->m_val;
                return r;
            }
        }
        if (String *s1 = dynamic_cast<String *>(o1))
        {
            if (String *s2 = dynamic_cast<String *>(o2))
            {
                Object *r = m_gc.new_object(ObjectType::String);
                static_cast<String *>(r)->m_val = s1->m_val + s2->m_val;
                return r;
            }
        }
        throw runtime_error("incorrect operand for '+'");
    case TokenType::Minus:
        if (Int *i1 = dynamic_cast<Int *>(o1))
        {
            if (Int *i2 = dynamic_cast<Int *>(o2))
            {
                Object *r = m_gc.new_object(ObjectType::Int);
                static_cast<Int *>(r)->m_val = i1->m_val - i2->m_val;
                return r;
            }
        }
        if (Float *f1 = dynamic_cast<Float *>(o1))
        {
            if (Float *f2 = dynamic_cast<Float *>(o2))
            {
                Object *r = m_gc.new_object(ObjectType::Float);
                static_cast<Float *>(r)->m_val = f1->m_val - f2->m_val;
                return r;
            }
        }
        throw runtime_error("incorrect operand for '-'");
    case TokenType::Mul:
        if (Int *i1 = dynamic_cast<Int *>(o1))
        {
            if (Int *i2 = dynamic_cast<Int *>(o2))
            {
                Object *r = m_gc.new_object(ObjectType::Int);
                static_cast<Int *>(r)->m_val = i1->m_val * i2->m_val;
                return r;
            }
        }
        if (Float *f1 = dynamic_cast<Float *>(o1))
        {
            if (Float *f2 = dynamic_cast<Float *>(o2))
            {
                Object *r = m_gc.new_object(ObjectType::Float);
                static_cast<Float *>(r)->m_val = f1->m_val * f2->m_val;
                return r;
            }
        }
        throw runtime_error("incorrect operand for '*'");
    case TokenType::Div:
        if (Int *i1 = dynamic_cast<Int *>(o1))
        {
            if (Int *i2 = dynamic_cast<Int *>(o2))
            {
                Object *r = m_gc.new_object(ObjectType::Int);
                static_cast<Int *>(r)->m_val = i1->m_val / i2->m_val;
                return r;
            }
        }
        if (Float *f1 = dynamic_cast<Float *>(o1))
        {
            if (Float *f2 = dynamic_cast<Float *>(o2))
            {
                Object *r = m_gc.new_object(ObjectType::Float);
                static_cast<Float *>(r)->m_val = f1->m_val / f2->m_val;
                return r;
            }
        }
        throw runtime_error("incorrect operand for '/'");
    case TokenType::Mod:
        if (Int *i1 = dynamic_cast<Int *>(o1))
        {
            if (Int *i2 = dynamic_cast<Int *>(o2))
            {
                Object *r = m_gc.new_object(ObjectType::Int);
                static_cast<Int *>(r)->m_val = i1->m_val % i2->m_val;
                return r;
            }
        }
        throw runtime_error("incorrect operand for '%'");
    default:
        throw runtime_error("incorrect operand");
    }
}

Object *Interpreter::visit_unary_expr(UnaryExpr *e)
{
    Object *o = evaluate(e->m_expr);

    switch (e->m_token.m_type)
    {
    case TokenType::Not:
        if (Bool *b = dynamic_cast<Bool *>(o))
        {
            Object *r = m_gc.new_object(ObjectType::Bool);
            static_cast<Bool *>(r)->m_val = !b->m_val;
            return r;
        }
        throw runtime_error("incorrect operand for 'not'");
    case TokenType::Minus:
        if (Int *i = dynamic_cast<Int *>(o))
        {
            Object *r = m_gc.new_object(ObjectType::Int);
            static_cast<Int *>(r)->m_val = -i->m_val;
            return r;
        }
        if (Float *f = dynamic_cast<Float *>(o))
        {
            Object *r = m_gc.new_object(ObjectType::Float);
            static_cast<Float *>(r)->m_val = -f->m_val;
            return r;
        }
        throw runtime_error("incorrect operand for '-'");

    default:
        throw runtime_error("incorrect operand");
        ;
    }
}

Object *Interpreter::visit_int_literal(IntLiteral *e)
{
    Object *o = m_gc.new_object(ObjectType::Int);
    static_cast<Int *>(o)->m_val = e->m_val;
    return o;
}

Object *Interpreter::visit_float_literal(FloatLiteral *e)
{
    Object *o = m_gc.new_object(ObjectType::Float);
    static_cast<Float *>(o)->m_val = e->m_val;
    return o;
}

Object *Interpreter::visit_bool_literal(BoolLiteral *e)
{
    Object *o = m_gc.new_object(ObjectType::Bool);
    static_cast<Bool *>(o)->m_val = e->m_val;
    return o;
}

Object *Interpreter::visit_string_literal(StringLiteral *e)
{
    Object *o = m_gc.new_object(ObjectType::String);
    static_cast<String *>(o)->m_val = e->m_val;
    return o;
}