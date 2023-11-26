#pragma once

#include <list>

#include "halo_object.hpp"

namespace halo
{
    enum class ObjectType
    {
        Object,
        Int,
        Float,
        String,
        Bool
    };

    class GC
    {
        std::list<Object *> m_objects;

    public:
        ~GC()
        {
            for (auto e : m_objects)
            {
                delete e;
            }
        };

        Object *new_object(ObjectType t)
        {
            switch (t)
            {
            case ObjectType::Int:
                m_objects.push_back(new Int());
                return m_objects.back();
            case ObjectType::Float:
                m_objects.push_back(new Float());
                return m_objects.back();
            case ObjectType::String:
                m_objects.push_back(new String());
                return m_objects.back();
            case ObjectType::Bool:
                m_objects.push_back(new Bool());
                return m_objects.back();
            default:
                return nullptr;
            }
        }
    };
}