#ifndef PLAYGROUNDCPP_STEPS_HH__
#define PLAYGROUNDCPP_STEPS_HH__
#include <cstddef>
#include <utility>

namespace playgroundcpp {

    struct times {
        std::size_t begin_;
        std::size_t end_;
        std::size_t step_size_;

        constexpr explicit times(std::size_t end) : begin_{0}, end_{end}, step_size_{1} {} 
        constexpr times(std::size_t begin, std::size_t end, std::size_t step_size) : begin_{begin}, end_{end}, step_size_ {step_size} {}

        template<typename F>
        constexpr int do_(F&& f, int b, int e, int s) const {
            int i[] = {(f(b), b < e ? do_(std::forward<F>(f), b+s, e, s) : 0 )};
            return i[0];
        }

        template<typename F>
        constexpr void execute(F&& f) const {
            do_(std::forward<F>(f), begin_, end_, step_size_);
//            for(std::size_t i = begin_; i < end_; i += step_size_)
//                f(i);
        }

        template<typename F>
        constexpr void operator()(F&& f) const {
            execute(std::forward<F>(f));
        }
    };

    constexpr times operator""_times(unsigned long long end) {
        return times(end);
    }
}
#endif
