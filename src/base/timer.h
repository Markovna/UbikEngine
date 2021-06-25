#pragma once

#include <chrono>

class time_span {
 private:
  using seconds_t = std::chrono::duration<float, std::chrono::seconds::period>;
  using milliseconds_t = std::chrono::milliseconds;
  using microseconds_t = std::chrono::microseconds;

  static constexpr const float kSecToMicrosec = 1000000.0f;
  static constexpr const float kMillisecToMicrosec = 1000.0f;

 public:
  [[nodiscard]] float as_seconds() const {
    return std::chrono::duration_cast<seconds_t>(microseconds_).count();
  }
  [[nodiscard]] int32_t as_milliseconds() const {
    return std::chrono::duration_cast<milliseconds_t>(microseconds_).count();
  }
  [[nodiscard]] int64_t as_microseconds() const {
    return microseconds_.count();
  }
  static time_span seconds(float seconds) {
    return time_span(microseconds_t(int64_t(seconds * kSecToMicrosec)));
  }
  static time_span milliseconds(int32_t milliseconds) {
    return time_span(microseconds_t(int64_t(milliseconds * kMillisecToMicrosec)));
  }
  static time_span microseconds(int64_t microseconds) {
    return time_span(microseconds_t(int64_t(microseconds)));
  }

 private:
  microseconds_t microseconds_;

 public:
  explicit time_span(microseconds_t microseconds) noexcept : microseconds_(microseconds) {}
  time_span() noexcept : microseconds_(0) {}
};

class timer {
 private:
  using time_point_t = std::chrono::steady_clock::time_point;
 public:
  timer() noexcept : time_point_(std::chrono::steady_clock::now()) {}

  time_span restart() {
    time_span t = time();
    time_point_ = std::chrono::steady_clock::now();
    return t;
  }

  [[nodiscard]] time_span time() const {
    using namespace std::chrono;
    return time_span(duration_cast<microseconds>(steady_clock::now() - time_point_));
  }
 private:
  time_point_t time_point_;
};