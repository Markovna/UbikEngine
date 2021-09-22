#pragma once

#include <chrono>
#include <vector>
#include <thread>
#include <sstream>
#include <iomanip>

#include "base/log.h"
#include "base/macro.h"

#define UBIK_PROFILE 0

namespace profiler {

class Profiler {
private:
    struct ScopeInfo {
        const char *name = nullptr;
        std::chrono::steady_clock::time_point start;
    };

    struct ThreadContext {
        std::vector<ScopeInfo> scopes;
        std::thread::id id;
    };

public:
    Profiler() = default;
    ~Profiler();

    void BeginScope(const char *name);

    void EndScope();
private:
    ThreadContext& GetThreadContext();

private:
    std::vector<ThreadContext> contexts_;
    std::stringstream out_stream_;
    std::mutex mutex_;
};

Profiler &get();

struct Scope {
public:
    explicit Scope(const char *name) { get().BeginScope(name); }
    ~Scope() { get().EndScope(); }
};

};

#ifndef UBIK_PROFILE
#define UBIK_PROFILE 0
#endif

#if UBIK_PROFILE
#   define UBIK_PROFILE_SCOPE(__name) ::sprint::profiler::Scope UBIK_CONCAT(_profile_scope, __LINE__)(__name);
#   define UBIK_PROFILE_FUNCTION() UBIK_PROFILE_SCOPE(UBIK_FUNCTION)
#else
#   define UBIK_PROFILE_SCOPE(__name)
#   define UBIK_PROFILE_FUNCTION()
#endif

