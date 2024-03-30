#include "Interpreter.hpp"

#include <iostream>
#include <string>
#include <algorithm>

using namespace std;
using namespace halo;

struct Scope
{
    Environment &m_env;

    Scope(Environment &env, Environment::ScopeType st)
        : m_env(env)
    {
        m_env.add_scope(st);
    }

    ~Scope()
    {
        m_env.remove_scope();
    }
};

struct FunScope
{
    Environment &m_env;

    FunScope(Environment &env, Environment::ScopeType st)
        : m_env(env)
    {
        m_env.add_scope(st);
    }

    ~FunScope()
    {
        m_env.remove_scope();
    }
};

struct BreakSignal
{
    // empty
};

struct ContinueSignal
{
    // empty
};

struct ReturnSignal
{
    Object *m_res;

    ReturnSignal(Object *res)
        : m_res(res)
    {
    }
};

struct Function : Callable
{
    Interpreter *m_interp = nullptr;
    FunStmt *m_fst = nullptr;

    Object *call(const std::vector<Object *> &args) override
    {
        FunScope fc(m_interp->get_env(), Environment::ScopeType::Fun);
        m_interp->inc_fun_scope_counter(m_fst->m_name.m_line);

        for (size_t i = 0; i < args.size(); ++i)
        {
            m_interp->get_env().define(m_fst->m_params[i], args[i]);
        }

        try
        {
            m_interp->execute(m_fst->m_body);
        }
        catch (const ReturnSignal &rs)
        {
            m_interp->dec_fun_scope_counter();
            return rs.m_res;
        }

        m_interp->dec_fun_scope_counter();
        return nullptr;
    }

    int arity() const override
    {
        return m_fst->m_params.size();
    }

    string to_str() const override
    {
        return "<fun " + m_fst->m_name.m_lexeme + ">(" + to_string(arity()) + ")";
    }
};

struct LambdaFunction : Callable
{
    Interpreter *m_interp = nullptr;
    Lambda *m_l = nullptr;
    std::unordered_map<std::string, Object *> m_capture;

    // struct LambdaScope
    // {
    //     Interpreter &m_interp;
    //     LambdaFunction &m_lf;
    //     Environment m_env;

    //     LambdaScope(Interpreter &interp, LambdaFunction &lf)
    //         : m_interp(interp), m_lf(lf)
    //     {
    //         m_env.m_data.push_back(interp.get_env().m_data[0]);
    //         m_env.m_data.push_back(lf.m_capture);
    //         m_interp.get_env().swap_env(m_env);
    //     }

    //     ~LambdaScope()
    //     {
    //         m_env.m_data[0] = m_interp.get_env().m_data[0];
    //         m_lf.m_capture = m_interp.get_env().m_data[1];
    //         m_interp.get_env().swap_env(m_env);
    //     }
    // };

    Object *call(const std::vector<Object *> &args) override
    {
        // LambdaScope lscope(*m_interp, *this);
        // m_interp->get_env().m_data.emplace_back();
        // m_interp->get_env().m_scopes.push_back(Environment::ScopeType::Lambda);

        // for (size_t i = 0; i < args.size(); ++i)
        // {
        //     m_interp->get_env().define(m_l->m_params[i], args[i]);
        // }

        // try
        // {
        //     m_interp->execute(m_l->m_body);
        // }
        // catch (const ReturnSignal &rs)
        // {
        //     return rs.m_res;
        // }

        FunScope fc(m_interp->get_env(), Environment::ScopeType::Lambda);
        m_interp->inc_fun_scope_counter(0);
        for (size_t i = 0; i < args.size(); ++i)
        {
            m_interp->get_env().define(m_l->m_params[i], args[i]);
        }

        FunScope fc2(m_interp->get_env(), Environment::ScopeType::Capture);
        m_interp->get_env().m_data.back() = move(m_capture);
        // for (const auto &[k, v] : m_capture)
        // {
        //     m_interp->get_env().define(Token(TokenType::Identifier, k, 0, 0), v);
        // }

        try
        {
            m_interp->execute(m_l->m_body);
        }
        catch (const ReturnSignal &rs)
        {
            m_capture = move(m_interp->get_env().m_data.back());
            m_interp->dec_fun_scope_counter();
            return rs.m_res;
        }
        m_capture = move(m_interp->get_env().m_data.back());
        m_interp->dec_fun_scope_counter();

        return nullptr;
    }

    int arity() const override
    {
        return m_l->m_params.size();
    }

    string to_str() const override
    {
        return "<lambda>(" + to_string(arity()) + ")";
    }
};

struct Class : ClassBase
{
    Interpreter *m_interp = nullptr;
    ClassStmt *m_cst = nullptr;
    unordered_map<string, Function *> m_methods;

    Object *call(const std::vector<Object *> &args) override
    {
        auto my = GC::instance().new_object(ObjectType::Object);
        my->m_type = this;

        for (auto &f : m_cst->m_fields)
        {
            my->m_fields.emplace(f, nullptr);
        }

        auto it = m_methods.find("_init_");

        if (it == m_methods.end())
        {
            return my;
        }

        Function *init = it->second;

        FunScope fc(m_interp->get_env(), Environment::ScopeType::Fun); // fun _init_
        m_interp->inc_fun_scope_counter(init->m_fst->m_name.m_line);

        m_interp->get_env().define(Token(TokenType::Var, "my", 0, 0), my);

        for (size_t i = 0; i < args.size(); ++i)
        {
            m_interp->get_env().define(init->m_fst->m_params[i], args[i]);
        }

        try
        {
            m_interp->execute(init->m_fst->m_body);
        }
        catch (const ReturnSignal &rs)
        {
            m_interp->dec_fun_scope_counter();
            return my;
        }

        m_interp->dec_fun_scope_counter();
        return my;
    }

    Object *call_method(Object *my, const std::string &name, const std::vector<Object *> &args) override
    {
        auto it = m_methods.find(name);

        if (it == m_methods.end())
        {
            throw runtime_error("Execution error\nname '" + name + "' is not defined");
        }

        Function *method = it->second;

        if (method->arity() != int(args.size()))
        {
            throw runtime_error("Execution error\n<call expression> incorrect number of arguments for '" + method->to_str() + "'");
        }

        FunScope fc(m_interp->get_env(), Environment::ScopeType::Fun);
        m_interp->inc_fun_scope_counter(method->m_fst->m_name.m_line);

        m_interp->get_env().define(Token(TokenType::Var, "my", 0, 0), my);

        for (size_t i = 0; i < args.size(); ++i)
        {
            m_interp->get_env().define(method->m_fst->m_params[i], args[i]);
        }

        try
        {
            m_interp->execute(method->m_fst->m_body);
        }
        catch (const ReturnSignal &rs)
        {
            m_interp->dec_fun_scope_counter();
            return rs.m_res;
        }

        m_interp->dec_fun_scope_counter();
        return nullptr;
    }

    int arity() const override
    {
        auto it = m_methods.find("_init_");
        if (it == m_methods.end())
        {
            return 0;
        }

        return it->second->arity();
    }

    string to_str() const override
    {
        string res = "<class " + m_cst->m_name.m_lexeme + ">:";
        vector<string> vs;
        for (const auto &[name, fn] : m_methods)
        {
            vs.push_back(name);
        }

        sort(vs.begin(), vs.end());

        bool first = true;
        for (const auto &name : vs)
        {
            res += first ? "\n" : "";
            res += name;
        }

        return res;
    }
};

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

struct Print : Callable
{
    Interpreter *interp = nullptr;

    Object *call(const std::vector<Object *> &args) override
    {
        interp->get_out() << (!args.front() ? "null"s : args.front()->to_str());
        return nullptr;
    }

    int arity() const override
    {
        return 1;
    }

    string to_str() const override
    {
        return "print";
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

struct ToFloat : Callable
{
    Object *call(const std::vector<Object *> &args) override
    {
        Object *res = GC::instance().new_object(ObjectType::Float);
        dynamic_cast<Float *>(res)->m_val = stod(args.front()->to_str());
        return res;
    }

    int arity() const override
    {
        return 1;
    }

    string to_str() const override
    {
        return "to_float";
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

struct GetRecursionDepth : Callable
{
    Interpreter *interp = nullptr;

    Object *call([[maybe_unused]] const std::vector<Object *> &args) override
    {
        interp->get_out() << interp->get_recursion_depth() << endl;
        return nullptr;
    }

    int arity() const override
    {
        return 0;
    }

    string to_str() const override
    {
        return "get_recursion_depth";
    }
};

struct SetRecursionDepth : Callable
{
    Interpreter *interp = nullptr;

    Object *call(const std::vector<Object *> &args) override
    {
        if (auto depth = dynamic_cast<Int *>(args.front()))
        {
            if (depth->m_val > 0)
            {
                interp->set_recursion_depth(depth->m_val);
                return nullptr;
            }

            throw runtime_error("Execution error\n<native fun> invalid depth value in 'set_recursion_depth'");
        }

        throw runtime_error("Execution error\n<native fun> invalid argument type in 'set_recursion_depth'");
    }

    int arity() const override
    {
        return 1;
    }

    string to_str() const override
    {
        return "set_recursion_depth";
    }
};

Interpreter::Interpreter(istream &in, ostream &out)
    : m_in(in), m_out(out), m_fun_scope_counter(0), m_max_fun_depth(1024)
{
    m_env.add_scope(Environment::ScopeType::Global);

    PrintLine *pl = static_cast<PrintLine *>(GC::instance().new_object<PrintLine>());
    pl->interp = this;
    m_env.define(Token(TokenType::Var, "println", 0, 0), pl);

    Print *p = static_cast<Print *>(GC::instance().new_object<Print>());
    p->interp = this;
    m_env.define(Token(TokenType::Var, "print", 0, 0), p);

    ReadLine *rl = static_cast<ReadLine *>(GC::instance().new_object<ReadLine>());
    rl->interp = this;
    m_env.define(Token(TokenType::Var, "readln", 0, 0), rl);

    GetRecursionDepth *grd = static_cast<GetRecursionDepth *>(GC::instance().new_object<GetRecursionDepth>());
    grd->interp = this;
    m_env.define(Token(TokenType::Var, "get_recursion_depth", 0, 0), grd);

    SetRecursionDepth *srd = static_cast<SetRecursionDepth *>(GC::instance().new_object<SetRecursionDepth>());
    srd->interp = this;
    m_env.define(Token(TokenType::Var, "set_recursion_depth", 0, 0), srd);

    m_env.define(Token(TokenType::Var, "to_int", 0, 0), GC::instance().new_object<ToInt>());
    m_env.define(Token(TokenType::Var, "to_float", 0, 0), GC::instance().new_object<ToFloat>());
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
        throw runtime_error("Execution error\nline " + to_string(e->m_token.m_line) + ": <binary expression> incorrect operand types for '" + e->m_token.m_lexeme + "' operator");
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
        throw runtime_error("Execution error\nline " + to_string(e->m_token.m_line) + ": <binary expression> incorrect operand types for '" + e->m_token.m_lexeme + "' operator");
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
        throw runtime_error("Execution error\nline " + to_string(e->m_token.m_line) + ": <binary expression> incorrect operand types for '" + e->m_token.m_lexeme + "' operator");
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
        throw runtime_error("Execution error\nline " + to_string(e->m_token.m_line) + ": <binary expression> incorrect operand types for '" + e->m_token.m_lexeme + "' operator");
    case TokenType::Mod:
        if (Object *res = bin_op<Int, ObjectType::Int>(o1, o2, modulus<long long>()))
        {
            return res;
        }
        throw runtime_error("Execution error\nline " + to_string(e->m_token.m_line) + ": <binary expression> incorrect operand types for '" + e->m_token.m_lexeme + "' operator");
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
        throw runtime_error("Execution error\nline " + to_string(e->m_token.m_line) + ": <binary expression> incorrect operand types for '" + e->m_token.m_lexeme + "' operator");
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
        throw runtime_error("Execution error\nline " + to_string(e->m_token.m_line) + ": <binary expression> incorrect operand types for '" + e->m_token.m_lexeme + "' operator");
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
        throw runtime_error("Execution error\nline " + to_string(e->m_token.m_line) + ": <binary expression> incorrect operand types for '" + e->m_token.m_lexeme + "' operator");
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
        throw runtime_error("Execution error\nline " + to_string(e->m_token.m_line) + ": <binary expression> incorrect operand types for '" + e->m_token.m_lexeme + "' operator");
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
        throw runtime_error("Execution error\nline " + to_string(e->m_token.m_line) + ": <binary expression> unknown operator '" + e->m_token.m_lexeme + "'");
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
        throw runtime_error("Execution error\nline " + to_string(e->m_token.m_line) + ": <unary expression> incorrect operand type for '" + e->m_token.m_lexeme + "' operator");

    default:
        throw runtime_error("Execution error\nline " + to_string(e->m_token.m_line) + ": <unary expression> incorrect operand type for '" + e->m_token.m_lexeme + "' operator");
    }
}

Object *Interpreter::visit_call_expr(Call *e)
{
    if (auto p = dynamic_cast<Dot *>(e->m_expr))
    {
        Object *o = evaluate(p->m_expr);

        vector<Object *> args;

        for (auto arg : e->m_args)
        {
            args.push_back(evaluate(arg));
        }

        return o->call_method(p->m_name.m_lexeme, args);
    }

    Object *o = evaluate(e->m_expr);

    Callable *c = dynamic_cast<Callable *>(o);

    if (!c)
    {
        throw runtime_error("Execution error\n<call expression> '" + o->to_str() + "' is not a function or lambda");
    }

    if (c->arity() != int(e->m_args.size()))
    {
        throw runtime_error("Execution error\n<call expression> incorrect number of arguments for '" + c->to_str() + "'");
    }

    vector<Object *> args;

    for (auto arg : e->m_args)
    {
        args.push_back(evaluate(arg));
    }

    return c->call(args);
}

Object *Interpreter::visit_dot_expr(Dot *e)
{
    Object *o = evaluate(e->m_expr);
    return o->get_field(e->m_name.m_lexeme);
}

Object *Interpreter::visit_subscript_expr(Subscript *e)
{
    Object *expr = evaluate(e->m_expr);

    if (auto s = dynamic_cast<String *>(expr))
    {
        return s->get(evaluate(e->m_index));
    }

    return nullptr;
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
        throw runtime_error("Execution error\nline " + to_string(e->m_token.m_line) + ": <literal> unknown literal '" + e->m_token.m_lexeme + "'");
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

Object *Interpreter::visit_lambda(Lambda *e)
{
    LambdaFunction *lf = static_cast<LambdaFunction *>(GC::instance().new_object<LambdaFunction>());
    lf->m_interp = this;
    lf->m_l = e;
    for (auto t : e->m_capture)
    {
        lf->m_capture.emplace(t.m_lexeme, m_env.get(t));
    }
    return lf;
}

void Interpreter::visit_var_stmt(VarStmt *e)
{
    m_env.define(e->m_token, e->m_expr ? evaluate(e->m_expr) : nullptr);
}

void Interpreter::visit_assignment_stmt(AssignmentStmt *e)
{
    if (auto p = dynamic_cast<Var *>(e->m_lval))
    {
        m_env.assign(p->m_token, evaluate(e->m_expr));
        return;
    }
    if (auto p2 = dynamic_cast<Dot *>(e->m_lval))
    {
        Object *o = evaluate(p2->m_expr);
        o->set_field(p2->m_name.m_lexeme, evaluate(e->m_expr));
        return;
    }
    if (auto p3 = dynamic_cast<Subscript *>(e->m_lval))
    {
        Object *o = evaluate(p3->m_expr);
        if (auto a = dynamic_cast<Indexable *>(o))
        {
            a->set(evaluate(p3->m_index), o);
        }
        // o->set_field(p2->m_name.m_lexeme, evaluate(e->m_expr));
        return;
    }

    throw runtime_error("Execution error\n<assignment statement> can be used only with variables or object fields");
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
            Scope s(m_env, Environment::ScopeType::If);
            execute(e->m_then_branches[i]);
            return;
        }
    }

    if (!e->m_else_branch.empty())
    {
        Scope s(m_env, Environment::ScopeType::If);
        execute(e->m_else_branch);
    }
}

void Interpreter::visit_while_stmt(WhileStmt *e)
{
    try
    {
        while (is_true(evaluate(e->m_cond)))
        {
            try
            {
                Scope s(m_env, Environment::ScopeType::While);
                execute(e->m_do_branch);
            }
            catch (ContinueSignal)
            {
                // empty
            }
        }
    }
    catch (BreakSignal)
    {
        // empty
    }
}

void Interpreter::visit_break_stmt([[maybe_unused]] BreakStmt *e)
{
    throw BreakSignal();
}

void Interpreter::visit_continue_stmt([[maybe_unused]] ContinueStmt *e)
{
    throw ContinueSignal();
}

void Interpreter::visit_fun_stmt(FunStmt *e)
{
    Function *fn = static_cast<Function *>(GC::instance().new_object<Function>());
    fn->m_interp = this;
    fn->m_fst = e;
    m_env.define(Token(TokenType::Var, fn->m_fst->m_name.m_lexeme, 0, 0), fn);
}

void Interpreter::visit_return_stmt(ReturnStmt *e)
{
    Object *r = e->m_expr == nullptr ? nullptr : evaluate(e->m_expr);
    throw ReturnSignal(r);
}

void Interpreter::visit_class_stmt(ClassStmt *e)
{
    Class *cl = static_cast<Class *>(GC::instance().new_object<Class>());
    cl->m_interp = this;
    cl->m_cst = e;

    for (const auto &f : e->m_methods)
    {
        Function *fn = static_cast<Function *>(GC::instance().new_object<Function>());
        fn->m_interp = this;
        fn->m_fst = f.get();
        if (cl->m_methods.find(fn->m_fst->m_name.m_lexeme) != cl->m_methods.end())
        {
            throw runtime_error("Execution error\nline " + to_string(fn->m_fst->m_name.m_line) + ": <class statement> duplicate method '" + fn->m_fst->m_name.m_lexeme + "' in class '" + cl->m_cst->m_name.m_lexeme + "'");
        }
        cl->m_methods.emplace(fn->m_fst->m_name.m_lexeme, fn);
    }

    m_env.define(Token(TokenType::Var, cl->m_cst->m_name.m_lexeme, 0, 0), cl);
}
