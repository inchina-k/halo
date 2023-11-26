#include "halo_object.hpp"

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

string String::to_str() const
{
    return m_val;
}

string Bool::to_str() const
{
    return m_val ? "true" : "false";
}
