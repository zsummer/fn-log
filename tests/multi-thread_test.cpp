#include "fn_log.h"



constexpr int size = sizeof(FNLog::Logger);

void thread_proc()
{
    while ((LOGD().write_buffer("rrrrrrrrrrrrrrrrrrrradfads33333333333333rrd",
        sizeof("rrrrrrrrrrrrrrrrrrrradfads33333333333333rrd") - 1)
        << -23 << ": " << 32.2223 << (void*) nullptr).log_data_);
}

int main(int argc, char* argv[])
{
    FNLog::Logger& logger = FNLog::GetDefaultLogger();
    FNLog::GuardLogger gl(logger);
    FNLog::UseDefaultConfig1(logger);
    int ret = FNLog::StartDefaultLogger();
    if (ret != 0 || logger.last_error_ != 0)
    {
        printf("log start error. ret:<%d>.", ret);
        return ret || FNLog::GetDefaultLogger().last_error_;
    }
    static const int WRITE_THREAD_COUNT = 1;
    for (int i = 0; i < WRITE_THREAD_COUNT; i++)
    {
        new std::thread(thread_proc);
    }

    do
    {
        long long last_writed = logger.channels_[0].devices_[0].log_fields_[FNLog::DEVICE_LOG_TOTAL_WRITE_LINE].num_;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        long long now_writed = logger.channels_[0].devices_[0].log_fields_[FNLog::DEVICE_LOG_TOTAL_WRITE_LINE].num_;
        LOGI() << "now writed:" << now_writed - last_writed << ", cache hit:"
            << (double)logger.channels_[0].log_fields_[FNLog::CHANNEL_LOG_ALLOC_CACHE].num_
            / logger.channels_[0].log_fields_[FNLog::CHANNEL_LOG_ALLOC_CALL].num_ * 100.0;

    } while (true);

    LOGCA(0, 1) << "finish";
    return 0;
}

