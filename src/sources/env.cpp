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
    auto p = lookup(t);

    if (!p.second)
    {
        throw runtime_error(t.m_lexeme + " var is not defined");
    }

    p.first->second = o;
}

Object *Environment::get(Token t)
{
    auto p = lookup(t);

    if (!p.second)
    {
        throw runtime_error(t.m_lexeme + " name is not found");
    }

    return p.first->second;
}

std::pair<std::unordered_map<std::string, Object *>::iterator, bool> Environment::lookup(Token t)
{
    for (auto it = m_data.rbegin(); it != m_data.rend(); ++it)
    {
        auto it2 = it->find(t.m_lexeme);

        if (it2 != it->end())
        {
            return make_pair(it2, true);
        }
    }

    return make_pair(std::unordered_map<std::string, Object *>::iterator(), false);
}

void Environment::add_scope()
{
    m_data.emplace_back();
}

void Environment::remove_scope()
{
    m_data.pop_back();
}

void Environment::swap_env(Environment &other)
{
    m_data.swap(other.m_data);
}
