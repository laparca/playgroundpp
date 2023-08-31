#include <deque>
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>
#include <sstream>
#include <string>
#include <memory> // std::allocator
#include <concepts>

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

template<typename T, typename Allocator>
struct chanel_interface {
	using allocator_type = Allocator;
	using value_type = T;
	using reference = value_type&;
	using universal_reference = value_type&&;
	using const_reference = value_type const&;
	using pointer = value_type*;
	using const_pointer = value_type const*;

	virtual void insert(const_reference) = 0;
	virtual void insert(universal_reference) = 0;
	virtual value_type get() = 0;

	virtual void close() = 0;
	virtual bool is_closed() = 0;
	virtual size_t size() = 0;
	virtual size_t capacity() = 0;
};

template<typename T, typename Allocator = std::allocator<T>> requires std::copyable<T>
struct chanel0 : public chanel_interface<T, Allocator> {
	using allocator_type = Allocator;
	using value_type = T;
	using reference = value_type&;
	using universal_reference = value_type&&;
	using const_reference = value_type const&;
	using pointer = value_type*;
	using const_pointer = value_type const*;

	void insert(const_reference value) override {
		const_pointer tmp = &value;
		const_pointer expected = nullptr;
		while(!next_.compare_exchange_weak(expected, tmp)) {
			next_.wait(expected);
			expected = nullptr;
		}

		move = false;

		next_.notify_all();

		// Wait until the value is readed
		next_.wait(tmp);
	}

	void insert(universal_reference value) override {
		const_pointer tmp = &value;
		const_pointer expected = nullptr;
		while(!next_.compare_exchange_weak(expected, tmp)) {
			next_.wait(expected);
			expected = nullptr;
		}

		move = true;

		next_.notify_all();

		// Wait until the value is readed
		next_.wait(tmp);
	}

	value_type get() override {
		const_pointer tmp = next_.load();
		while(tmp == nullptr || !next_.compare_exchange_weak(tmp, nullptr)) {
			next_.wait(nullptr);
			tmp = next_.load();
		}

		auto defer = deferred([this]() {
			next_.notify_all();
		});

		if (move)
			return std::move(const_cast<reference>(*tmp));
		else
			return *tmp;
	}

	void close() override {}
	bool is_closed() override {
		return false;
	}
	size_t size() override {
		return 0;
	}

	size_t capacity() override {
		return 0;
	}
private:
	std::atomic<const_pointer> next_ = nullptr;
	bool move = false;
};

template<typename T, typename Allocator = std::allocator<T>>
struct chanelN : public chanel_interface<T, Allocator> {
	using allocator_type = Allocator;
	using value_type = T;
	using reference = value_type&;
	using universal_reference = value_type&&;
	using const_reference = value_type const&;
	using pointer = value_type*;
	using const_pointer = value_type const*;

	enum BucketCellStatus {
		EMPTY,
		WRITING,
		CAN_READ,
		READING,
		CLOSED
	};

	chanelN(size_t capacity = 0, Allocator allocator = Allocator{})
	: allocator_{allocator}
	, capacity_{capacity}
	, size_{0}
	, buffer_{allocator.allocate(capacity_)}
	, buffer_status_(capacity_)
	, start_{0}
	, end_{0} {}

	~chanelN() {
		close();
		delete buffer_;
	}

	template<typename U>
	void internal_insert(U&& v) {
		// Is the queue full?
		//while(end_ == start_ && size_ != 0) {
		//	start_.wait(start_.load());
		//}

		// Reserve the slot where to store the value
		size_t pos = end_.load();
		while(!end_.compare_exchange_weak(pos, (pos+1) % capacity_)) {
			pos = end_.load();
		}

		// wait to the slot to be enabled to write
		auto status = buffer_status_[pos].load();
		while(true) {
			while (status != EMPTY) {
				buffer_status_[pos].wait(status);
				status = buffer_status_[pos].load();
			}

			status = EMPTY;
			if(buffer_status_[pos].compare_exchange_weak(status, WRITING)) {
				break;
			}
		}

		std::construct_at(&buffer_[pos], std::forward<U>(v));
		size_++;
		buffer_status_[pos].store(CAN_READ);
		buffer_status_[pos].notify_one();
	}

	void insert(const_reference v) override {
		internal_insert(v);
	}

	void insert(universal_reference v) override {
		internal_insert(std::move(v));
	}

	T get() override {
		size_t pos = start_.load();
		while(!start_.compare_exchange_weak(pos, (pos+1) % capacity_)) {
			pos = start_.load();
		}

		// wait to the slot to be enabled to write
		auto status = buffer_status_[pos].load();
		while(true) {
			while (status != CAN_READ) {
				buffer_status_[pos].wait(status);
				status = buffer_status_[pos].load();
			}

			status = CAN_READ;
			if(buffer_status_[pos].compare_exchange_weak(status, READING)) {
				break;
			}
		}

		auto defer = deferred([&,this]() {
			size_ --;
			std::destroy_at(&buffer_[pos]);
			buffer_status_[pos].store(EMPTY);
			buffer_status_[pos].notify_one();
		});

		return std::move(buffer_[pos]);
	}

	void close() override {
		for (auto& status: buffer_status_) {
			status.store(CLOSED);
			status.notify_all();
		}
	}
	bool is_closed() override {
		return buffer_status_[0] == CLOSED;
	}
	size_t size() override {
		return size_;
	}
	size_t capacity() override {
		return capacity_;
	}

private:
	std::atomic<const T*> next_ = nullptr;
	Allocator allocator_;
	size_t capacity_;
	std::atomic<size_t> size_;
	T* buffer_;
	std::vector<std::atomic<BucketCellStatus>> buffer_status_;
	std::atomic<size_t> start_;
	std::atomic<size_t> end_;
};

template<typename T, typename Allocator = std::allocator<T>>
struct chanel : public chanel_interface<T, Allocator>
{
	using allocator_type = Allocator;
	using value_type = T;
	using reference = value_type&;
	using universal_reference = value_type&&;
	using const_reference = value_type const&;
	using pointer = value_type*;
	using const_pointer = value_type const*;

	chanel(size_t capacity = 0) {
		if (capacity > 0)
			internal_chanel_ = std::make_shared<chanelN<T, Allocator>>(capacity);
		else
			internal_chanel_ = std::make_shared<chanel0<T, Allocator>>();
	}

	chanel(const chanel&) = default;
	chanel(chanel&&) = default;

	~chanel() = default;

	void insert(const_reference v) override {
		internal_chanel_->insert(v);
	}

	void insert(universal_reference v) override {
		internal_chanel_->insert(std::move(v));
	}

	T get() override {
		return internal_chanel_->get();
	}

	void close() override {
		internal_chanel_->close();
	}

	bool is_closed() override {
		return internal_chanel_->is_closed();
	}

	size_t size() override {
		return internal_chanel_->size();
	}

	size_t capacity() override {
		return internal_chanel_->capacity();
	}

	value_type operator()() {
		return get();
	}
private:
	std::shared_ptr<chanel_interface<T, Allocator>> internal_chanel_;
};

template<typename T>
chanel<T>& operator<<(chanel<T>& q, const T& v) {
	q.insert(v);
	return q;
}

int main(int argc, char** argv) {
	chanel<ssize_t> q;
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
				s << "[C:" << consumer << "] Values is " << q() << std::endl;
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
				q << (i+start);
			}
			
		});	

	for (auto &th : consumers)
		th->join();
	for (auto &th : producers)
		th->join();	
	return 0;
}
