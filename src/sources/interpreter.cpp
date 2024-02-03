#include "Interpreter.hpp"

#include <iostream>
#include <string>

using namespace std;
using namespace halo;

struct PrintLine : Callable
{
    Interpreter *interp = nullptr;

    Object *call(const std::vector<Object *> &args) override
    {
        interp->get_out() << (!args.front() ? "null"s : args.front()->to_str()) << endl;
        return nullptr;
    }

    int arity() const override
    {
        return 1;
    }

    string to_str() const override
    {
        return "println";
    }
};

struct ReadLine : Callable
{
    Interpreter *interp = nullptr;

    Object *call([[maybe_unused]] const std::vector<Object *> &args) override
    {
        string str;
        getline(interp->get_in(), str);
        Object *res = GC::instance().new_object(ObjectType::String);
        dynamic_cast<String *>(res)->m_val = str;

        return res;
    }

    int arity() const override
    {
        return 0;
    }

    string to_str() const override
    {
        return "readln";
    }
};

struct ToInt : Callable
{
    Object *call(const std::vector<Object *> &args) override
    {
        Object *res = GC::instance().new_object(ObjectType::Int);
        dynamic_cast<Int *>(res)->m_val = stoi(args.front()->to_str());
        return res;
    }

    int arity() const override
    {
        return 1;
    }

    string to_str() const override
    {
        return "to_int";
    }
};

struct ToStr : Callable
{
    Object *call(const std::vector<Object *> &args) override
    {
        Object *res = GC::instance().new_object(ObjectType::String);
        dynamic_cast<String *>(res)->m_val = args.front()->to_str();
        return res;
    }

    int arity() const override
    {
        return 1;
    }

    string to_str() const override
    {
        return "to_str";
    }
};

Interpreter::Interpreter(istream &in, ostream &out)
    : m_in(in), m_out(out)
{
    m_env.add_scope();

    PrintLine *pl = static_cast<PrintLine *>(GC::instance().new_object<PrintLine>());
    pl->interp = this;
    m_env.define(Token(TokenType::Var, "println", 0, 0), pl);

    ReadLine *rl = static_cast<ReadLine *>(GC::instance().new_object<ReadLine>());
    rl->interp = this;
    m_env.define(Token(TokenType::Var, "readln", 0, 0), rl);

    m_env.define(Token(TokenType::Var, "to_int", 0, 0), GC::instance().new_object<ToInt>());
    m_env.define(Token(TokenType::Var, "to_str", 0, 0), GC::instance().new_object<ToStr>());
}

void Interpreter::interpret(Expr *e)
{
    Object *res = (e == nullptr) ? nullptr : evaluate(e);

    cout << (res != nullptr ? res->to_str() : "null") << endl;
}

Object *Interpreter::evaluate(Expr *e)
{
    return e->visit(this);
}

void Interpreter::execute(const std::vector<unique_ptr<Stmt>> &stmts)
{
    for (auto &stmt : stmts)
    {
        execute_stmt(stmt.get());
    }

    // test purpose
    // for (auto &e : m_env.m_data.back())
    // {
    //     cout << e.first << ':' << (e.second ? e.second->to_str() : "null") << endl;
    // }
}

void Interpreter::execute_stmt(Stmt *stmt)
{
    stmt->visit(this);
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
        Object *r = GC::instance().new_object(ObjectType::Bool);
        static_cast<Bool *>(r)->m_val = equals(o1, o2);
        return r;
    }
    case TokenType::BangEqual:
    {
        Object *r = GC::instance().new_object(ObjectType::Bool);
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
        Object *r = GC::instance().new_object(ObjectType::Bool);
        static_cast<Bool *>(r)->m_val = !is_true(o);
        return r;
    }
    case TokenType::Minus:
        if (Int *i = dynamic_cast<Int *>(o))
        {
            Object *r = GC::instance().new_object(ObjectType::Int);
            static_cast<Int *>(r)->m_val = -i->m_val;
            return r;
        }
        if (Float *f = dynamic_cast<Float *>(o))
        {
            Object *r = GC::instance().new_object(ObjectType::Float);
            static_cast<Float *>(r)->m_val = -f->m_val;
            return r;
        }
        throw runtime_error("line " + to_string(e->m_token.m_line) + ": incorrect operand for '" + e->m_token.m_lexeme + "'");

    default:
        throw runtime_error("line " + to_string(e->m_token.m_line) + ": incorrect operand for '" + e->m_token.m_lexeme + "'");
    }
}

Object *Interpreter::visit_call_expr(Call *e)
{
    Object *o = evaluate(e->m_expr);

    Callable *c = dynamic_cast<Callable *>(o);

    if (!c)
    {
        throw runtime_error(o->to_str() + " is not a function");
    }

    if (c->arity() != int(e->m_args.size()))
    {
        throw runtime_error(c->to_str() + ": incorrect number of args");
    }

    vector<Object *> args;

    for (auto arg : e->m_args)
    {
        args.push_back(evaluate(arg));
    }

    return c->call(args);
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
        Object *o = GC::instance().new_object(ObjectType::Int);
        static_cast<Int *>(o)->m_val = stoi(e->m_token.m_lexeme);
        return o;
    }
    case TokenType::FloatLiteral:
    {
        Object *o = GC::instance().new_object(ObjectType::Float);
        static_cast<Float *>(o)->m_val = stoi(e->m_token.m_lexeme);
        return o;
    }
    case TokenType::True:
    case TokenType::False:
    {
        Object *o = GC::instance().new_object(ObjectType::Bool);
        static_cast<Bool *>(o)->m_val = e->m_token.m_type == TokenType::True;
        return o;
    }
    case TokenType::StrLiteral:
    {
        Object *o = GC::instance().new_object(ObjectType::String);
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

Object *Interpreter::visit_var(Var *e)
{
    return m_env.get(e->m_token);
}

void Interpreter::visit_var_stmt(VarStmt *e)
{
    m_env.define(e->m_token, e->m_expr ? evaluate(e->m_expr) : nullptr);
}

void Interpreter::visit_assignment_stmt(AssignmentStmt *e)
{
    m_env.assign(e->m_token, evaluate(e->m_expr));
}

void Interpreter::visit_expression_stmt(ExpressionStmt *e)
{
    evaluate(e->m_expr);
}

void Interpreter::visit_if_stmt(IfStmt *e)
{
    for (size_t i = 0; i < e->m_conds.size(); ++i)
    {
        Object *o = evaluate(e->m_conds[i]);

        if (is_true(o))
        {
            m_env.add_scope();
            execute(e->m_then_branches[i]);
            m_env.remove_scope();
            return;
        }
    }

    if (!e->m_else_branch.empty())
    {
        m_env.add_scope();
        execute(e->m_else_branch);
        m_env.remove_scope();
    }
}