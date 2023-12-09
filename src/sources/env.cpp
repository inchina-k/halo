#include "env.hpp"

using namespace std;
using namespace halo;

void Environment::define(Token t, Object *o)
{
    if (m_data.find(t.m_lexeme) != m_data.end())
    {
        throw runtime_error("variable is defined already");
    }

    m_data[t.m_lexeme] = o;
}

void Environment::assign(Token t, Object *o)
{
    auto it = m_data.find(t.m_lexeme);

    if (it == m_data.end())
    {
        throw runtime_error("var is not defined");
    }

    it->second = o;
}

Object *Environment::get(Token t) const
{
    auto it = m_data.find(t.m_lexeme);

    if (it == m_data.end())
    {
        throw runtime_error("var is not found");
    }

    return it->second;
}
