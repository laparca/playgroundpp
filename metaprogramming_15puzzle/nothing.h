#ifndef NOTHING_H
#define NOTHING_H

struct Nothing {
    using type = Nothing;
};

template<class T>
struct is_nothing {
    static constexpr bool value = false;
};

template<>
struct is_nothing<Nothing> {
    static constexpr bool value = true;
};

template<class T>
constexpr bool is_nothing_v = is_nothing<T>::value;

#endif

