#include "Interpreter.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <cassert>

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

struct StackTmpManager
{
    size_t m_curr_size;

    StackTmpManager(size_t curr_size)
        : m_curr_size(curr_size)
    {
    }

    ~StackTmpManager()
    {
        GC::instance().get_interp()->clear_tmp_stack_from(m_curr_size);
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
    string m_class_name;

    Object *call(const std::vector<Object *> &args) override
    {
        FunScope fc(m_interp->get_env(), Environment::ScopeType::Fun);
        m_interp->inc_fun_scope_counter();

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

    string debug_info() const override
    {
        return "fun " + m_fst->m_name.m_lexeme;
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

    Object *call(const std::vector<Object *> &args) override
    {
        FunScope fc(m_interp->get_env(), Environment::ScopeType::Lambda);
        m_interp->inc_fun_scope_counter();
        for (size_t i = 0; i < args.size(); ++i)
        {
            m_interp->get_env().define(m_l->m_params[i], args[i]);
        }

        FunScope fc2(m_interp->get_env(), Environment::ScopeType::Capture);
        m_interp->get_env().m_data.back() = move(m_capture);

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

    string debug_info() const override
    {
        return "lambda";
    }

    string to_str() const override
    {
        return "<lambda>(" + to_string(arity()) + ")";
    }

    void mark() override
    {
        m_marked = true;

        for (auto &[str, obj] : m_capture)
        {
            if (obj && !obj->m_marked)
            {
                obj->mark();
            }
        }
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
        m_interp->inc_fun_scope_counter();

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
        Function *method = it->second;

        FunScope fc(m_interp->get_env(), Environment::ScopeType::Fun);
        m_interp->inc_fun_scope_counter();

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

    void check_method(const std::string &name, const std::vector<Object *> &args) override
    {
        auto it = m_methods.find(name);

        if (it == m_methods.end())
        {
            throw runtime_error(m_interp->report_error("name '" + name + "' is not defined"));
        }

        Function *method = it->second;

        if (method->arity() != int(args.size()))
        {
            throw runtime_error(m_interp->report_error("incorrect number of arguments for '" + method->to_str() + "'"));
        }
    }

    std::string get_name() const override
    {
        return m_cst->m_name.m_lexeme;
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

    string debug_info() const override
    {
        return "method " + m_cst->m_name.m_lexeme + "._init_";
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

    void mark() override
    {
        m_marked = true;

        for (auto &[str, obj] : m_methods)
        {
            if (obj && !obj->m_marked)
            {
                obj->mark();
            }
        }
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

    string debug_info() const override
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

    string debug_info() const override
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
        GC::instance().get_interp()->get_tmp_vals().push_back(res);
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

    string debug_info() const override
    {
        return "readln";
    }
};

struct ToInt : Callable
{
    Object *call(const std::vector<Object *> &args) override
    {
        Object *res = GC::instance().new_object(ObjectType::Int);
        GC::instance().get_interp()->get_tmp_vals().push_back(res);

        try
        {
            dynamic_cast<Int *>(res)->m_val = stoi(args.front()->to_str());
        }
        catch (const std::exception &)
        {
            throw runtime_error(GC::instance().get_interp()->report_error("invalid argument type in fun 'to_int'"));
        }

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

    string debug_info() const override
    {
        return "to_int";
    }
};

struct ToFloat : Callable
{
    Object *call(const std::vector<Object *> &args) override
    {
        Object *res = GC::instance().new_object(ObjectType::Float);
        GC::instance().get_interp()->get_tmp_vals().push_back(res);

        try
        {
            dynamic_cast<Float *>(res)->m_val = stod(args.front()->to_str());
        }
        catch (const std::exception &)
        {
            throw runtime_error(GC::instance().get_interp()->report_error("invalid argument type in fun 'to_float'"));
        }

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

    string debug_info() const override
    {
        return "to_float";
    }
};

struct ToStr : Callable
{
    Object *call(const std::vector<Object *> &args) override
    {
        Object *res = GC::instance().new_object(ObjectType::String);
        GC::instance().get_interp()->get_tmp_vals().push_back(res);

        try
        {
            dynamic_cast<String *>(res)->m_val = args.front()->to_str();
        }
        catch (const std::exception &)
        {
            throw runtime_error(GC::instance().get_interp()->report_error("invalid argument type in fun 'to_str'"));
        }

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

    string debug_info() const override
    {
        return "to_str";
    }
};

struct GetRecursionDepth : Callable
{
    Interpreter *m_interp = nullptr;

    Object *call([[maybe_unused]] const std::vector<Object *> &args) override
    {
        m_interp->get_out() << m_interp->get_recursion_depth() << endl;
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

    string debug_info() const override
    {
        return "get_recursion_depth";
    }
};

struct SetRecursionDepth : Callable
{
    Interpreter *m_interp = nullptr;

    Object *call(const std::vector<Object *> &args) override
    {
        if (auto depth = dynamic_cast<Int *>(args.front()))
        {
            if (depth->m_val > 0)
            {
                m_interp->set_recursion_depth(depth->m_val);
                return nullptr;
            }

            throw runtime_error(m_interp->report_error("invalid depth value in 'set_recursion_depth'"));
        }

        throw runtime_error(m_interp->report_error("invalid argument type in 'set_recursion_depth'"));
    }

    int arity() const override
    {
        return 1;
    }

    string to_str() const override
    {
        return "set_recursion_depth";
    }

    string debug_info() const override
    {
        return "set_recursion_depth";
    }
};

struct PrintGCInfo : Callable
{
    Interpreter *m_interp = nullptr;

    Object *call([[maybe_unused]] const std::vector<Object *> &args) override
    {
        m_interp->get_out() << "Objects in GC: " << GC::instance().count() << endl;
        m_interp->get_out() << "Threshold: " << GC::instance().get_treshold() << endl;
        // cerr << "Objects in GC: " << GC::instance().count() << endl;
        // cerr << "Threshold: " << GC::instance().get_treshold() << endl;
        return nullptr;
    }

    int arity() const override
    {
        return 0;
    }

    string to_str() const override
    {
        return "print_gc_info";
    }

    string debug_info() const override
    {
        return "print_gc_info";
    }
};

struct GCCollect : Callable
{
    Object *call([[maybe_unused]] const std::vector<Object *> &args) override
    {
        GC::instance().collect();
        return nullptr;
    }

    int arity() const override
    {
        return 0;
    }

    string to_str() const override
    {
        return "gc_collect";
    }

    string debug_info() const override
    {
        return "gc_collect";
    }
};

Interpreter::Interpreter(istream &in, ostream &out)
    : m_env(this), m_in(in), m_out(out), m_fun_scope_counter(0), m_max_fun_depth(1024), m_script("cli")
{
    GC::instance().set_interp(this);

    m_env.add_scope(Environment::ScopeType::Global);

    PrintLine *pl = dynamic_cast<PrintLine *>(GC::instance().new_object<PrintLine>());
    pl->interp = this;
    m_env.define(Token(TokenType::Var, "println", 0, 0), pl);

    Print *p = dynamic_cast<Print *>(GC::instance().new_object<Print>());
    p->interp = this;
    m_env.define(Token(TokenType::Var, "print", 0, 0), p);

    ReadLine *rl = dynamic_cast<ReadLine *>(GC::instance().new_object<ReadLine>());
    rl->interp = this;
    m_env.define(Token(TokenType::Var, "readln", 0, 0), rl);

    GetRecursionDepth *grd = dynamic_cast<GetRecursionDepth *>(GC::instance().new_object<GetRecursionDepth>());
    grd->m_interp = this;
    m_env.define(Token(TokenType::Var, "get_recursion_depth", 0, 0), grd);

    SetRecursionDepth *srd = dynamic_cast<SetRecursionDepth *>(GC::instance().new_object<SetRecursionDepth>());
    srd->m_interp = this;
    m_env.define(Token(TokenType::Var, "set_recursion_depth", 0, 0), srd);

    PrintGCInfo *pgci = dynamic_cast<PrintGCInfo *>(GC::instance().new_object<PrintGCInfo>());
    pgci->m_interp = this;
    m_env.define(Token(TokenType::Var, "print_gc_info", 0, 0), pgci);

    m_env.define(Token(TokenType::Var, "to_int", 0, 0), GC::instance().new_object<ToInt>());
    m_env.define(Token(TokenType::Var, "to_float", 0, 0), GC::instance().new_object<ToFloat>());
    m_env.define(Token(TokenType::Var, "to_str", 0, 0), GC::instance().new_object<ToStr>());

    m_env.define(Token(TokenType::Var, "gc_collect", 0, 0), GC::instance().new_object<GCCollect>());
}

void Interpreter::interpret(Expr *e)
{
    Object *res = (e == nullptr) ? nullptr : evaluate(e);

    cout << (res != nullptr ? res->to_str() : "null") << endl;
}

Object *Interpreter::evaluate_whole_expr(Expr *e)
{
    StackTmpManager stm(m_tmp_vals.size());
    return evaluate(e);
}

Object *Interpreter::evaluate(Expr *e)
{
    Object *res = e->visit(this);
    m_tmp_vals.push_back(res);
    return res;
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
    DebugManager debug_manager(this);
    m_debug_info.back().m_line = e->m_line;
    m_debug_info.back().m_name = "binary expression";

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
        throw runtime_error(report_error("incorrect operand types for '" + e->m_token.m_lexeme + "' operator"));
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
        throw runtime_error(report_error("incorrect operand types for '" + e->m_token.m_lexeme + "' operator"));
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
        throw runtime_error(report_error("incorrect operand types for '" + e->m_token.m_lexeme + "' operator"));
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
        throw runtime_error(report_error("incorrect operand types for '" + e->m_token.m_lexeme + "' operator"));
    case TokenType::Mod:
        if (Object *res = bin_op<Int, ObjectType::Int>(o1, o2, modulus<long long>()))
        {
            return res;
        }
        throw runtime_error(report_error("incorrect operand types for '" + e->m_token.m_lexeme + "' operator"));
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
        throw runtime_error(report_error("incorrect operand types for '" + e->m_token.m_lexeme + "' operator"));
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
        throw runtime_error(report_error("incorrect operand types for '" + e->m_token.m_lexeme + "' operator"));
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
        throw runtime_error(report_error("incorrect operand types for '" + e->m_token.m_lexeme + "' operator"));
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
        throw runtime_error(report_error("incorrect operand types for '" + e->m_token.m_lexeme + "' operator"));
    case TokenType::EqualEqual:
    {
        Object *r = GC::instance().new_object(ObjectType::Bool);
        GC::instance().get_interp()->get_tmp_vals().push_back(r);
        static_cast<Bool *>(r)->m_val = equals(o1, o2);
        return r;
    }
    case TokenType::BangEqual:
    {
        Object *r = GC::instance().new_object(ObjectType::Bool);
        GC::instance().get_interp()->get_tmp_vals().push_back(r);
        static_cast<Bool *>(r)->m_val = !equals(o1, o2);
        return r;
    }
    default:
        throw runtime_error(report_error("unknown operator '" + e->m_token.m_lexeme + "'"));
    }
}

Object *Interpreter::visit_logical_expr(LogicalExpr *e)
{
    DebugManager debug_manager(this);
    m_debug_info.back().m_line = e->m_line;
    m_debug_info.back().m_name = "logical expression";

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
    DebugManager debug_manager(this);
    m_debug_info.back().m_line = e->m_line;
    m_debug_info.back().m_name = "unary expression";

    Object *o = evaluate(e->m_expr);

    switch (e->m_token.m_type)
    {
    case TokenType::Not:
    {
        Object *r = GC::instance().new_object(ObjectType::Bool);
        GC::instance().get_interp()->get_tmp_vals().push_back(r);
        static_cast<Bool *>(r)->m_val = !is_true(o);
        return r;
    }
    case TokenType::Minus:
        if (Int *i = dynamic_cast<Int *>(o))
        {
            Object *r = GC::instance().new_object(ObjectType::Int);
            GC::instance().get_interp()->get_tmp_vals().push_back(r);
            static_cast<Int *>(r)->m_val = -i->m_val;
            return r;
        }
        if (Float *f = dynamic_cast<Float *>(o))
        {
            Object *r = GC::instance().new_object(ObjectType::Float);
            GC::instance().get_interp()->get_tmp_vals().push_back(r);
            static_cast<Float *>(r)->m_val = -f->m_val;
            return r;
        }
        throw runtime_error(report_error("incorrect operand type for '" + e->m_token.m_lexeme + "' operator"));

    default:
        throw runtime_error(report_error("incorrect operand type for '" + e->m_token.m_lexeme + "' operator"));
    }
}

Object *Interpreter::visit_call_expr(Call *e)
{
    if (auto p = dynamic_cast<Dot *>(e->m_expr))
    {
        Object *o = evaluate(p->m_expr);
        m_tmp_vals.push_back(o);

        vector<Object *> args;

        for (auto arg : e->m_args)
        {
            Object *tmp = evaluate(arg);
            m_tmp_vals.push_back(tmp);
            args.push_back(tmp);
        }

        o->m_type->check_method(p->m_name.m_lexeme, args);

        DebugManager debug_manager(this);
        m_debug_info.back().m_line = e->m_line;
        m_debug_info.back().m_name = "call expression";
        m_debug_info.back().m_call_info = "method " + o->m_type->get_name() + "." + p->m_name.m_lexeme;

        Object *res = o->call_method(p->m_name.m_lexeme, args);
        m_tmp_vals.push_back(res);
        return res;
    }

    Object *o = evaluate(e->m_expr);
    m_tmp_vals.push_back(o);

    Callable *c = dynamic_cast<Callable *>(o);

    if (!c)
    {
        throw runtime_error(report_error("'" + o->to_str() + "' is not a function or lambda"));
    }

    if (c->arity() != int(e->m_args.size()))
    {
        throw runtime_error(report_error("incorrect number of arguments for '" + c->to_str() + "'"));
    }

    vector<Object *> args;

    for (auto arg : e->m_args)
    {
        Object *tmp = evaluate(arg);
        m_tmp_vals.push_back(tmp);
        args.push_back(tmp);
    }

    DebugManager debug_manager(this);
    m_debug_info.back().m_line = e->m_line;
    m_debug_info.back().m_name = "call expression";
    m_debug_info.back().m_call_info = c->debug_info();

    Object *res = c->call(args);
    m_tmp_vals.push_back(res);
    return res;
}

Object *Interpreter::visit_dot_expr(Dot *e)
{
    DebugManager debug_manager(this);
    m_debug_info.back().m_line = e->m_line;
    m_debug_info.back().m_name = "dot expression";

    Object *o = evaluate(e->m_expr);
    return o->get_field(e->m_name.m_lexeme);
}

Object *Interpreter::visit_subscript_expr(Subscript *e)
{
    DebugManager debug_manager(this);
    m_debug_info.back().m_line = e->m_line;
    m_debug_info.back().m_name = "subscript expression";

    Object *expr = evaluate(e->m_expr);
    m_tmp_vals.push_back(expr);

    if (auto s = dynamic_cast<String *>(expr))
    {
        Object *o = evaluate(e->m_index);
        m_tmp_vals.push_back(o);
        return s->get(o);
    }
    if (auto l = dynamic_cast<List *>(expr))
    {
        Object *o = evaluate(e->m_index);
        m_tmp_vals.push_back(o);
        return l->get(o);
    }

    return nullptr;
}

Object *Interpreter::visit_literal(Literal *e)
{
    DebugManager debug_manager(this);
    m_debug_info.back().m_line = e->m_line;
    m_debug_info.back().m_name = "literal";

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
        o->m_eternal = true;
        static_cast<Int *>(o)->m_val = stoi(e->m_token.m_lexeme);
        e->m_val = o;
        return o;
    }
    case TokenType::FloatLiteral:
    {
        Object *o = GC::instance().new_object(ObjectType::Float);
        o->m_eternal = true;
        static_cast<Float *>(o)->m_val = stod(e->m_token.m_lexeme);
        e->m_val = o;
        return o;
    }
    case TokenType::True:
    case TokenType::False:
    {
        Object *o = GC::instance().new_object(ObjectType::Bool);
        o->m_eternal = true;
        static_cast<Bool *>(o)->m_val = e->m_token.m_type == TokenType::True;
        e->m_val = o;
        return o;
    }
    case TokenType::StrLiteral:
    {
        Object *o = GC::instance().new_object(ObjectType::String);
        o->m_eternal = true;
        dynamic_cast<String *>(o)->m_val = e->m_token.m_lexeme;
        e->m_val = o;
        return o;
    }
    default:
        throw runtime_error(report_error("unknown literal '" + e->m_token.m_lexeme + "'"));
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
    DebugManager debug_manager(this);
    m_debug_info.back().m_line = e->m_line;
    m_debug_info.back().m_name = "variable";

    return m_env.get(e->m_token);
}

Object *Interpreter::visit_lambda(Lambda *e)
{
    DebugManager debug_manager(this);
    m_debug_info.back().m_line = e->m_line;
    m_debug_info.back().m_name = "lambda";

    LambdaFunction *lf = dynamic_cast<LambdaFunction *>(GC::instance().new_object<LambdaFunction>());
    m_tmp_vals.push_back(lf);
    lf->m_interp = this;
    lf->m_l = e;
    for (auto t : e->m_capture)
    {
        lf->m_capture.emplace(t.m_lexeme, m_env.get(t));
    }
    return lf;
}

Object *Interpreter::visit_list(ListExpr *e)
{
    DebugManager debug_manager(this);
    m_debug_info.back().m_line = e->m_line;
    m_debug_info.back().m_name = "list";

    vector<Object *> vals;
    for (auto el : e->m_params)
    {
        Object *v = evaluate(el);
        v->m_eternal = true;
        vals.push_back(v);
    }

    Object *o = GC::instance().new_object(ObjectType::List);
    m_tmp_vals.push_back(o);
    dynamic_cast<List *>(o)->m_vals = vals;

    for (auto el : dynamic_cast<List *>(o)->m_vals)
    {
        el->m_eternal = false;
    }

    return o;
}

void Interpreter::visit_var_stmt(VarStmt *e)
{
    DebugManager debug_manager(this);
    m_debug_info.back().m_line = e->m_line;
    m_debug_info.back().m_name = "var statement";

    m_env.define(e->m_token, e->m_expr ? evaluate_whole_expr(e->m_expr) : nullptr);
}

void Interpreter::visit_assignment_stmt(AssignmentStmt *e)
{
    DebugManager debug_manager(this);
    m_debug_info.back().m_line = e->m_line;
    m_debug_info.back().m_name = "assignment statement";

    if (auto p = dynamic_cast<Var *>(e->m_lval))
    {
        m_env.assign(p->m_token, evaluate_whole_expr(e->m_expr));
        return;
    }
    if (auto p2 = dynamic_cast<Dot *>(e->m_lval))
    {
        Object *o = evaluate_whole_expr(p2->m_expr);
        o->m_eternal = true;
        o->set_field(p2->m_name.m_lexeme, evaluate_whole_expr(e->m_expr));
        o->m_eternal = false;
        return;
    }
    if (auto p3 = dynamic_cast<Subscript *>(e->m_lval))
    {
        Object *o = evaluate_whole_expr(p3->m_expr);
        o->m_eternal = true;
        if (auto a = dynamic_cast<Indexable *>(o))
        {
            a->set(evaluate_whole_expr(p3->m_index), evaluate_whole_expr(e->m_expr));
        }
        o->m_eternal = false;
        return;
    }

    throw runtime_error(report_error("can be used only with variables or object fields"));
}

void Interpreter::visit_expression_stmt(ExpressionStmt *e)
{
    DebugManager debug_manager(this);
    m_debug_info.back().m_line = e->m_line;
    m_debug_info.back().m_name = "expression statement";

    evaluate_whole_expr(e->m_expr);
}

void Interpreter::visit_if_stmt(IfStmt *e)
{
    DebugManager debug_manager(this);
    m_debug_info.back().m_line = e->m_line;
    m_debug_info.back().m_name = "if statement";

    for (size_t i = 0; i < e->m_conds.size(); ++i)
    {
        Object *o = evaluate_whole_expr(e->m_conds[i]);

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
    DebugManager debug_manager(this);
    m_debug_info.back().m_line = e->m_line;
    m_debug_info.back().m_name = "while statement";

    try
    {
        while (is_true(evaluate_whole_expr(e->m_cond)))
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

void Interpreter::visit_for_stmt(ForStmt *e)
{
    DebugManager debug_manager(this);
    m_debug_info.back().m_line = e->m_line;
    m_debug_info.back().m_name = "for statement";

    try
    {
        Scope hs(m_env, Environment::ScopeType::ForHeader);

        if (e->m_begin)
        {
            Object *begin_obj = evaluate_whole_expr(e->m_begin);
            begin_obj->m_eternal = true;
            Object *end_obj = evaluate_whole_expr(e->m_end);
            end_obj->m_eternal = true;
            Object *step_obj = nullptr;

            if (e->m_step)
            {
                step_obj = evaluate_whole_expr(e->m_step);
            }
            else
            {
                step_obj = GC::instance().new_object(ObjectType::Int);
                dynamic_cast<Int *>(step_obj)->m_val = 1;
            }

            begin_obj->m_eternal = false;
            end_obj->m_eternal = false;

            Int *ibegin = dynamic_cast<Int *>(begin_obj);
            if (!ibegin)
            {
                throw runtime_error(report_error("first index in range must be an integer"));
            }
            Int *iend = dynamic_cast<Int *>(end_obj);
            if (!iend)
            {
                throw runtime_error(report_error("last index in range must be an integer"));
            }
            Int *istep = dynamic_cast<Int *>(step_obj);
            if (!istep)
            {
                throw runtime_error(report_error("step in range must be an integer"));
            }

            if (istep->m_val == 0)
            {
                throw runtime_error(report_error("step in range must not be 0"));
            }

            hs.m_env.define(Token(TokenType::Var, "__for_begin__", 0, 0), ibegin);
            hs.m_env.define(Token(TokenType::Var, "__for_end__", 0, 0), iend);
            hs.m_env.define(Token(TokenType::Var, "__for_step__", 0, 0), istep);

            for (long long i = ibegin->m_val; istep->m_val > 0 ? i < iend->m_val : i > iend->m_val; i += istep->m_val)
            {
                try
                {
                    Scope s(m_env, Environment::ScopeType::For);
                    Object *curr = GC::instance().new_object(ObjectType::Int);
                    dynamic_cast<Int *>(curr)->m_val = i;
                    s.m_env.define(e->m_identifier, curr);
                    execute(e->m_do_branch);
                }
                catch (ContinueSignal)
                {
                    // empty
                }
            }
        }
        else if (e->m_iterable)
        {
            Object *iterable = evaluate_whole_expr(e->m_iterable);
            iterable->m_eternal = true;
            Object *it = nullptr;

            try
            {
                it = iterable->call_method("_iter_", vector<Object *>());
            }
            catch (const std::exception &)
            {
                iterable->m_eternal = false;
                throw runtime_error(report_error("uniterable object"));
            }

            iterable->m_eternal = false;

            hs.m_env.define(Token(TokenType::Var, "__for_iterable__", 0, 0), iterable);
            hs.m_env.define(Token(TokenType::Var, "__for_it__", 0, 0), it);

            try
            {
                Bool *has_next = dynamic_cast<Bool *>(it->call_method("_has_next_", vector<Object *>()));

                if (!has_next)
                {
                    throw runtime_error("");
                }

                while (has_next->m_val)
                {
                    Object *el = it->call_method("_next_", vector<Object *>());

                    try
                    {
                        Scope s(m_env, Environment::ScopeType::For);
                        s.m_env.define(e->m_identifier, el);
                        execute(e->m_do_branch);
                    }
                    catch (ContinueSignal)
                    {
                        // empty
                    }

                    has_next = dynamic_cast<Bool *>(it->call_method("_has_next_", vector<Object *>()));
                }
            }
            catch (const std::exception &)
            {
                throw runtime_error(report_error("invalid iterator"));
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
    DebugManager debug_manager(this);
    m_debug_info.back().m_line = e->m_line;
    m_debug_info.back().m_name = "break statement";

    throw BreakSignal();
}

void Interpreter::visit_continue_stmt([[maybe_unused]] ContinueStmt *e)
{
    DebugManager debug_manager(this);
    m_debug_info.back().m_line = e->m_line;
    m_debug_info.back().m_name = "continue statement";

    throw ContinueSignal();
}

void Interpreter::visit_fun_stmt(FunStmt *e)
{
    DebugManager debug_manager(this);
    m_debug_info.back().m_line = e->m_line;
    m_debug_info.back().m_name = "fun statement";

    Function *fn = dynamic_cast<Function *>(GC::instance().new_object<Function>());
    fn->m_interp = this;
    fn->m_fst = e;
    m_env.define(Token(TokenType::Var, fn->m_fst->m_name.m_lexeme, 0, 0), fn);
}

void Interpreter::visit_return_stmt(ReturnStmt *e)
{
    DebugManager debug_manager(this);
    m_debug_info.back().m_line = e->m_line;
    m_debug_info.back().m_name = "return statement";

    Object *r = e->m_expr == nullptr ? nullptr : evaluate_whole_expr(e->m_expr);
    throw ReturnSignal(r);
}

void Interpreter::visit_class_stmt(ClassStmt *e)
{
    DebugManager debug_manager(this);
    m_debug_info.back().m_line = e->m_line;
    m_debug_info.back().m_name = "class statement";

    Class *cl = dynamic_cast<Class *>(GC::instance().new_object<Class>());
    cl->m_interp = this;
    cl->m_cst = e;

    m_env.define(Token(TokenType::Var, cl->m_cst->m_name.m_lexeme, 0, 0), cl);

    for (const auto &f : e->m_methods)
    {
        Function *fn = dynamic_cast<Function *>(GC::instance().new_object<Function>());
        fn->m_interp = this;
        fn->m_fst = f.get();
        fn->m_class_name = e->m_name.m_lexeme;
        if (cl->m_methods.find(fn->m_fst->m_name.m_lexeme) != cl->m_methods.end())
        {
            throw runtime_error(report_error("duplicate method '" + fn->m_fst->m_name.m_lexeme + "' in class '" + cl->m_cst->m_name.m_lexeme + "'"));
        }
        cl->m_methods.emplace(fn->m_fst->m_name.m_lexeme, fn);
    }
}

size_t Interpreter::get_curr_error_line()
{
    return m_debug_info.empty() ? 0 : m_debug_info.back().m_line;
}

std::string Interpreter::get_curr_error_element()
{
    return m_debug_info.empty() ? "" : m_debug_info.back().m_name;
}

std::string Interpreter::report_error(std::string desc)
{
    ostringstream res;

    res << "Execution error\n";
    res << "    line " << get_curr_error_line() << ": <" << get_curr_error_element() << "> " << desc << "\n";

    res << "Call stack\n";

    for (auto i = m_debug_info.rbegin(); i != m_debug_info.rend(); ++i)
    {
        if (i->m_name == "call expression")
        {
            res << "    " << i->m_call_info << " (at line " << i->m_line << ")\n";
        }
    }

    res << "    script: " << m_script;

    return res.str();
}
