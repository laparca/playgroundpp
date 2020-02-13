#include <map>
#include <functional>
#include <type_traits>

namespace callback {
    namespace {
        struct default_single_callback_tag {};

        template<class Tag, class Result, class... Args>
            struct single {
                using callback_type = std::function<Result(Args...)>;

                static callback_type callback_;
                static Result callback(Args... args) {
                    if (callback_)
                        return callback_(std::forward<Args>(args)...);
                }

                Result operator()(Args... args) {
                    return callback(std::forward<Args>(args)...);
                }
            };
        template<class Tag, class Result, class... Args>
            typename single<Tag, Result, Args...>::callback_type single<Tag, Result, Args...>::callback_;

        template<class Tag, class ResultCollector, template<class...>class Map, class Result, class... Args>
            struct single_broadcast {
                using callback_type = std::function<Result(Args...)>;

                static Result callback(Args... args) {
                    ResultCollector rc;
                    for (auto& c : callbacks_)
                        rc.add_result(c->second(std::forward<Args>(args)...));
                    return rc.pack();
                }

                Result operator()(Args... args) {
                    return callback(std::forward<Args>(args)...);
                }

                static std::size_t add(callback_type c) {
                    auto id = last_id++;
                    callbacks_.nemplace({id, c});
                    return id;
                }

                static void remove(std::size_t id) {
                    auto it = callbacks_.find(id);
                    if (it != callbacks_.end())
                        callbacks_.erase(it);
                }

                static Map<std::size_t, callback_type> callbacks_;
                static std::size_t last_id;
            };

        template<class Tag, class ResultCollector, template<class...>class Map, class Result, class... Args>
            Map<std::size_t, typename single_broadcast<Tag, ResultCollector, Map, Result, Args...>::callback_type> single_broadcast<Tag, ResultCollector, Map, Result, Args...>::callbacks_;

        template<class Tag, class ResultCollector, template<class...>class Map, class Result, class... Args>
            std::size_t single_broadcast<Tag, ResultCollector, Map, Result, Args...>::last_id = 0;


        template<template<class...>class Map, class ArgsToId, class DefaultReturn, class Result, class... Args>
            struct multiple_id {
                using callback_type = std::function<Result(Args...)>;
                using id_type = decltype(ArgsToId{}(Args{}...));

                static Result callback(Args... args) {
                    id_type callback_id = args_to_id_(std::forward<Args>(args)...);

                    auto c_it = callback_map_.find(callback_id);

                    if (c_it != callback_map_.end()) {
                        return c_it->second(std::forward<Args>(args)...);
                    }

                    return default_return_();
                }

                static void add(id_type id, callback_type c) {
                    callback_map_[id] = c;
                }

                static void remove(id_type id) {
                    auto elm = callback_map_.find(id);
                    if (elm != callback_map_.end())
                        callback_map_.erase(elm);
                }

                Result operator()(Args... args) {
                    return callback(std::forward<Args>(args)...);
                }

                private:
                static Map<id_type, callback_type> callback_map_;
                static ArgsToId args_to_id_;
                static DefaultReturn default_return_;
            };

        template<template<class...>class Map, class ArgsToId, class DefaultReturn, class Result, class... Args>
            Map<typename multiple_id<Map, ArgsToId, DefaultReturn, Result, Args...>::id_type, typename multiple_id<Map, ArgsToId, DefaultReturn, Result, Args...>::callback_type> multiple_id<Map, ArgsToId, DefaultReturn, Result, Args...>::callback_map_;

        template<template<class...>class Map, class ArgsToId, class DefaultReturn, class Result, class... Args>
            ArgsToId multiple_id<Map, ArgsToId, DefaultReturn, Result, Args...>::args_to_id_;

        template<template<class...>class Map, class ArgsToId, class DefaultReturn, class Result, class... Args>
            DefaultReturn multiple_id<Map, ArgsToId, DefaultReturn, Result, Args...>::default_return_;

    }

    template<class Result, class... Args>
        struct generate_for {
            template<class Tag = default_single_callback_tag>
                using single = single<Tag, Result, Args...>;

            template<class ResultCollector, class Tag = default_single_callback_tag, template<class...>class Map = std::map>
                using single_broadcast = single_broadcast<Tag, ResultCollector, Map, Result, Args...>;

            template<class ArgsToId, class DefaultReturn, template<class...>class Map = std::map>
                using multiple_id = multiple_id<Map, ArgsToId, DefaultReturn, Result, Args...>;
        };
} // namespace callbacks


struct id_func {
    template<typename T>
        constexpr T operator()(T&& t) { return t; }
};

struct default_return_void {
    void operator()() {}
};

extern "C" {
    void (*internal_callback_func)(int);
    void register_callback(void(*c)(int)) {
        internal_callback_func = c;
    }
    void dispatch(int v) {
        internal_callback_func(v);
    }
}

#include <iostream>
using my_callback_helper = callback::generate_for<void, int>::multiple_id<id_func, default_return_void>;

class TestCallback {
    private:
        int id_;

        void my_callback_receptor(int id) {
            std::cout << "TestCallback::my_callback_receptor(" << id << "); // id = " << id_ << std::endl;
        }

    public:
        TestCallback(int id = 0) : id_{id} {
            my_callback_helper::add(id_, std::bind(&TestCallback::my_callback_receptor, this, std::placeholders::_1));
        }

        ~TestCallback() {
            my_callback_helper::remove(id_);
        }
};

void dispatch_a_lot(int b, int e) {
    for(int i = b; i <= e; ++i)
        dispatch(i);
}

int main() {
    register_callback(my_callback_helper::callback);
    {
        TestCallback tc{1};

        my_callback_helper::add(0, [](int a) { std::cout << "Received the value " << a << " in lambda 1" << std::endl; });
        my_callback_helper::add(2, [](int a) { std::cout << "Received the value " << a << " in lambda 2" << std::endl; });

        dispatch_a_lot(0, 10);
    }

    std::cout << "TestCall is destroyed" << std::endl;
    dispatch_a_lot(0, 10);

    return 0;
}
