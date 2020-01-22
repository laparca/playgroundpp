#ifndef GROUP_COMPARATOR_HH__
#define GROUP_COMPARATOR_HH__
#endif

#include <playgroundcpp/functional.hh>

#if __cplusplus >= 201703L
#include <tuple>
#endif

namespace playgroundcpp {

namespace group_comparator {
    
#if __cplusplus < 201103L
#error "Wrong C++ standard version. At least you need C++11"
#elif __cplusplus >= 201103L && __cplusplus < 201703L // [C++11 .. C++17)

template<class... Ts>
struct any_of;

template<class T>
struct any_of<T> {
    T value_;

    constexpr any_of(T&& v) : value_{v} {}

    template<class F, class T2>
    constexpr bool apply(F&& f, T2&& v) const {
        return f(value_, v);
    }
};

template<class T, class... Ts>
struct any_of<T, Ts...> {
    T value_;
    any_of<Ts...> rest_of_values_;

    constexpr any_of(T&& v, Ts&&... vs) : value_{v}, rest_of_values_{std::forward<Ts>(vs)...} {}
    
    template<class F, class T2>
    constexpr bool apply(F&& f, T2&& v) const {
        return f(value_, v) || rest_of_values_.apply(std::forward<F>(f), std::forward<T2>(v));
    }
};

template<class... Ts>
struct all_of;

template<class T>
struct all_of<T> {
    T value_;

    constexpr all_of(T&& v) : value_{v} {}

    template<class F, class T2>
    constexpr bool apply(F&& f, T2&& v) const {
        return f(value_, v);
    }
};

template<class T, class... Ts>
struct all_of<T, Ts...> {
    T value_;
    all_of<Ts...> rest_of_values_;

    constexpr all_of(T&& v, Ts&&... vs) : value_{v}, rest_of_values_{std::forward<Ts>(vs)...} {}

    template<class F, class T2>
    constexpr bool apply(F&& f, T2&& v) const {
        return f(value_, v) && rest_of_values_.apply(std::forward<F>(f), std::forward<T2>(v));
    }
};

#elif __cplusplus >= 201703L

template<class... Ts>
struct any_of {
    using types_t = std::tuple<Ts...>;
    types_t values_;

    constexpr any_of(Ts&&... vs) : values_{std::forward<Ts>(vs)...} {}

    template<class F, class T2>
    constexpr bool apply(F&& f, T2&& v) const {
        return apply_impl(
            std::forward<F>(f),
            std::forward<T2>(v),
            std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<types_t>>>{});
    }

private:
    template<class F, class T2, std::size_t... I>
    constexpr bool apply_impl(F&& f, T2&& v, std::index_sequence<I...>) const {
        return (f(std::get<I>(values_), v) || ...);
    }
};
template<class... Ts>
any_of(Ts&&...) -> any_of<Ts...>;

template<class... Ts>
struct all_of {
    using types_t = std::tuple<Ts...>;
    types_t values_;

    constexpr all_of(Ts&&... vs) : values_{std::forward<Ts>(vs)...} {}

    template<class F, class T2>
    constexpr bool apply(F&& f, T2&& v) const {
        return apply_impl(
            std::forward<F>(f),
            std::forward<T2>(v),
            std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<types_t>>>{});
    }

private:
    template<class F, class T2, std::size_t... I>
    constexpr bool apply_impl(F&& f, T2&& v, std::index_sequence<I...>) const {
        return (f(std::get<I>(values_), v) && ...);
    }
};
template<class... Ts>
all_of(Ts&&...) -> all_of<Ts...>;
#endif

#define DO_OPERATOR(type, op, func)                                       \
template<typename T, typename... Ts>                                      \
constexpr bool operator op(type<Ts...> a, const T& v) {                   \
    return a.apply(func{}, v);                                            \
}                                                                         \
template<typename T, typename... Ts>                                      \
constexpr bool operator op(const T& v, type<Ts...> a) {                   \
    return a.apply(functional::flip<func>{func{}}, v);                    \
}                                                                         \
template<typename T, typename... Ts>                                      \
constexpr bool operator op(type<Ts...> a, T&& v) {                        \
    return a.apply(func{}, std::forward<T>(v));                           \
}                                                                         \
template<typename T, typename... Ts>                                      \
constexpr bool operator op(T&& v, type<Ts...> a) {                        \
    return a.apply(functional::flip<func>{func{}}, std::forward<T>(v));   \
}                                                 

DO_OPERATOR(any_of, >, functional::gt);
DO_OPERATOR(any_of, <, functional::lt);
DO_OPERATOR(any_of, >=, functional::ge);
DO_OPERATOR(any_of, <=, functional::le);
DO_OPERATOR(any_of, ==, functional::eq);
DO_OPERATOR(any_of, !=, functional::ne);

DO_OPERATOR(all_of, >, functional::gt);
DO_OPERATOR(all_of, <, functional::lt);
DO_OPERATOR(all_of, >=, functional::ge);
DO_OPERATOR(all_of, <=, functional::le);
DO_OPERATOR(all_of, ==, functional::eq);
DO_OPERATOR(all_of, !=, functional::ne);

template<class... Ts>
#if __cplusplus > 201103L
[[deprecated("Not needed since C++14. You should use any_of{x, x, ...} form.")]]
#endif
constexpr any_of<Ts...> for_any_of(Ts&&... vs) {
    return any_of<Ts...>{std::forward<Ts>(vs)...};
}

template<class... Ts>
#if __cplusplus > 201103L
[[deprecated("Not needed since C++14. You should use all_of{x, x, ...} form.")]]
#endif
constexpr all_of<Ts...> for_all_of(Ts&&... vs) {
    return all_of<Ts...>{std::forward<Ts>(vs)...};
}

} // group_comparator

} // playgroundcpp
