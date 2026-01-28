#pragma once

#include <utility>
#include <chrono>
#include <functional>

namespace test::detail
{
// Rather simple std::chrono-based timer for microbenchmarking
class Timer {
  std::chrono::time_point<std::chrono::system_clock> m_start, m_stop;
public:
  void tick() {
    m_start = std::chrono::system_clock::now();
  }
  
  void tock() {
    m_stop = std::chrono::system_clock::now();
  }

  template <typename Du = std::chrono::nanoseconds>
  auto get() const {
    return std::chrono::duration_cast<Du>(m_stop - m_start);
  }

  auto get_ms() const { return get<std::chrono::milliseconds>(); }
  auto get_us() const { return get<std::chrono::microseconds>(); }
  auto get_ns() const { return get<std::chrono::nanoseconds>(); }
};

// Rather simple capture-averaging benchmark; assumes capture resets state
template <typename Du = std::chrono::nanoseconds>
auto benchmark_region(uint32_t n_samples, std::function<void()> capture) {
  Timer t;
  std::chrono::nanoseconds avg {};

  for (uint32_t i = 0; i < n_samples; ++i) {
    t.tick();
    capture();
    t.tock();
    avg += t.get_ns();
  }
  
  avg /= static_cast<int64_t>(n_samples);

  return std::chrono::duration_cast<Du>(avg);
}

auto benchmark_region_ms(uint32_t n_samples, std::function<void()> capture) {
  return benchmark_region<std::chrono::milliseconds>(n_samples, capture); 
}
auto benchmark_region_us(uint32_t n_samples, std::function<void()> capture) {
  return benchmark_region<std::chrono::microseconds>(n_samples, capture); 
}
auto benchmark_region_ns(uint32_t n_samples, std::function<void()> capture) {
  return benchmark_region<std::chrono::nanoseconds>(n_samples, capture); 
}
} // namespace test::detail