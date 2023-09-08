#pragma once

#include <memory>
#include <thread>
#include <type_traits>
#include <vector>

namespace laparca {

template <typename F> class thread_pool {
public:
  thread_pool(size_t pool_size, F f) : pool_(pool_size) {
    for (auto &element : pool_) {
      element = std::make_unique<std::thread>(f);
    }
  }

  void join() {
    for (auto &element : pool_) {
      element->join();
    }
  }

private:
  std::vector<std::unique_ptr<std::thread>> pool_;
};

template <typename F> thread_pool(size_t, F) -> thread_pool<std::decay_t<F>>;
} // namespace laparca