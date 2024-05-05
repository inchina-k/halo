#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <map>
#include <iostream>

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

        void set_field(const std::string &name, Object *val);
        Object *get_field(const std::string &name);

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

    struct Indexable : virtual Object
    {
        Indexable()
            : Object(nullptr)
        {
        }

        virtual Object *get(Object *index) = 0;
        virtual void set(Object *index, Object *val) = 0;
    };

    struct Callable : virtual Object
    {
        Callable()
            : Object(nullptr)
        {
        }

        virtual Object *call([[maybe_unused]] const std::vector<Object *> &args);

        virtual int arity() const;

        virtual std::string debug_info() const;
    };

    struct ClassBase : Callable
    {
        virtual Object *call_method(Object *my, const std::string &name, const std::vector<Object *> &args) = 0;
        virtual void check_method(const std::string &name, const std::vector<Object *> &args) = 0;
        virtual std::string get_name() const = 0;
    };

    struct StringIter : ClassBase
    {
        std::string::const_iterator m_beg;
        std::string::const_iterator m_end;

        StringIter()
        {
            m_type = this;
        }

        Object *has_next(Object *my);
        Object *next(Object *my);

        Object *call_method(Object *my, const std::string &name, [[maybe_unused]] const std::vector<Object *> &args) override;
        void check_method(const std::string &name, const std::vector<Object *> &args) override;

        std::string get_name() const override
        {
            return "StringIter";
        }
    };

    struct String : Indexable, ClassBase
    {
        std::string m_val;

        String()
            : Indexable()
        {
            m_type = this;
        }

        Object *get(Object *index) override;
        void set(Object *, Object *) override;

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

        Object *iter(Object *my);

        Object *call_method(Object *my, const std::string &name, const std::vector<Object *> &args) override;
        void check_method(const std::string &name, const std::vector<Object *> &args) override;

        std::string get_name() const override
        {
            return "String";
        }

        Object *substr(Object *my, const std::vector<Object *> &args);
    };

    struct ListIter : ClassBase
    {
        std::vector<Object *>::const_iterator m_beg;
        std::vector<Object *>::const_iterator m_end;

        ListIter()
        {
            m_type = this;
        }

        Object *has_next(Object *my);
        Object *next(Object *my);

        Object *call_method(Object *my, const std::string &name, [[maybe_unused]] const std::vector<Object *> &args) override;
        void check_method(const std::string &name, const std::vector<Object *> &args) override;

        std::string get_name() const override
        {
            return "StringIter";
        }
    };

    struct List : Indexable, ClassBase
    {
        std::vector<Object *> m_vals;

        List()
            : Indexable()
        {
            m_type = this;
        }

        Object *get(Object *index) override;

        void set(Object *index, Object *val) override;

        std::string to_str() const override;

        bool equals(Object *other) const override
        {
            List *p = dynamic_cast<List *>(other);

            if (!p)
            {
                return false;
            }

            if (m_vals.size() != p->m_vals.size())
            {
                return false;
            }

            for (size_t i = 0; i < m_vals.size(); ++i)
            {
                if (!m_vals[i]->equals(p->m_vals[i]))
                {
                    return false;
                }
            }

            return true;
        }

        Object *iter(Object *my);

        Object *call_method(Object *my, const std::string &name, const std::vector<Object *> &args) override;
        void check_method(const std::string &name, const std::vector<Object *> &args) override;

        std::string get_name() const override
        {
            return "List";
        }

        Object *put(Object *my, const std::vector<Object *> &args);
        Object *pop(Object *my);
        Object *pop_at(Object *my, const std::vector<Object *> &args);
        Object *pop_all(Object *my, const std::vector<Object *> &args);
        Object *len(Object *my);
        Object *clear(Object *my);
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