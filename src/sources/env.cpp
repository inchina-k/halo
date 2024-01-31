#include "env.hpp"

using namespace std;
using namespace halo;

void Environment::define(Token t, Object *o)
{
    if (m_data.back().find(t.m_lexeme) != m_data.back().end())
    {
        throw runtime_error("variable is defined already");
    }

    m_data.back()[t.m_lexeme] = o;
}

void Environment::assign(Token t, Object *o)
{
    auto it = m_data.back().find(t.m_lexeme);

    if (it == m_data.back().end())
    {
        throw runtime_error("var is not defined");
    }

    it->second = o;
}

Object *Environment::get(Token t) const
{
    auto it = m_data.back().find(t.m_lexeme);

    if (it == m_data.back().end())
    {
        throw runtime_error("var is not found");
    }

    return it->second;
}

void Environment::add_scope()
{
    m_data.emplace_back();
}

void Environment::remove_scope()
{
    m_data.pop_back();
}