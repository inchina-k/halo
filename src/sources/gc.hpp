#pragma once

#include <list>

#include "object.hpp"

namespace halo
{
    class Interpreter;

    enum class ObjectType
    {
        Object,
        Int,
        Float,
        Bool,
        String,
        StringIter,
        Callable,
        List,
        ListIter,
        Null
    };

    class GC
    {
        std::list<Object *> m_objects;
        Interpreter *m_interp;

        GC()
        {
        }

    public:
        static GC &instance()
        {
            static GC inst;
            return inst;
        }

        void set_interp(Interpreter *interp)
        {
            m_interp = interp;
        }

        Interpreter *get_interp()
        {
            return m_interp;
        }

        ~GC()
        {
            for (auto e : m_objects)
            {
                delete e;
            }
        };

        template <typename T>
        Object *new_object()
        {
            m_objects.push_back(new T());
            return m_objects.back();
        }

        Object *new_object(ObjectType t, Object *o = nullptr)
        {
            switch (t)
            {
            case ObjectType::Object:
                m_objects.push_back(new Object());
                return m_objects.back();
            case ObjectType::Int:
                m_objects.push_back(new Int());
                return m_objects.back();
            case ObjectType::Float:
                m_objects.push_back(new Float());
                return m_objects.back();
            case ObjectType::Bool:
                m_objects.push_back(new Bool());
                return m_objects.back();
            case ObjectType::String:
                m_objects.push_back(new String());
                return m_objects.back();
            case ObjectType::StringIter:
                m_objects.push_back(new StringIter());
                return m_objects.back();
            case ObjectType::List:
                m_objects.push_back(new List());
                return m_objects.back();
            case ObjectType::ListIter:
                m_objects.push_back(new ListIter());
                return m_objects.back();
            case ObjectType::Callable:
                m_objects.push_back(o);
                return m_objects.back();
            case ObjectType::Null:
                m_objects.push_back(new Null());
                return m_objects.back();
            default:
                return nullptr;
            }
        }
    };
}
