#include "gc.hpp"
#include "interpreter.hpp"

using namespace halo;
using namespace std;

void GC::collect()
{
    m_interp->get_env().mark();

    for (auto it = m_objects.begin(); it != m_objects.end();)
    {
        if ((*it)->m_marked || (*it)->m_eternal)
        {
            (*it)->m_marked = false;
            ++it;
        }
        else
        {
            delete *it;
            it = m_objects.erase(it);
        }
    }
}