#ifndef OPERATIONAL_SEMANTICS_HAS_EQUALITY_H
#define OPERATIONAL_SEMANTICS_HAS_EQUALITY_H

#include<type_traits>

namespace CHECK /// https://stackoverflow.com/a/6536204
{
    struct No {};
    template<typename T, typename Arg> No operator== (const T&, const Arg&);

    template<typename T, typename Arg = T>
    struct EqualExists
    {
        enum { value = !std::is_same<decltype(*(T*)(0) == *(Arg*)(0)), No>::value };
    };
}

#endif //OPERATIONAL_SEMANTICS_HAS_EQUALITY_H
