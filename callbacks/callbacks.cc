#include <playgroundcpp/callback.hh>

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
using my_callback_helper = playgroundcpp::callback::generate_for<void, int>::multiple_id<id_func, default_return_void>;

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
