#include <deque>
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>
#include <sstream>
#include <string>

template<typename F>
struct deferred {
	deferred(const F& f) : f_(f) {}
	~deferred() {
		f_();
	}
private:
	F f_;
};

#ifdef DEBUG
#define M(x) do { std::cout << x; } while(false)
#define NTRACE(x)
#else
#define M(x)
#define NTRACE(x) do { std::cout << x << std::endl; } while(false)
#endif

template<typename T>
struct queue {
	queue(size_t capacity = 0) : capacity_{capacity} {}

	void insert(const T& v) {
		M(0);
		const T* tmp = &v;
		const T* expected = nullptr;
		while(!next_.compare_exchange_weak(expected, tmp)) {
			M(1);
			next_.wait(expected);
			expected = nullptr;
		}
		M(2);

		next_.notify_all();

		// Wait until the value is readed
		next_.wait(tmp);
	}

	T retrieve() {
		M(3);
		const T* tmp = next_.load();
		while(tmp == nullptr || !next_.compare_exchange_weak(tmp, nullptr)) {
			M(4);
			next_.wait(nullptr);
			tmp = next_.load();
		}

		M(5);
		auto defer = deferred([this]() {
			M(6);
			next_.notify_all();
		});

		return *tmp;
	}

	void close() {}
	size_t size() { return 0; }
	size_t capacity() { return capacity_; }

private:
	std::atomic<const T*> next_ = nullptr;
	T** buffer_;
	size_t capacity_;
};

int main(int argc, char** argv) {
	queue<ssize_t> q;
	ssize_t totalElements = 1000;
	ssize_t numConsumers = 3;
	ssize_t numProducers = 2;

	using namespace std::string_literals;

	for (int i = 1; i < argc; i++) {
		if (argv[i] == "elements"s && i+1 < argc) {
			totalElements = std::stoi(argv[++i]);
		}
		else if (argv[i] == "consumers"s && i+1 < argc) {
			numConsumers = std::stoi(argv[++i]);
		}
		else if (argv[i] == "producers"s && i+1 < argc) {
			numProducers = std::stoi(argv[++i]);
		}
	}

	std::vector<std::unique_ptr<std::thread>> consumers(numConsumers);
	std::vector<std::unique_ptr<std::thread>> producers(numProducers);

	for (ssize_t consumer = 0; consumer < numConsumers; ++consumer)
		consumers[consumer] = std::make_unique<std::thread>([&q, consumer, totalElements, numConsumers]() {
			const ssize_t elementsToConsume = (totalElements / numConsumers) + (consumer + 1 == numConsumers ? (totalElements % numConsumers): 0);
			std::stringstream s;
			s << "[C:" << consumer<< "] Should consume " << elementsToConsume << " elements" << std::endl;
			std::clog << s.str();
			for (ssize_t i = 0; i < elementsToConsume; ++i) {
				M(7);
#ifdef DEBUG
				q.retrieve();
#else
				std::stringstream s;
				s << "[C:" << consumer << "] Values is " << q.retrieve() << std::endl;
				std::clog << s.str();
#endif
			}
		});

	for (ssize_t producer = 0; producer < numProducers; ++producer)
		producers[producer] = std::make_unique<std::thread>([&q,producer, totalElements, numProducers](){
			const ssize_t elementsToProduce = (totalElements / numProducers) + (producer + 1 == numProducers ? (totalElements % numProducers) : 0);
			const ssize_t start = (totalElements / numProducers) * producer;

			std::clog << "[P:" << producer << "] Should produce " << elementsToProduce << " elements starting at " << start << std::endl;
			for (ssize_t i = 0; i < elementsToProduce; ++i) {
				M(8);
				std::clog << "[P:" << producer << "]: " << (i+start) << std::endl;
				q.insert(i+start);
			}
			
		});	

	for (auto &th : consumers)
		th->join();
	for (auto &th : producers)
		th->join();	
	return 0;
}
