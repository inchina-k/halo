#include "object.hpp"
#include "gc.hpp"
#include "token_type.hpp"

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

string Null::to_str() const
{
    return "null";
}

Object *Object::call_method(const std::string &name, const std::vector<Object *> &args)
{
    return m_type->call_method(this, name, args);
}

Object *String::call_method(Object *my, const std::string &name, const std::vector<Object *> &args)
{
    if (name == "substr")
    {
        if (args.size() != 2)
        {
            throw runtime_error("invalid number of arguments in 'substr'");
        }

        auto arg1 = dynamic_cast<Int *>(args[0]);
        auto arg2 = dynamic_cast<Int *>(args[1]);

        if (!arg1 || !arg2)
        {
            throw runtime_error("invalid argument type in 'substr'");
        }

        auto str = dynamic_cast<String *>(my);

        if (arg1->m_val < 0 || arg1->m_val > int(str->m_val.size() - 1))
        {
            throw runtime_error("invalid index in 'substr'");
        }

        if (arg2->m_val < 0)
        {
            throw runtime_error("invalid range in 'substr'");
        }

        Object *res = GC::instance().new_object(ObjectType::String);
        dynamic_cast<String *>(res)->m_val = str->m_val.substr(arg1->m_val, arg2->m_val);
        return res;
    }

    throw runtime_error("undefined method '" + name + "'");
}

Object *String::get(Object *index)
{
    if (auto i = dynamic_cast<Int *>(index))
    {
        if (i->m_val < 0 || i->m_val > int(m_val.size() - 1))
        {
            throw runtime_error("invalid index");
        }

        Object *o = GC::instance().new_object(ObjectType::String);
        dynamic_cast<String *>(o)->m_val = m_val[i->m_val];
        return o;
    }

    throw runtime_error("invalid index value type");
}

Object *List::get(Object *index)
{
    if (auto i = dynamic_cast<Int *>(index))
    {
        if (i->m_val < 0 || i->m_val > int(m_vals.size() - 1))
        {
            throw runtime_error("invalid index");
        }

        return m_vals[i->m_val];
    }

    throw runtime_error("invalid index value type");
}

void List::set(Object *index, Object *val)
{
    if (auto i = dynamic_cast<Int *>(index))
    {
        if (i->m_val < 0 || i->m_val > int(m_vals.size() - 1))
        {
            throw runtime_error("invalid index");
        }

        m_vals[i->m_val] = val;
    }

    throw runtime_error("invalid index value type");
}

string List::to_str() const
{
    std::string res = "List[";
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