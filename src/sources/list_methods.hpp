#include "object.hpp"
#include "gc.hpp"
#include "token_type.hpp"
#include <string>
#include <algorithm>

using namespace std;
using namespace halo;

Object *put(Object *my, const std::vector<Object *> &args)
{
    if (args.size() != 1)
    {
        throw runtime_error("invalid number of arguments in 'put'");
    }

    auto list = dynamic_cast<List *>(my);
    list->m_vals.push_back(args[0]);

    return nullptr;
}

Object *pop(Object *my, const std::vector<Object *> &args)
{
    if (args.size() != 0)
    {
        throw runtime_error("invalid number of arguments in 'pop'");
    }

    auto list = dynamic_cast<List *>(my);

    if (list->m_vals.empty())
    {
        throw runtime_error("attempt to access an element in an empty container in 'pop'");
    }

    list->m_vals.pop_back();

    return nullptr;
}

Object *pop_at(Object *my, const std::vector<Object *> &args)
{
    if (args.size() != 1)
    {
        throw runtime_error("invalid number of arguments in 'pop_at'");
    }

    auto arg = dynamic_cast<Int *>(args[0]);

    if (!arg)
    {
        throw runtime_error("invalid argument type in 'pop_at'");
    }

    auto list = dynamic_cast<List *>(my);

    if (list->m_vals.empty())
    {
        throw runtime_error("attempt to access an element in an empty container in 'pop_at'");
    }

    if (arg->m_val < 0 || arg->m_val > int(list->m_vals.size() - 1))
    {
        throw runtime_error("invalid index in 'pop_at'");
    }

    list->m_vals.erase(list->m_vals.begin() + arg->m_val);

    return nullptr;
}

Object *pop_all(Object *my, const std::vector<Object *> &args)
{
    if (args.size() != 1)
    {
        throw runtime_error("invalid number of arguments in 'pop_all'");
    }

    auto list = dynamic_cast<List *>(my);
    list->m_vals.erase(std::remove_if(list->m_vals.begin(), list->m_vals.end(),
                                      [args](Object *x)
                                      { return x->equals(args[0]); }),
                       list->m_vals.end());

    return nullptr;
}

Object *len(Object *my, const std::vector<Object *> &args)
{
    if (args.size() != 0)
    {
        throw runtime_error("invalid number of arguments in 'len'");
    }

    auto list = dynamic_cast<List *>(my);

    Object *o = GC::instance().new_object(ObjectType::Int);
    dynamic_cast<Int *>(o)->m_val = list->m_vals.size();
    return o;
}

Object *clear(Object *my, const std::vector<Object *> &args)
{
    if (args.size() != 0)
    {
        throw runtime_error("invalid number of arguments in 'clear'");
    }

    auto list = dynamic_cast<List *>(my);
    list->m_vals.clear();

    return nullptr;
}