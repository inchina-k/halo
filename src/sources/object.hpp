#pragma once

#include <string>
#include <vector>
#include <stdexcept>

namespace halo
{
    class Object
    {
    public:
        Object()
        {
        }

        virtual ~Object()
        {
        }

        virtual std::string to_str() const
        {
            return "Object";
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