#pragma once

#include <unordered_map>
#include <vector>
#include <string>

#include "object.hpp"
#include "token.hpp"

namespace halo
{
    struct Environment
    {
        std::vector<std::unordered_map<std::string, Object *>> m_data;

        void define(Token t, Object *o);
        void assign(Token t, Object *o);
        Object *get(Token t);
        std::pair<std::unordered_map<std::string, Object *>::iterator, bool> lookup(Token t);
        void add_scope();
        void remove_scope();
    };
}