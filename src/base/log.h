#pragma once

#include <memory>


#include "spdlog/spdlog.h"

namespace logger {

void init(const char *path);

namespace details {

enum logger_type {
  Core,
  App
};

spdlog::logger CreateLogger(std::string name, const std::string &);

template<logger_type type>
struct LogBase {
 public:
  template<typename ...Args>
  static void Info(const std::string &str, const Args &...args) {
    logger_->info(str, args...);
  }

  template<typename... Args>
  static void Warning(const std::string &str, const Args &... args) {
    logger_->warn(str, args...);
  }

  template<typename... Args>
  static void Error(const std::string &str, const Args &... args) {
    logger_->error(str, args...);
  }

 public:
  static void init(const char *name, const char *log_path) {
    logger_ = std::make_unique<spdlog::logger>(CreateLogger({name}, log_path));
  }

  friend void ::logger::init(const char *path);

  static std::unique_ptr<spdlog::logger> logger_;
};

template <logger_type type>
std::unique_ptr<spdlog::logger> LogBase<type>::logger_ {};

}

using core = details::LogBase<details::logger_type::Core>;
using app = details::LogBase<details::logger_type::App>;

template<typename... Args>
static std::string format(const std::string &str, const Args &... args) {
  spdlog::memory_buf_t buf;
  fmt::format_to(buf, str, args...);
  return {buf.data(), buf.size()};
}

}