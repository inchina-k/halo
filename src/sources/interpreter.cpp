#include "Interpreter.hpp"

#include <iostream>

using namespace std;
using namespace halo;

void Interpreter::interpret(Expr *e)
{
    Object *res = evaluate(e);

    cout << (res != nullptr ? res->to_str() : "null") << endl;
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
        if (Object *res = bin_op<Int, ObjectType::Int>(o1, o2, plus<long long>()))
        {
            return res;
        }
        if (Object *res = bin_op<Float, ObjectType::Float>(o1, o2, plus<double>()))
        {
            return res;
        }
        if (Object *res = bin_op<String, ObjectType::String>(o1, o2, plus<string>()))
        {
            return res;
        }
        if (Object *res = bin_op_conv<Float, Int, ObjectType::Float>(o1, o2, plus<double>()))
        {
            return res;
        }
        throw runtime_error("line " + to_string(e->m_token.m_line) + ": incorrect operand types for '" + e->m_token.m_lexeme + "'");
    case TokenType::Minus:
        if (Object *res = bin_op<Int, ObjectType::Int>(o1, o2, minus<long long>()))
        {
            return res;
        }
        if (Object *res = bin_op<Float, ObjectType::Float>(o1, o2, minus<double>()))
        {
            return res;
        }
        if (Object *res = bin_op_conv<Float, Int, ObjectType::Float>(o1, o2, minus<double>()))
        {
            return res;
        }
        throw runtime_error("line " + to_string(e->m_token.m_line) + ": incorrect operand types for '" + e->m_token.m_lexeme + "'");
    case TokenType::Mul:
        if (Object *res = bin_op<Int, ObjectType::Int>(o1, o2, multiplies<long long>()))
        {
            return res;
        }
        if (Object *res = bin_op<Float, ObjectType::Float>(o1, o2, multiplies<double>()))
        {
            return res;
        }
        if (Object *res = bin_op_conv<Float, Int, ObjectType::Float>(o1, o2, multiplies<double>()))
        {
            return res;
        }
        throw runtime_error("line " + to_string(e->m_token.m_line) + ": incorrect operand types for '" + e->m_token.m_lexeme + "'");
    case TokenType::Div:
        if (Object *res = bin_op<Int, ObjectType::Int>(o1, o2, divides<long long>()))
        {
            return res;
        }
        if (Object *res = bin_op<Float, ObjectType::Float>(o1, o2, divides<double>()))
        {
            return res;
        }
        if (Object *res = bin_op_conv<Float, Int, ObjectType::Float>(o1, o2, divides<double>()))
        {
            return res;
        }
        throw runtime_error("line " + to_string(e->m_token.m_line) + ": incorrect operand types for '" + e->m_token.m_lexeme + "'");
    case TokenType::Mod:
        if (Object *res = bin_op<Int, ObjectType::Int>(o1, o2, modulus<long long>()))
        {
            return res;
        }
        throw runtime_error("line " + to_string(e->m_token.m_line) + ": incorrect operand types for '" + e->m_token.m_lexeme + "'");
    case TokenType::Less:
        if (Object *res = bin_op<Int, ObjectType::Bool, Bool>(o1, o2, less<long long>()))
        {
            return res;
        }
        if (Object *res = bin_op<Float, ObjectType::Bool, Bool>(o1, o2, less<double>()))
        {
            return res;
        }
        if (Object *res = bin_op_conv<Float, Int, ObjectType::Bool, Bool>(o1, o2, less<double>()))
        {
            return res;
        }
        throw runtime_error("line " + to_string(e->m_token.m_line) + ": incorrect operand types for '" + e->m_token.m_lexeme + "'");
    case TokenType::LessEqual:
        if (Object *res = bin_op<Int, ObjectType::Bool, Bool>(o1, o2, less_equal<long long>()))
        {
            return res;
        }
        if (Object *res = bin_op<Float, ObjectType::Bool, Bool>(o1, o2, less_equal<double>()))
        {
            return res;
        }
        if (Object *res = bin_op_conv<Float, Int, ObjectType::Bool, Bool>(o1, o2, less_equal<double>()))
        {
            return res;
        }
        throw runtime_error("line " + to_string(e->m_token.m_line) + ": incorrect operand types for '" + e->m_token.m_lexeme + "'");
    case TokenType::Greater:
        if (Object *res = bin_op<Int, ObjectType::Bool, Bool>(o1, o2, greater<long long>()))
        {
            return res;
        }
        if (Object *res = bin_op<Float, ObjectType::Bool, Bool>(o1, o2, greater<double>()))
        {
            return res;
        }
        if (Object *res = bin_op_conv<Float, Int, ObjectType::Bool, Bool>(o1, o2, greater<double>()))
        {
            return res;
        }
        throw runtime_error("line " + to_string(e->m_token.m_line) + ": incorrect operand types for '" + e->m_token.m_lexeme + "'");
    case TokenType::GreaterEqual:
        if (Object *res = bin_op<Int, ObjectType::Bool, Bool>(o1, o2, greater_equal<long long>()))
        {
            return res;
        }
        if (Object *res = bin_op<Float, ObjectType::Bool, Bool>(o1, o2, greater_equal<double>()))
        {
            return res;
        }
        if (Object *res = bin_op_conv<Float, Int, ObjectType::Bool, Bool>(o1, o2, greater_equal<double>()))
        {
            return res;
        }
        throw runtime_error("line " + to_string(e->m_token.m_line) + ": incorrect operand types for '" + e->m_token.m_lexeme + "'");
    case TokenType::EqualEqual:
    {
        Object *r = m_gc.new_object(ObjectType::Bool);
        static_cast<Bool *>(r)->m_val = equals(o1, o2);
        return r;
    }
    case TokenType::BangEqual:
    {
        Object *r = m_gc.new_object(ObjectType::Bool);
        static_cast<Bool *>(r)->m_val = !equals(o1, o2);
        return r;
    }
    default:
        throw runtime_error("line " + to_string(e->m_token.m_line) + ": unknown operation '" + e->m_token.m_lexeme + "'");
    }
}

Object *Interpreter::visit_logical_expr(LogicalExpr *e)
{
    Object *left = evaluate(e->m_left);

    if (e->m_token.m_type == TokenType::Or && is_true(left))
    {
        return left;
    }

    if (e->m_token.m_type == TokenType::And && !is_true(left))
    {
        return left;
    }

    return evaluate(e->m_right);
}

Object *Interpreter::visit_unary_expr(UnaryExpr *e)
{
    Object *o = evaluate(e->m_expr);

    switch (e->m_token.m_type)
    {
    case TokenType::Not:
    {
        Object *r = m_gc.new_object(ObjectType::Bool);
        static_cast<Bool *>(r)->m_val = !is_true(o);
        return r;
    }
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
        throw runtime_error("line " + to_string(e->m_token.m_line) + ": incorrect operand for '" + e->m_token.m_lexeme + "'");

    default:
        throw runtime_error("line " + to_string(e->m_token.m_line) + ": incorrect operand for '" + e->m_token.m_lexeme + "'");
    }
}

Object *Interpreter::visit_literal(Literal *e)
{
    if (e->m_val)
    {
        return e->m_val;
    }

    switch (e->m_token.m_type)
    {
    case TokenType::Null:
        return e->m_val;
    case TokenType::IntLiteral:
    {
        Object *o = m_gc.new_object(ObjectType::Int);
        static_cast<Int *>(o)->m_val = stoi(e->m_token.m_lexeme);
        return o;
    }
    case TokenType::FloatLiteral:
    {
        Object *o = m_gc.new_object(ObjectType::Float);
        static_cast<Float *>(o)->m_val = stoi(e->m_token.m_lexeme);
        return o;
    }
    case TokenType::True:
    case TokenType::False:
    {
        Object *o = m_gc.new_object(ObjectType::Bool);
        static_cast<Bool *>(o)->m_val = e->m_token.m_type == TokenType::True;
        return o;
    }
    case TokenType::StrLiteral:
    {
        Object *o = m_gc.new_object(ObjectType::String);
        static_cast<String *>(o)->m_val = e->m_token.m_lexeme;
        return o;
    }
    default:
        throw runtime_error("line " + to_string(e->m_token.m_line) + ": unknown literal '" + e->m_token.m_lexeme + "'");
    }
}

bool Interpreter::is_true(Object *o)
{
    if (!o)
    {
        return false;
    }

    if (Bool *b = dynamic_cast<Bool *>(o))
    {
        return b->m_val;
    }

    if (String *s = dynamic_cast<String *>(o))
    {
        return !s->m_val.empty();
    }

    if (Int *i = dynamic_cast<Int *>(o))
    {
        return i->m_val != 0;
    }

    if (Float *f = dynamic_cast<Float *>(o))
    {
        return f->m_val != 0.0;
    }

    return true;
}
