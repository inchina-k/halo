#include "object.hpp"
#include "gc.hpp"
#include "token_type.hpp"
#include "interpreter.hpp"
#include <string>

using namespace std;
using namespace halo;

string Object::to_str() const
{
    std::string res = m_type->get_name() + "[";
    bool first = true;

    for (auto field : m_fields)
    {
        res += first ? "" : ", ";
        res += field.first;
        res += "=";
        res += field.second ? field.second->to_str() : "null";
        first = false;
    }

    res += "]";

    return res;
}

string Int::to_str() const
{
    return to_string(m_val);
}

void Int::mark()
{
    m_marked = true;
}

string Float::to_str() const
{
    return to_string(m_val);
}

void Float::mark()
{
    m_marked = true;
}

string Bool::to_str() const
{
    return m_val ? "true" : "false";
}

void Bool::mark()
{
    m_marked = true;
}

string String::to_str() const
{
    return m_val;
}

void String::mark()
{
    m_marked = true;
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

void List::mark()
{
    m_marked = true;

    for (auto val : m_vals)
    {
        if (val && !val->m_marked)
        {
            val->mark();
        }
    }
}

string Null::to_str() const
{
    return "null";
}

void Null::mark()
{
    m_marked = true;
}

void StringIter::mark()
{
    m_marked = true;
}

void ListIter::mark()
{
    m_marked = true;
}

/* Object */

void Object::set_field(const std::string &name, Object *val)
{
    auto it = m_fields.find(name);

    if (it == m_fields.end())
    {
        throw std::runtime_error(GC::instance().get_interp()->report_error("field '" + name + "' is not defined"));
    }

    it->second = val;
}

Object *Object::get_field(const std::string &name)
{
    auto it = m_fields.find(name);

    if (it == m_fields.end())
    {
        throw std::runtime_error(GC::instance().get_interp()->report_error("field '" + name + "' is not defined"));
    }

    return it->second;
}

Object *Object::call_method(const std::string &name, const std::vector<Object *> &args)
{
    return m_type->call_method(this, name, args);
}

void Object::mark()
{
    m_marked = true;

    for (auto &[str, obj] : m_fields)
    {
        if (obj && !obj->m_marked)
        {
            obj->mark();
        }
    }
}

/* Callable */

Object *Callable::call([[maybe_unused]] const std::vector<Object *> &args)
{
    throw std::runtime_error(GC::instance().get_interp()->report_error("call is not implemented"));
}

int Callable::arity() const
{
    throw std::runtime_error(GC::instance().get_interp()->report_error("arity is not implemented"));
}

string Callable::debug_info() const
{
    throw std::runtime_error(GC::instance().get_interp()->report_error("debug info is not implemented"));
}

/* String */

Object *String::get(Object *index)
{
    if (auto i = dynamic_cast<Int *>(index))
    {
        if (i->m_val < 0 || i->m_val > int(m_val.size() - 1))
        {
            throw runtime_error(GC::instance().get_interp()->report_error("invalid index in " + get_name()));
        }

        Object *o = GC::instance().new_object(ObjectType::String);
        dynamic_cast<String *>(o)->m_val = m_val[i->m_val];
        return o;
    }

    throw runtime_error(GC::instance().get_interp()->report_error("invalid index value type in " + get_name()));
}

void String::set(Object *, Object *)
{
    throw std::runtime_error(GC::instance().get_interp()->report_error("set operation is not available for type " + get_name()));
}

Object *String::iter(Object *my)
{
    auto str = dynamic_cast<String *>(my);

    Object *res = GC::instance().new_object(ObjectType::StringIter);
    dynamic_cast<StringIter *>(res)->m_beg = str->m_val.begin();
    dynamic_cast<StringIter *>(res)->m_end = str->m_val.end();
    return res;
}

void String::check_method(const std::string &name, const std::vector<Object *> &args)
{
    static unordered_map<string, size_t> methods = {{"substr", 2}, {"_iter_", 0}};

    auto it = methods.find(name);

    if (it == methods.end())
    {
        throw runtime_error(GC::instance().get_interp()->report_error("name '" + name + "' is not defined in class " + get_name()));
    }

    if (it->second != args.size())
    {
        throw runtime_error(GC::instance().get_interp()->report_error("invalid number of arguments in method '" + name + "' in class " + get_name()));
    }
}

Object *String::call_method(Object *my, const std::string &name, const std::vector<Object *> &args)
{
    if (name == "substr")
    {
        return substr(my, args);
    }
    else if (name == "_iter_")
    {
        return iter(my);
    }

    throw runtime_error(GC::instance().get_interp()->report_error("undefined method '" + name + "' in class " + get_name()));
}

Object *String::substr(Object *my, const std::vector<Object *> &args)
{
    auto arg1 = dynamic_cast<Int *>(args[0]);
    auto arg2 = dynamic_cast<Int *>(args[1]);

    if (!arg1 || !arg2)
    {
        throw runtime_error(GC::instance().get_interp()->report_error("invalid argument type in method 'substr' in class " + get_name()));
    }

    auto str = dynamic_cast<String *>(my);

    if (arg1->m_val < 0 || arg1->m_val > int(str->m_val.size() - 1))
    {
        throw runtime_error(GC::instance().get_interp()->report_error("invalid index in method 'substr' in class " + get_name()));
    }

    if (arg2->m_val < 0)
    {
        throw runtime_error(GC::instance().get_interp()->report_error("invalid range in method 'substr' in class " + get_name()));
    }

    Object *res = GC::instance().new_object(ObjectType::String);
    dynamic_cast<String *>(res)->m_val = str->m_val.substr(arg1->m_val, arg2->m_val);
    return res;
}

/* StringIter */

Object *StringIter::has_next(Object *my)
{
    auto str_iter = dynamic_cast<StringIter *>(my);

    Object *res = GC::instance().new_object(ObjectType::Bool);
    dynamic_cast<Bool *>(res)->m_val = str_iter->m_beg != str_iter->m_end;
    return res;
}

Object *StringIter::next(Object *my)
{
    auto str_iter = dynamic_cast<StringIter *>(my);

    Object *res = GC::instance().new_object(ObjectType::String);
    dynamic_cast<String *>(res)->m_val = *str_iter->m_beg;
    ++str_iter->m_beg;
    return res;
}

void StringIter::check_method(const std::string &name, const std::vector<Object *> &args)
{
    static unordered_map<string, size_t> methods = {{"_next_", 0}, {"_has_next_", 0}};

    auto it = methods.find(name);

    if (it == methods.end())
    {
        throw runtime_error(GC::instance().get_interp()->report_error("name '" + name + "' is not defined in class " + get_name()));
    }

    if (it->second != args.size())
    {
        throw runtime_error(GC::instance().get_interp()->report_error("invalid number of arguments in method '" + name + "' in class " + get_name()));
    }
}

Object *StringIter::call_method(Object *my, const std::string &name, [[maybe_unused]] const std::vector<Object *> &args)
{
    if (name == "_has_next_")
    {
        return has_next(my);
    }
    else if (name == "_next_")
    {
        return next(my);
    }

    throw runtime_error(GC::instance().get_interp()->report_error("undefined method '" + name + "' in class " + get_name()));
}

/* List */

Object *List::get(Object *index)
{
    if (auto i = dynamic_cast<Int *>(index))
    {
        if (i->m_val < 0 || i->m_val > int(m_vals.size() - 1))
        {
            throw runtime_error(GC::instance().get_interp()->report_error("invalid index in " + get_name()));
        }

        return m_vals[i->m_val];
    }

    throw runtime_error(GC::instance().get_interp()->report_error("invalid index value type in " + get_name()));
}

void List::set(Object *index, Object *val)
{
    if (auto i = dynamic_cast<Int *>(index))
    {
        if (i->m_val < 0 || i->m_val > int(m_vals.size() - 1))
        {
            throw runtime_error(GC::instance().get_interp()->report_error("invalid index in " + get_name()));
        }

        m_vals[i->m_val] = val;
        return;
    }

    throw runtime_error(GC::instance().get_interp()->report_error("invalid index value type in " + get_name()));
}

Object *List::iter(Object *my)
{
    auto list = dynamic_cast<List *>(my);

    Object *res = GC::instance().new_object(ObjectType::ListIter);
    dynamic_cast<ListIter *>(res)->m_beg = list->m_vals.begin();
    dynamic_cast<ListIter *>(res)->m_end = list->m_vals.end();
    return res;
}

void List::check_method(const std::string &name, const std::vector<Object *> &args)
{
    static unordered_map<string, size_t> methods = {{"put", 1}, {"pop", 0}, {"pop_at", 1}, {"pop_all", 1}, {"len", 0}, {"clear", 0}, {"_iter_", 0}};

    auto it = methods.find(name);

    if (it == methods.end())
    {
        throw runtime_error(GC::instance().get_interp()->report_error("name '" + name + "' is not defined in class " + get_name()));
    }

    if (it->second != args.size())
    {
        throw runtime_error(GC::instance().get_interp()->report_error("invalid number of arguments in method '" + name + "' in class " + get_name()));
    }
}

Object *List::call_method(Object *my, const std::string &name, const std::vector<Object *> &args)
{
    if (name == "put")
    {
        return put(my, args);
    }
    if (name == "pop")
    {
        return pop(my);
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
        return len(my);
    }
    if (name == "clear")
    {
        return clear(my);
    }
    if (name == "_iter_")
    {
        return iter(my);
    }

    throw runtime_error(GC::instance().get_interp()->report_error("undefined method '" + name + "' in class " + get_name()));
}

Object *List::put(Object *my, const std::vector<Object *> &args)
{
    auto list = dynamic_cast<List *>(my);
    list->m_vals.push_back(args[0]);

    return nullptr;
}

Object *List::pop(Object *my)
{
    auto list = dynamic_cast<List *>(my);

    if (list->m_vals.empty())
    {
        throw runtime_error(GC::instance().get_interp()->report_error("attempt to access an element in an empty container in 'pop'"));
    }

    list->m_vals.pop_back();

    return nullptr;
}

Object *List::pop_at(Object *my, const std::vector<Object *> &args)
{
    auto arg = dynamic_cast<Int *>(args[0]);

    if (!arg)
    {
        throw runtime_error(GC::instance().get_interp()->report_error("invalid argument type in 'pop_at'"));
    }

    auto list = dynamic_cast<List *>(my);

    if (list->m_vals.empty())
    {
        throw runtime_error(GC::instance().get_interp()->report_error("attempt to access an element in an empty container in 'pop_at'"));
    }

    if (arg->m_val < 0 || arg->m_val > int(list->m_vals.size() - 1))
    {
        throw runtime_error(GC::instance().get_interp()->report_error("invalid index in 'pop_at'"));
    }

    list->m_vals.erase(list->m_vals.begin() + arg->m_val);

    return nullptr;
}

Object *List::pop_all(Object *my, const std::vector<Object *> &args)
{
    auto list = dynamic_cast<List *>(my);
    list->m_vals.erase(std::remove_if(list->m_vals.begin(), list->m_vals.end(),
                                      [args](Object *x)
                                      { return x->equals(args[0]); }),
                       list->m_vals.end());

    return nullptr;
}

Object *List::len(Object *my)
{
    auto list = dynamic_cast<List *>(my);

    Object *o = GC::instance().new_object(ObjectType::Int);
    dynamic_cast<Int *>(o)->m_val = list->m_vals.size();
    return o;
}

Object *List::clear(Object *my)
{
    auto list = dynamic_cast<List *>(my);
    list->m_vals.clear();

    return nullptr;
}

/* ListIter */

Object *ListIter::has_next(Object *my)
{
    auto list_iter = dynamic_cast<ListIter *>(my);

    Object *res = GC::instance().new_object(ObjectType::Bool);
    dynamic_cast<Bool *>(res)->m_val = list_iter->m_beg != list_iter->m_end;
    return res;
}

Object *ListIter::next(Object *my)
{
    auto list_iter = dynamic_cast<ListIter *>(my);

    Object *res = *list_iter->m_beg;
    ++list_iter->m_beg;
    return res;
}

Object *ListIter::call_method(Object *my, const std::string &name, [[maybe_unused]] const std::vector<Object *> &args)
{
    if (name == "_has_next_")
    {
        return has_next(my);
    }
    if (name == "_next_")
    {
        return next(my);
    }

    throw runtime_error(GC::instance().get_interp()->report_error("undefined method '" + name + "' in class " + get_name()));
}

void ListIter::check_method(const std::string &name, const std::vector<Object *> &args)
{
    static unordered_map<string, size_t> methods = {{"_next_", 0}, {"_has_next_", 0}};

    auto it = methods.find(name);

    if (it == methods.end())
    {
        throw runtime_error(GC::instance().get_interp()->report_error("name '" + name + "' is not defined in class " + get_name()));
    }

    if (it->second != args.size())
    {
        throw runtime_error(GC::instance().get_interp()->report_error("invalid number of arguments in method '" + name + "' in class " + get_name()));
    }
}