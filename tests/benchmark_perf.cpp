/*
 * benchmark_perf.cpp

 */

 
#define FN_LOG_MAX_LOG_SIZE 8000 
#define FN_LOG_MAX_LOG_QUEUE_SIZE 100000 
#include "fn_log.h"

#include <algorithm>
#include <vector>
#include <chrono>
#include <cstdio>
#include <atomic>

static const std::string config_text =
R"----(
 - channel: 0
    sync: async
    -device: 0
        disable: false
        out_type: file
        file: "$PNAME"
        rollback: 4
        limit_size: 100 m
)----";

static const int WARMUP = 5000;
static const int LATENCY_SAMPLES = 100000;
static const int THROUGHPUT_SECONDS = 5;


void bench_latency()
{
    printf("[Latency] warming up...\n");
    for (int i = 0; i < WARMUP; i++)
        LogInfo() << "warmup " << i;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::vector<long long> ns_vec;
    ns_vec.reserve(LATENCY_SAMPLES);

    for (int i = 0; i < LATENCY_SAMPLES; i++)
    {
        auto t0 = std::chrono::high_resolution_clock::now();
        LogInfo() << "latency msg " << i << " payload";
        auto t1 = std::chrono::high_resolution_clock::now();
        ns_vec.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count());
    }

    std::sort(ns_vec.begin(), ns_vec.end());
    long long sum = 0;
    for (auto v : ns_vec) sum += v;

    auto pct = [&](double p) -> long long {
        size_t idx = (size_t)(p / 100.0 * (double)ns_vec.size());
        if (idx >= ns_vec.size()) idx = ns_vec.size() - 1;
        return ns_vec[idx];
    };

    printf("[Latency] samples=%d  avg=%lld ns\n", LATENCY_SAMPLES, sum / (long long)ns_vec.size());
    printf("  p50=%lld  p95=%lld  p99=%lld  p999=%lld  max=%lld ns\n",
           pct(50), pct(95), pct(99), pct(99.9), ns_vec.back());
}


void bench_throughput(int thread_count)
{
    std::atomic<bool> running{false};
    std::atomic<long long> total{0};

    auto worker = [&](int id) {
        while (!running.load(std::memory_order_acquire))
            std::this_thread::yield();
        long long cnt = 0;
        while (running.load(std::memory_order_relaxed))
        {
            LogInfo() << "tput t:" << id << " n:" << cnt << " payload data";
            cnt++;
        }
        total.fetch_add(cnt, std::memory_order_relaxed);
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < thread_count; i++)
        threads.emplace_back(worker, i);

    auto t0 = std::chrono::high_resolution_clock::now();
    running.store(true, std::memory_order_release);
    std::this_thread::sleep_for(std::chrono::seconds(THROUGHPUT_SECONDS));
    running.store(false, std::memory_order_release);

    for (auto& t : threads) t.join();
    auto t1 = std::chrono::high_resolution_clock::now();

    double sec = std::chrono::duration<double>(t1 - t0).count();
    long long lines = total.load();
    printf("[Throughput] threads=%-2d  lines=%-12lld  %.0f lines/sec\n",
           thread_count, lines, (double)lines / sec);
}

/*
optimeze todo: 
    1. rdtsc 
    2. head log delay format and write to transit (use some meta data) 
    3. all delay format support 
    4. arena MPSC (like SPSC)  
    5. meta and log content separate
*/

int main(int argc, char* argv[])
{
    (void)argc; (void)argv;

    int ret = FNLog::FastStartDefaultLogger(config_text);
    if (ret != 0)
    {
        printf("start logger error: %d\n", ret);
        return ret;
    }

    printf("\n======== fn-log Performance Benchmark ========\n\n");

    bench_latency();
    printf("\n");


    std::this_thread::sleep_for(std::chrono::seconds(2));

    bench_throughput(1);
    bench_throughput(2);
    bench_throughput(4);

    printf("\n===============================================\n\n");

    LogAlarm() << "benchmark finished";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}
