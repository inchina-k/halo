#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <map>

namespace halo
{
    struct ClassBase;

    struct Object
    {
        ClassBase *m_type;
        std::map<std::string, Object *> m_fields;

        Object(ClassBase *type = nullptr)
            : m_type(type)
        {
        }

        virtual ~Object()
        {
        }

        virtual std::string to_str() const
        {
            std::string res = "Object[";
            bool first = true;

            for (auto field : m_fields)
            {
                res += first ? "" : ", ";
                res += field.first;
                res += "=";
                res += field.second ? field.second->to_str() : "null";
                first = false;
            }

            res += "]";

            return res;
        }

        void set_field(const std::string &name, Object *val)
        {
            auto it = m_fields.find(name);

            if (it == m_fields.end())
            {
                throw std::runtime_error("Execution error\nfield '" + name + "' is not defined");
            }

            it->second = val;
        }

        Object *get_field(const std::string &name)
        {
            auto it = m_fields.find(name);

            if (it == m_fields.end())
            {
                throw std::runtime_error("Execution error\nfield '" + name + "' is not defined");
            }

            return it->second;
        }

        Object *call_method(const std::string &name, const std::vector<Object *> &args);

        virtual bool equals(Object *other) const
        {
            return this == other;
        }
    };

    struct Int : Object
    {
        long long m_val;

        Int()
            : Object(nullptr), m_val(0)
        {
        }

        std::string to_str() const override;

        bool equals(Object *other) const override
        {
            Int *p = dynamic_cast<Int *>(other);

            if (!p)
            {
                return false;
            }

            return m_val == p->m_val;
        }
    };

    struct Float : Object
    {
        double m_val;

        Float()
            : Object(nullptr), m_val(0)
        {
        }

        std::string to_str() const override;

        bool equals(Object *other) const override
        {
            Float *p = dynamic_cast<Float *>(other);

            if (!p)
            {
                return false;
            }

            return m_val == p->m_val;
        }
    };

    struct Bool : Object
    {
        bool m_val;

        Bool()
            : Object(nullptr), m_val(0)
        {
        }

        std::string to_str() const override;

        bool equals(Object *other) const override
        {
            Bool *p = dynamic_cast<Bool *>(other);

            if (!p)
            {
                return false;
            }

            return m_val == p->m_val;
        }
    };

    struct Indexable : Object
    {
        Indexable()
            : Object(nullptr)
        {
        }

        virtual Object *get(Object *index) = 0;
        virtual void set(Object *index, Object *val) = 0;
    };

    struct String : Indexable
    {
        std::string m_val;

        String()
            : Indexable()
        {
        }

        Object *get(Object *index) override;

        void set(Object *, Object *) override
        {
            throw std::runtime_error("set operation is not available for type string");
        }

        std::string to_str() const override;

        bool equals(Object *other) const override
        {
            String *p = dynamic_cast<String *>(other);

            if (!p)
            {
                return false;
            }

            return m_val == p->m_val;
        }
    };

    struct Callable : Object
    {
        Callable()
            : Object(nullptr)
        {
        }

        virtual Object *call([[maybe_unused]] const std::vector<Object *> &args)
        {
            throw std::runtime_error("call not implemented");
        }

        virtual int arity() const
        {
            throw std::runtime_error("arity not implemented");
        }
    };

    struct ClassBase : Callable
    {
        virtual Object *call_method(Object *my, const std::string &name, const std::vector<Object *> &args) = 0;
    };

    struct Null : Object
    {
        Null()
            : Object(nullptr)
        {
        }

        std::string to_str() const override;

        bool equals(Object *other) const override
        {
            Null *p = dynamic_cast<Null *>(other);

            if (p)
            {
                return true;
            }

            return false;
        }
    };
}