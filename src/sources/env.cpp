#include "env.hpp"

using namespace std;
using namespace halo;

void Environment::define(Token t, Object *o)
{
    if (m_data.back().find(t.m_lexeme) != m_data.back().end())
    {
        throw runtime_error("Execution error\nline " + to_string(t.m_line) + ": <environment> name '" + t.m_lexeme + "' is defined already");
    }

    m_data.back()[t.m_lexeme] = o;
}

void Environment::assign(Token t, Object *o)
{
    auto p = lookup(t);

    if (!p.second)
    {
        throw runtime_error("Execution error\nline " + to_string(t.m_line) + ": <environment> name '" + t.m_lexeme + "' is not defined");
    }

    p.first->second = o;
}

Object *Environment::get(Token t)
{
    auto p = lookup(t);

    if (!p.second)
    {
        throw runtime_error("Execution error\nline " + to_string(t.m_line) + ": <environment> name '" + t.m_lexeme + "' is not defined");
    }

    return p.first->second;
}

std::pair<std::unordered_map<std::string, Object *>::iterator, bool> Environment::lookup(Token t)
{
    size_t i = m_scopes.size();

    while (i != 1)
    {
        --i;

        auto it = m_data[i].find(t.m_lexeme);

        if (it != m_data[i].end())
        {
            return make_pair(it, true);
        }

        if (m_scopes[i] == ScopeType::Fun || m_scopes[i] == ScopeType::Lambda)
        {
            break;
        }
    }

    auto it = m_data[0].find(t.m_lexeme);

    if (it != m_data[0].end())
    {
        return make_pair(it, true);
    }

    return make_pair(std::unordered_map<std::string, Object *>::iterator(), false);
}

void Environment::add_scope(ScopeType st)
{
    m_data.emplace_back();
    m_scopes.push_back(st);
}

void Environment::remove_scope()
{
    m_data.pop_back();
    m_scopes.pop_back();
}

void Environment::swap_env(Environment &other)
{
    m_data.swap(other.m_data);
    m_scopes.swap(other.m_scopes);
}
