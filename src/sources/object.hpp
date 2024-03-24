#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <map>

namespace halo
{
    struct Object
    {
        std::map<std::string, Object *> m_fields;

        Object()
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

        void set_field(std::string name, Object *val)
        {
            auto it = m_fields.find(name);

            if (it == m_fields.end())
            {
                throw std::runtime_error("Execution error\nfield '" + name + "' is not found");
            }

            it->second = val;
        }

        virtual bool equals(Object *other) const
        {
            return this == other;
        }
    };

    struct Int : Object
    {
        long long m_val;

        Int()
            : m_val(0)
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
            : m_val(0)
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
            : m_val(0)
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

    struct String : Object
    {
        std::string m_val;

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

    struct Null : Object
    {
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