#include "profiler.h"
#include <fstream>

namespace profiler {

static void Write(std::thread::id thread_id,
                  const char *name,
                  std::chrono::steady_clock::time_point start,
                  std::chrono::nanoseconds duration,
                  std::ostream& out_stream) {

    out_stream << std::setprecision(3) << std::fixed;
    out_stream << ",{";
    out_stream << R"("cat":"function",)";
    out_stream << "\"dur\":" << std::chrono::duration_cast<std::chrono::microseconds>(duration).count() << ',';
    out_stream << R"("name":")" << name << "\",";
    out_stream << R"("ph":"X",)";
    out_stream << "\"pid\":0,";
    out_stream << R"("tid":")" << thread_id << "\",";
    out_stream << "\"ts\":" << std::chrono::time_point_cast<std::chrono::microseconds>(start).time_since_epoch().count();
    out_stream << "}";
}

Profiler::~Profiler() {
    std::ofstream file_stream;
    file_stream.open("results.json");
    assert(file_stream.is_open() && "Couldn't open profiler results file.");
    std::lock_guard lock(mutex_);
    file_stream << R"({"otherData": {},"traceEvents":[{})" << out_stream_.str() << "]}";
}

void Profiler::BeginScope(const char *name) {
    GetThreadContext().scopes.push_back({name, std::chrono::steady_clock::now()});
}

void Profiler::EndScope() {
    auto& ctx = GetThreadContext();
    auto& scope = ctx.scopes.back();
    {
        std::lock_guard lock(mutex_);
        Write(ctx.id, scope.name, scope.start, std::chrono::steady_clock::now() - scope.start, out_stream_);
    }
    ctx.scopes.pop_back();
}

Profiler::ThreadContext &Profiler::GetThreadContext() {
    thread_local ThreadContext* ctx = [&](){
        ThreadContext& new_ctx = contexts_.emplace_back();
        new_ctx.id = std::this_thread::get_id();
        new_ctx.scopes.reserve(64);
        return &new_ctx;
    }();
    return *ctx;
}

Profiler &get() {
    static Profiler profiler;
    return profiler;
}

}