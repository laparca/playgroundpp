#include <coroutine>
#include <iostream>
#include <atomic>
#include <thread>
#include <mutex>
#include <deque>
#include <vector>
#include <condition_variable>
#include <concepts>
#include <functional>
#include <iterator>
#include <optional>
#include <chrono>
#include <sstream>
#include <ctime>
#include <iomanip>

std::string return_current_time_and_date()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%FT%T%z");
    return ss.str();
}

std::mutex __D_mut;
#define D(x) \
do { \
    std::stringstream ss; \
    ss << return_current_time_and_date() << " [th:" << std::this_thread::get_id() << "]: " << x << std::endl; \
    std::cout << ss.str(); \
} while(false)
//std::lock_guard<std::mutex> lck(__D_mut);

#if DEBUG
#define TRACE(sentence) D(__PRETTY_FUNCTION__ << ":" << __LINE__ << ": " << #sentence); sentence
#else
#define TRACE(sentence) sentence
#endif
struct promise;

template<typename T>
struct queue
{
    
    template<std::convertible_to<T> U>
    void push(U&& value)
    {
        TRACE(std::lock_guard<std::mutex> lck_{mut_});
        TRACE(values_.emplace_back(std::forward<U>(value)));
        TRACE(cv_.notify_one());
    }

    std::optional<T> pop()
    {
        TRACE(std::unique_lock<std::mutex> lck_{mut_});
        TRACE(cv_.wait(lck_, [this]{ return values_.size() > 0 || is_closed(); }));
        TRACE(if (is_closed()))
        {
            TRACE(return {});
        }

        TRACE(auto value = values_.front());
        TRACE(values_.pop_front());
        TRACE(return value);
    }

    void close()
    {
        TRACE(closed_.store(true));
        TRACE(cv_.notify_all());
    }

    bool is_closed() {
        TRACE(return closed_.load());
    }
    std::atomic<bool> closed_{false};
    std::deque<T> values_;
    std::mutex mut_;
    std::condition_variable cv_;
};

using work_queue = queue<std::function<void()>>;

template<size_t PoolSize>
struct thread_pool
{
    template<typename F>
    thread_pool(F&& func)
    {
        TRACE(for (size_t i = 0; i < PoolSize; i++))
        {
            TRACE(threads_.emplace_back(func));
        }
    }

    ~thread_pool()
    {
        TRACE(for (auto& th : threads_))
        {
            TRACE(th.join());
        }
    }
    std::vector<std::thread> threads_;
};

struct coroutine : std::coroutine_handle<promise>
{
    using promise_type = ::promise;

    std::optional<int> get_value();

};
 
struct promise
{
    coroutine get_return_object() { return {coroutine::from_promise(*this)}; }
    //std::suspend_always initial_suspend() noexcept { return {}; }
    std::suspend_never initial_suspend() noexcept { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void return_void()
    {
        TRACE(ended_.store(true));
        TRACE(cv_.notify_all());
    }
    void unhandled_exception() {}

    //std::suspend_always yield_value(int from)
    std::suspend_never yield_value(int from)
    {
        TRACE(std::unique_lock<std::mutex> lck(mut_));
        TRACE(cv_.wait(lck, [this]{ return !value_.has_value(); }));
        TRACE(value_ = from); // caching the result in promise
        TRACE(cv_.notify_one());
        TRACE(return {});
    }

    std::optional<int> get_value()
    {
        TRACE(std::unique_lock<std::mutex> lck(mut_));
        TRACE(cv_.wait(lck, [this]{ return value_.has_value() || ended_.load(); }));
        TRACE(if (ended_.load()))
        {
            TRACE(return {});
        }
        TRACE(auto value = *value_);
        TRACE(value_.reset());
        TRACE(cv_.notify_one());
        TRACE(return value);
    }

    std::mutex mut_;
    std::condition_variable cv_;
    std::optional<int> value_;
    std::atomic<bool> ended_{false};
};

std::optional<int> coroutine::get_value() {
    TRACE(return promise().get_value());
}

auto awaitable_function(work_queue& q, int i)
{
    struct awaitable {
        bool await_ready() { TRACE(return ready_.load()); }
        void await_suspend(std::coroutine_handle<> h)
        {
            TRACE(queue_.push([this, h]{
                TRACE(h.resume());
                TRACE(ready_.store(true));
            }));
        }

        int await_resume() {
            TRACE(return i_);
        }

        work_queue& queue_;
        int i_;
        std::atomic<bool> ready_;
    };

    return awaitable(q, i, false);
}

coroutine my_coro(work_queue& q, int i)
{
    TRACE(for (int j = 0; j < i; j++)) {
        TRACE(auto v = co_await awaitable_function(q, j));
        TRACE(co_yield v);
    }
}

int main()
{
    TRACE(work_queue work_queue);
    TRACE(thread_pool<4> tp{[&work_queue]{
        TRACE(for(auto value = work_queue.pop(); value; value = work_queue.pop()))
        {
            TRACE((*value)());
        }
    }});
    
    TRACE(coroutine h = my_coro(work_queue, 20));
    
    TRACE(for(auto v = h.get_value(); v.has_value(); v = h.get_value()))
    {
        D("The readed value is " << v.value());
    }
    TRACE(h.destroy());
    TRACE(work_queue.close());

    return 0;
}