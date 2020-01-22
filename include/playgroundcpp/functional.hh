#ifndef PLAYGROUNDCPP_FUNCTIONAL_HH__
#define PLAYGROUNDCPP_FUNCTIONAL_HH__
#endif

namespace playgroundcpp {

namespace functional {
    
template<class F>
struct flip {
    F func_;
    
    constexpr flip() : func_{} {}
    constexpr flip(const F& func) : func_{func} {}
    constexpr flip(F&& func) : func_{func} {}

    template<class T1, class T2>
    constexpr auto operator()(T1&& v1, T2&& v2) -> decltype(func_(std::forward<T2>(v2), std::forward<T1>(v1))) {
        return func_(std::forward<T2>(v2), std::forward<T1>(v1));
    }
};
template<class F>
constexpr flip<F> do_flip(F) {
    return flip<F>{};
}

struct eq {
    template<class T1, class T2>
    constexpr bool operator()(T1&& v1, T2&& v2) {
        return v1 == v2;
    }
};

struct ne {
    template<class T1, class T2>
    constexpr bool operator()(T1&& v1, T2&& v2) {
        return v1 != v2;
    }
};

struct gt {
    template<class T1, class T2>
    constexpr bool operator()(T1&& v1, T2&& v2) {
        return v1 > v2;
    }
};

struct ge {
    template<class T1, class T2>
    constexpr bool operator()(T1&& v1, T2&& v2) {
        return v1 >= v2;
    }
};

struct lt {
    template<class T1, class T2>
    constexpr bool operator()(T1&& v1, T2&& v2) {
        return v1 < v2;
    }
};

struct le {
    template<class T1, class T2>
    constexpr bool operator()(T1&& v1, T2&& v2) {
        return v1 <= v2;
    }
};

struct add {
    template<class T1, class T2>
    constexpr auto operator()(T1&& v1, T2&& v2) -> decltype(v1 + v2) {
        return v1 + v2;
    }
};

struct adde {
    template<class T1, class T2>
    constexpr auto operator()(T1&& v1, T2&& v2) -> decltype(v1 += v2) {
        return v1 += v2;
    }
};

struct sub {
    template<class T1, class T2>
    constexpr auto operator()(T1&& v1, T2&& v2) -> decltype(v1 - v2) {
        return v1 - v2;
    }
};

struct sube {
    template<class T1, class T2>
    constexpr auto operator()(T1&& v1, T2&& v2) -> decltype(v1 -= v2) {
        return v1 -= v2;
    }
};

struct mul {
    template<class T1, class T2>
    constexpr auto operator()(T1&& v1, T2&& v2) -> decltype(v1 * v2) {
        return v1 * v2;
    }
};

struct mule {
    template<class T1, class T2>
    constexpr auto operator()(T1&& v1, T2&& v2) -> decltype(v1 *= v2) {
        return v1 *= v2;
    }
};

} // functional

} // playgroundcpp
