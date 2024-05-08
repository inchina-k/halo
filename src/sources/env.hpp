#pragma once

#include <unordered_map>
#include <vector>
#include <string>

#include "object.hpp"
#include "token.hpp"

namespace halo
{
    class Interpreter;

    struct Environment
    {
        enum class ScopeType
        {
            Global,
            General,
            If,
            While,
            ForHeader,
            For,
            Fun,
            Init,
            Capture,
            Lambda,
            Class
        };

        std::vector<std::unordered_map<std::string, Object *>> m_data;
        std::vector<ScopeType> m_scopes;
        Interpreter *m_interp;

        Environment(Interpreter *interp)
            : m_interp(interp)
        {
        }

        void define(Token t, Object *o);
        void assign(Token t, Object *o);
        Object *get(Token t);
        std::pair<std::unordered_map<std::string, Object *>::iterator, bool> lookup(Token t);
        void add_scope(ScopeType st);
        void remove_scope();
        void swap_env(Environment &other);
        void mark();
    };
}