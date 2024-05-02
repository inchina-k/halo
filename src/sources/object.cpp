#include "object.hpp"
#include "gc.hpp"
#include "token_type.hpp"
#include "list_methods.hpp"
#include "interpreter.hpp"
#include <string>

using namespace std;
using namespace halo;

string Int::to_str() const
{
    return to_string(m_val);
}

string Float::to_str() const
{
    return to_string(m_val);
}

string Bool::to_str() const
{
    return m_val ? "true" : "false";
}

string String::to_str() const
{
    return m_val;
}

string List::to_str() const
{
    string res = "[";
    bool first = true;

    for (auto val : m_vals)
    {
        res += first ? "" : ", ";
        res += val->to_str();
        first = false;
    }

    res += "]";

    return res;
}

string Null::to_str() const
{
    return "null";
}

Object *Object::call_method(const std::string &name, const std::vector<Object *> &args)
{
    return m_type->call_method(this, name, args);
}

Object *String::get(Object *index)
{
    if (auto i = dynamic_cast<Int *>(index))
    {
        if (i->m_val < 0 || i->m_val > int(m_val.size() - 1))
        {
            throw runtime_error(GC::instance().get_interp()->report_error("invalid index"));
        }

        Object *o = GC::instance().new_object(ObjectType::String);
        dynamic_cast<String *>(o)->m_val = m_val[i->m_val];
        return o;
    }

    throw runtime_error(GC::instance().get_interp()->report_error("invalid index value type"));
}

Object *String::iter(Object *my, const std::vector<Object *> &args)
{
    if (args.size() != 0)
    {
        throw runtime_error(GC::instance().get_interp()->report_error("invalid number of arguments in '_iter_'"));
    }

    auto str = dynamic_cast<String *>(my);

    Object *res = GC::instance().new_object(ObjectType::StringIter);
    dynamic_cast<StringIter *>(res)->m_beg = str->m_val.begin();
    dynamic_cast<StringIter *>(res)->m_end = str->m_val.end();
    return res;
}

Object *StringIter::has_next(Object *my, const std::vector<Object *> &args)
{
    if (args.size() != 0)
    {
        throw runtime_error(GC::instance().get_interp()->report_error("invalid number of arguments in '_has_next_'"));
    }

    auto str_iter = dynamic_cast<StringIter *>(my);

    Object *res = GC::instance().new_object(ObjectType::Bool);
    dynamic_cast<Bool *>(res)->m_val = str_iter->m_beg != str_iter->m_end;
    return res;
}

Object *StringIter::next(Object *my, const std::vector<Object *> &args)
{
    if (args.size() != 0)
    {
        throw runtime_error(GC::instance().get_interp()->report_error("invalid number of arguments in '_next_'"));
    }

    auto str_iter = dynamic_cast<StringIter *>(my);

    Object *res = GC::instance().new_object(ObjectType::String);
    dynamic_cast<String *>(res)->m_val = *str_iter->m_beg;
    ++str_iter->m_beg;
    return res;
}

Object *String::call_method(Object *my, const std::string &name, const std::vector<Object *> &args)
{
    if (name == "substr")
    {
        if (args.size() != 2)
        {
            throw runtime_error(GC::instance().get_interp()->report_error("invalid number of arguments in 'substr'"));
        }

        auto arg1 = dynamic_cast<Int *>(args[0]);
        auto arg2 = dynamic_cast<Int *>(args[1]);

        if (!arg1 || !arg2)
        {
            throw runtime_error(GC::instance().get_interp()->report_error("invalid argument type in 'substr'"));
        }

        auto str = dynamic_cast<String *>(my);

        if (arg1->m_val < 0 || arg1->m_val > int(str->m_val.size() - 1))
        {
            throw runtime_error(GC::instance().get_interp()->report_error("invalid index in 'substr'"));
        }

        if (arg2->m_val < 0)
        {
            throw runtime_error(GC::instance().get_interp()->report_error("invalid range in 'substr'"));
        }

        Object *res = GC::instance().new_object(ObjectType::String);
        dynamic_cast<String *>(res)->m_val = str->m_val.substr(arg1->m_val, arg2->m_val);
        return res;
    }
    else if (name == "_iter_")
    {
        return iter(my, args);
    }

    throw runtime_error(GC::instance().get_interp()->report_error("undefined method '" + name + "'"));
}

Object *StringIter::call_method(Object *my, const std::string &name, const std::vector<Object *> &args)
{
    if (name == "_has_next_")
    {
        return has_next(my, args);
    }
    else if (name == "_next_")
    {
        return next(my, args);
    }

    throw runtime_error(GC::instance().get_interp()->report_error("undefined method '" + name + "'"));
}

Object *List::get(Object *index)
{
    if (auto i = dynamic_cast<Int *>(index))
    {
        if (i->m_val < 0 || i->m_val > int(m_vals.size() - 1))
        {
            throw runtime_error(GC::instance().get_interp()->report_error("invalid index"));
        }

        return m_vals[i->m_val];
    }

    throw runtime_error(GC::instance().get_interp()->report_error("invalid index value type"));
}

void List::set(Object *index, Object *val)
{
    if (auto i = dynamic_cast<Int *>(index))
    {
        if (i->m_val < 0 || i->m_val > int(m_vals.size() - 1))
        {
            throw runtime_error(GC::instance().get_interp()->report_error("invalid index"));
        }

        m_vals[i->m_val] = val;
        return;
    }

    throw runtime_error(GC::instance().get_interp()->report_error("invalid index value type"));
}

Object *List::iter(Object *my, const std::vector<Object *> &args)
{
    if (args.size() != 0)
    {
        throw runtime_error(GC::instance().get_interp()->report_error("invalid number of arguments in '_iter_'"));
    }

    auto list = dynamic_cast<List *>(my);

    Object *res = GC::instance().new_object(ObjectType::ListIter);
    dynamic_cast<ListIter *>(res)->m_beg = list->m_vals.begin();
    dynamic_cast<ListIter *>(res)->m_end = list->m_vals.end();
    return res;
}

Object *ListIter::has_next(Object *my, const std::vector<Object *> &args)
{
    if (args.size() != 0)
    {
        throw runtime_error(GC::instance().get_interp()->report_error("invalid number of arguments in '_has_next_'"));
    }

    auto list_iter = dynamic_cast<ListIter *>(my);

    Object *res = GC::instance().new_object(ObjectType::Bool);
    dynamic_cast<Bool *>(res)->m_val = list_iter->m_beg != list_iter->m_end;
    return res;
}

Object *ListIter::next(Object *my, const std::vector<Object *> &args)
{
    if (args.size() != 0)
    {
        throw runtime_error(GC::instance().get_interp()->report_error("invalid number of arguments in '_next_'"));
    }

    auto list_iter = dynamic_cast<ListIter *>(my);

    Object *res = *list_iter->m_beg;
    ++list_iter->m_beg;
    return res;
}

Object *List::call_method(Object *my, const std::string &name, const std::vector<Object *> &args)
{
    if (name == "put")
    {
        return put(my, args);
    }
    if (name == "pop")
    {
        return pop(my, args);
    }
    if (name == "pop_at")
    {
        return pop_at(my, args);
    }
    if (name == "pop_all")
    {
        return pop_all(my, args);
    }
    if (name == "len")
    {
        return len(my, args);
    }
    if (name == "clear")
    {
        return clear(my, args);
    }
    if (name == "_iter_")
    {
        return iter(my, args);
    }

    throw runtime_error(GC::instance().get_interp()->report_error("undefined method '" + name + "'"));
}

Object *ListIter::call_method(Object *my, const std::string &name, const std::vector<Object *> &args)
{
    if (name == "_has_next_")
    {
        return has_next(my, args);
    }
    if (name == "_next_")
    {
        return next(my, args);
    }

    throw runtime_error(GC::instance().get_interp()->report_error("undefined method '" + name + "'"));
}
