#include <laparca/chanel.hpp>
#include <gtest/gtest.h>
#include <thread_pool.hpp>
#include <array>

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
    std::array<size_t, 3> values{2, 6, 42};

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

constexpr size_t sum_of_serie(size_t first, size_t last) {
    return (last + first) * (last - first + 1) / 2;
}

TEST(chanel, do_the_sum_of_values) {
	laparca::chanel<ssize_t> q(20);
	constexpr ssize_t totalElements = 10'000'000;
	constexpr size_t numConsumers = 10;
	constexpr ssize_t numProducers = 10;
    std::atomic<ssize_t> producersCount = 0;
    std::atomic<size_t> accum = 0;

	using namespace std::string_literals;

    laparca::thread_pool consumers(numConsumers, [&q, &accum]() {
        size_t partial_accum = 0;
        for (const auto value : q) {
            partial_accum += value;
        }

        accum += partial_accum;
    });
    
    laparca::thread_pool producers(numProducers, [&q, &producersCount, totalElements, numProducers](){
        const ssize_t producer = producersCount++;

		const ssize_t elementsToProduce = (totalElements / numProducers) + (producer + 1 == numProducers ? (totalElements % numProducers) : 0);
		const ssize_t start = (totalElements / numProducers) * producer;

		for (ssize_t i = 0; i < elementsToProduce; ++i) {
			q << (i+start+1);
		}
	});

    producers.join();
	
    q.close();

    consumers.join();
	

    ASSERT_EQ(accum, sum_of_serie(1, totalElements));
}