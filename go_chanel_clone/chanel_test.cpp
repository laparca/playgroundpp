#include <chanel.hpp>
#include <gtest/gtest.h>

TEST(chanel0_test, close) {
    laparca::chanel0<size_t> c;
    c.close();
    ASSERT_TRUE(c.is_closed());
    ASSERT_FALSE(c.pop().has_value());
}

TEST(chanel0_test, close_with_consumer_waiting) {
    laparca::chanel0<size_t> c;
    std::thread th{[&c]{
        ASSERT_FALSE(c.pop().has_value());
    }};

    std::this_thread::sleep_for(std::chrono::seconds(2));

    c.close();
    th.join();
}

TEST(chanel0_test, close_with_producer_waiting) {
    laparca::chanel0<size_t> c;
    std::thread th{[&c]{
        ASSERT_FALSE(c.push(1));
    }};

    std::this_thread::sleep_for(std::chrono::seconds(2));

    c.close();
    th.join();
}

TEST(chanel0_test, normal_behaviour) {
    laparca::chanel0<size_t> c;
    std::thread consumer{[&c]{
        auto value = c.pop();
        ASSERT_TRUE(value.has_value());
        if (value.has_value()) {
            ASSERT_EQ(value.value(), 42);
        }
    }};

    std::thread producer{[&c]{
        ASSERT_TRUE(c.push(42));
    }};

    producer.join();
    consumer.join();
}

TEST(chanelN_test, close) {
    laparca::chanelN<size_t> c(1);
    c.close();
    ASSERT_TRUE(c.is_closed());
    ASSERT_FALSE(c.push(1));
    ASSERT_FALSE(c.pop().has_value());
}

TEST(chanelN_test, close_with_consumer_waiting) {
    laparca::chanelN<size_t> c(1);
    std::thread th{[&c]{
        ASSERT_FALSE(c.pop().has_value());
    }};

    std::this_thread::sleep_for(std::chrono::seconds(2));

    c.close();
    th.join();
}

TEST(chanelN_test, close_with_producer_waiting) {
    laparca::chanelN<size_t> c(1);
    std::thread th{[&c]{
        ASSERT_TRUE(c.push(1));
        ASSERT_FALSE(c.push(2));
    }};

    std::this_thread::sleep_for(std::chrono::seconds(2));

    c.close();
    th.join();
}

TEST(chanelN_test, normal_behaviour) {
    laparca::chanelN<size_t> c(1);
    std::thread consumer{[&c]{
        auto value = c.pop();
        ASSERT_TRUE(value.has_value());
        if (value.has_value()) {
            ASSERT_EQ(value.value(), 42);
        }
    }};

    std::thread producer{[&c]{
        ASSERT_TRUE(c.push(42));
    }};

    producer.join();
    consumer.join();

    ASSERT_TRUE(c.push(43));
    ASSERT_EQ(c.pop(), std::optional<size_t>{43});
}

TEST(chanel, iteration_without_buffer) {
    size_t values[]{2, 6, 42};

    laparca::chanel<size_t> c;
    std::thread consumer{[&]{
        int i = 0;
        for (auto v : c) {
            ASSERT_EQ(v, values[i++]);
        }
    }};

    for (auto const& v: values) {
        ASSERT_TRUE(c.push(v));
    }    
    
    c.close();
    consumer.join();
}

TEST(chanel, iteration_with_buffer) {
    size_t values[]{2, 6, 42};

    laparca::chanel<size_t> c(2);
    std::thread consumer{[&]{
        int i = 0;
        for (auto v : c) {
            ASSERT_EQ(v, values[i++]);
        }
    }};

    for (auto const& v: values) {
        ASSERT_TRUE(c.push(v));
    }    
    
    c.close();
    consumer.join();
}