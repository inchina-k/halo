#pragma once

#include <unordered_map>
#include <string>

#include "object.hpp"
#include "token.hpp"

namespace halo
{
    struct Environment
    {
        std::unordered_map<std::string, Object *> m_data;

        void define(Token t, Object *o);
        void assign(Token t, Object *o);
        Object *get(Token t) const;
    };
}