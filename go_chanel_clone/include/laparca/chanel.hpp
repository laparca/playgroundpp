/******************************************************************************
 * Copyright (C) 2023 Samuel R. Sevilla <laparca@laparca.es>
 * 
 * Chanel is a implementation of the go chanel. It is a convinient way to
 * comunicate threads.
 * 
 * This implentation is lockless to improve performance.
 *****************************************************************************/
#pragma once

#include <deferred.hpp>

#include <optional>
#include <memory>
#include <vector>
#include <atomic>

namespace laparca {
/** 
 * chanel_interface defines the required functions that should be implemented by any chanel.
 * A chanel is a way to comunicate different process. It is the clasic way a producer-consumer
 * works.
 * 
 * \tparam T type of the elements that can be passed to the chanel
 * \tparam Allocator type of the allocator to use for instance elements of type T
 */
template<typename T, typename Allocator>
struct chanel_interface {
	using allocator_type = Allocator;
	using value_type = T;
	using reference = value_type&;
	using universal_reference = value_type&&;
	using const_reference = value_type const&;
	using pointer = value_type*;
	using const_pointer = value_type const*;

    virtual ~chanel_interface() {}

    /**
     * push inserts an element into the chanel. If the element is passed by reference,
     * the it is copied. In other case, the element will be moved.
     * 
     * \return true if the element was inserted. false if the chanel is closed.
     */
	virtual bool push(const_reference) = 0;
	virtual bool push(universal_reference) = 0;

    /**
     * pop return the first element in the chanel.
     * 
     * \return An optional with the value or an empty optional y the chanel is closed.
     */
	virtual std::optional<value_type> pop() = 0;

    /**
     * close the chanel. After this call, push and pop operations will return
     * false or empty optional.
     */
	virtual void close() = 0;

    /**
     * returns if the chanel is opened.
     */
	virtual bool is_closed() = 0;

    /**
     * returns the number of elements in the chanel.
     */
	virtual size_t size() = 0;

    /**
     * returns the capacity of the chanel.
     */
	virtual size_t capacity() = 0;
};

template<typename T, typename Allocator = std::allocator<T>>
struct chanel0 : public chanel_interface<T, Allocator> {
	using allocator_type = Allocator;
	using value_type = T;
	using reference = value_type&;
	using universal_reference = value_type&&;
	using const_reference = value_type const&;
	using pointer = value_type*;
	using const_pointer = value_type const*;

    virtual ~chanel0() {
        if (!is_closed()) close();
    }    

    bool push(const_reference value) override {
		return internal_push(value, false);
	}

	bool push(universal_reference value) override {
		return internal_push(std::move(value), true);
	}

	std::optional<value_type> pop() override {
		const_pointer tmp = next_.load();
		while(!is_closed() && (tmp == nullptr || !next_.compare_exchange_weak(tmp, nullptr))) {
			waiting_for_read_ ++;
			next_.wait(nullptr);
			waiting_for_read_ --;
			tmp = next_.load();
		}

		if (is_closed()) return {};

		auto defer = deferred([this]() {
			next_.notify_all();
		});

		if (move)
			return std::move(const_cast<reference>(*tmp));
		else
			return *tmp;
	}

	void close() override {
		closed_ = true;

		// Notify and wait for read threads
		next_ = reinterpret_cast<pointer>(0x01);
		next_.notify_all();
		while(waiting_for_read_.load());

		// Notify and wait for write threads
		next_ = nullptr;
		next_.notify_all();
		while(waiting_for_write_.load());
	}
	bool is_closed() override {
		return closed_;
	}
	size_t size() override {
		return 0;
	}

	size_t capacity() override {
		return 0;
	}
private:
	template<typename U>
	bool internal_push(U&& value, bool move) {
		const_pointer tmp = &value;
		const_pointer expected = nullptr;
		while(!is_closed() && !next_.compare_exchange_weak(expected, tmp)) {
			waiting_for_write_ ++;
			next_.wait(expected);
			waiting_for_write_ --;
			expected = nullptr;
		}

		if (is_closed()) return false;

		this->move = move;

		next_.notify_all();

		// Wait until the value is readed
        waiting_for_write_ ++;
		next_.wait(tmp);
        waiting_for_write_ --;

        return !is_closed();
	}

private:
	std::atomic<const_pointer> next_ = nullptr;
	std::atomic<size_t> waiting_for_read_ = 0;
	std::atomic<size_t> waiting_for_write_ = 0;
	bool move = false;
	bool closed_ = false;
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
		CAN_READ_CLOSING,
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
		if (!is_closed()) close();
        allocator_.deallocate(buffer_, capacity_);
	}

	bool push(const_reference v) override {
		return internal_push(v);
	}

	bool push(universal_reference v) override {
		return internal_push(std::move(v));
	}

	std::optional<value_type> pop() override {
		size_t pos = start_.load();
		while(!start_.compare_exchange_weak(pos, (pos+1) % capacity_)) {
			pos = start_.load();
		}

		// wait to the slot to be enabled to write
		auto status = buffer_status_[pos].load();
		while(status != CLOSED) {
			while (status != CLOSED && status != CAN_READ && status != CAN_READ_CLOSING) {
				buffer_status_[pos].wait(status);
				status = buffer_status_[pos].load();
			}
       
			if (status == CLOSED) return {};

			if(buffer_status_[pos].compare_exchange_weak(status, READING)) {
				break;
			}
		}
			
		if (status == CLOSED) return {};

		auto defer = deferred([&,this]() {
			size_ --;
			std::destroy_at(&buffer_[pos]);
			buffer_status_[pos].store(is_closed() ? CLOSED : EMPTY);
			buffer_status_[pos].notify_one();
		});

		return std::move(buffer_[pos]);
	}

	void close() override {
		closed_ = true;
		for (auto& status: buffer_status_) {
			auto expected_status = EMPTY;
			// Only mark as closed the empty cells
			status.compare_exchange_weak(expected_status, CLOSED);
			expected_status = CAN_READ;
			status.compare_exchange_weak(expected_status, CAN_READ_CLOSING);
			status.notify_all();
		}
	}

	bool is_closed() override {
		return closed_;
	}

	size_t size() override {
		return size_;
	}

	size_t capacity() override {
		return capacity_;
	}

private:
	template<typename U>
	bool internal_push(U&& v) {
		// Reserve the slot where to store the value
		size_t pos = end_.load();
		while(!end_.compare_exchange_weak(pos, (pos+1) % capacity_)) {
			pos = end_.load();
		}

		// wait to the slot to be enabled to write
		auto status = buffer_status_[pos].load();
		while(!is_closed()) {
			while (!is_closed() && status != EMPTY) {
				buffer_status_[pos].wait(status);
				status = buffer_status_[pos].load();
			}

			status = EMPTY;
			if(buffer_status_[pos].compare_exchange_weak(status, WRITING)) {
				break;
			}
		}

        if (is_closed()) return false;

		std::construct_at(&buffer_[pos], std::forward<U>(v));
		size_++;
		buffer_status_[pos].store(CAN_READ);
		buffer_status_[pos].notify_one();

		return true;
	}

private:
	std::atomic<const T*> next_ = nullptr;
	bool closed_ = false;
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

	bool push(const_reference v) override {
		return internal_chanel_->push(v);
	}

	bool push(universal_reference v) override {
		return internal_chanel_->push(std::move(v));
	}

	std::optional<value_type> pop() override {
		return internal_chanel_->pop();
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

	std::optional<value_type> operator()() {
		return pop();
	}

private:
	struct chanel_iterator {
		value_type operator*() {
			return *value_;
		}

		chanel_iterator& operator++() {
			value_ = chan_.pop();
			return *this;
		}

		chanel_iterator operator++(int) {
			deferred _{[this](){ value_ = chan_.pop(); }};
			return *this;
		}

        bool operator!=(const chanel_iterator& value) {
            return value_ != value.value_;
        }

		std::optional<value_type> value_;
        chanel& chan_;
	};

public:
	chanel_iterator begin() {
		return {pop(), *this};
	}

	chanel_iterator end() {
		return {{}, *this};
	}
private:
	std::shared_ptr<chanel_interface<T, Allocator>> internal_chanel_;
};

template<typename T>
chanel<T>& operator<<(chanel<T>& q, const T& v) {
	q.push(v);
	return q;
}
} // namespace laparca
