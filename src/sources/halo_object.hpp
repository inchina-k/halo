#pragma once

#include <string>

namespace halo
{
    class Object
    {
    public:
        virtual ~Object()
        {
        }

        virtual std::string to_str() const = 0;
    };

    struct Int : Object
    {
        long long m_val;

        Int()
            : m_val(0)
        {
        }

        std::string to_str() const override;
    };

    struct Float : Object
    {
        double m_val;

        Float()
            : m_val(0)
        {
        }

        std::string to_str() const override;
    };

    struct String : Object
    {
        std::string m_val;

        std::string to_str() const override;
    };

    struct Bool : Object
    {
        bool m_val;

        Bool()
            : m_val(0)
        {
        }

        std::string to_str() const override;
    };
}