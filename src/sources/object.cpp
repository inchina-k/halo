#include "object.hpp"

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
