

#define FN_LOG_MAX_DEVICE_SIZE 36
#define FN_LOG_MAX_CHANNEL_SIZE 3
#define FN_LOG_MAX_LOG_SIZE 1000
#define FN_LOG_MAX_LOG_QUEUE_SIZE 100000

#include "fn_log.h"
#include <chrono>

static const std::string example_config_text =
R"----(
 - channel: 0
    priority: trace
    -device: 0
        disable: false
        out_type: screen
        priority: trace

 - channel: 1
    -device:0
        disable: false
        out_type: empty
        priority: debug
        category: 20
        category_extend: 1
        idenfity: 20
        idenfity_extend: 1

 - channel: 2
    -device:0
        disable: false
        out_type: empty
        priority: error
        category: 0
        category_extend: 1
        idenfity: 0
        idenfity_extend: 1
    -device:1
        disable: false
        out_type: empty
        priority: info
        category: 1
        category_extend: 1
        idenfity: 1
        idenfity_extend: 1
    -device:2
        disable: false
        out_type: empty
        category: 2
        category_extend: 1
        idenfity: 2
        idenfity_extend: 1
    -device:3
        disable: false
        out_type: empty
        category: 3
        category_extend: 1
        idenfity: 3
        idenfity_extend: 1
    -device:4
        category: 4
        category_extend: 1
        idenfity: 4
        idenfity_extend: 1
    -device:5
        disable: false
        out_type: empty
        category: 5
        category_extend: 1
        idenfity: 5
        idenfity_extend: 1
    -device:6
        disable: false
        out_type: empty
        priority: error
        category: 6
        category_extend: 1
        idenfity: 6
        idenfity_extend: 1
    -device:7
        disable: false
        out_type: empty
        category: 7
        category_extend: 1
        idenfity: 7
        idenfity_extend: 1
    -device:8
        disable: false
        out_type: empty
        priority: trace
        category: 8
        category_extend: 1
        idenfity: 8
        idenfity_extend: 1
    -device:9
        disable: false
        out_type: empty
        category: 9
        category_extend: 1
        idenfity: 9
        idenfity_extend: 1
    -device:10
        disable: false
        out_type: empty
        category: 10
        category_extend: 1
        idenfity: 10
        idenfity_extend: 1
    -device:11
        disable: false
        out_type: empty
        category: 11
        category_extend: 1
        idenfity: 11
        idenfity_extend: 1
    -device:12
        disable: false
        out_type: empty
        category: 12
        category_extend: 1
        idenfity: 12
        idenfity_extend: 1
    -device:13
        disable: false
        out_type: empty
        category: 13
        category_extend: 1
        idenfity: 13
        idenfity_extend: 1
    -device:14
        disable: false
        out_type: empty
        category: 14
        category_extend: 1
        idenfity: 14
        idenfity_extend: 1
    -device:15
        disable: false
        out_type: empty
        category: 15
        category_extend: 1
        idenfity: 15
        idenfity_extend: 1
    -device:16
        disable: false
        out_type: empty
        category: 16
        category_extend: 1
        idenfity: 16
        idenfity_extend: 1
    -device:17
        disable: false
        out_type: empty
        category: 17
        category_extend: 1
        idenfity: 17
        idenfity_extend: 1
    -device:18
        disable: false
        out_type: empty
        category: 18
        category_extend: 1
        idenfity: 18
        idenfity_extend: 1
    -device:19
        disable: false
        out_type: empty
        category: 19
        category_extend: 1
        idenfity: 19
        idenfity_extend: 1
    -device:20
        disable: false
        out_type: empty
        category: 20
        category_extend: 1
        idenfity: 20
        idenfity_extend: 1
    -device:21
        disable: false
        out_type: empty
        category: 21
        category_extend: 1
        idenfity: 21
        idenfity_extend: 1
    -device:22
        disable: false
        out_type: empty
        category: 22
        category_extend: 1
        idenfity: 22
        idenfity_extend: 1
    -device:23
        disable: false
        out_type: empty
        category: 23
        category_extend: 1
        idenfity: 23
        idenfity_extend: 1
    -device:24
        disable: false
        out_type: empty
        category: 24
        category_extend: 1
        idenfity: 24
        idenfity_extend: 1
    -device:25
        disable: false
        out_type: empty
        category: 25
        category_extend: 1
        idenfity: 25
        idenfity_extend: 1
    -device:26
        disable: false
        out_type: empty
        category: 26
        category_extend: 1
        idenfity: 26
        idenfity_extend: 1
    -device:27
        disable: false
        out_type: empty
        category: 27
        category_extend: 1
        idenfity: 27
        idenfity_extend: 1
    -device:28
        disable: false
        out_type: empty
        category: 28
        category_extend: 1
        idenfity: 28
        idenfity_extend: 1
    -device:29
        disable: false
        out_type: empty
        category: 29
        category_extend: 1
        idenfity: 29
        idenfity_extend: 1
    -device:30
        disable: false
        out_type: empty
        category: 30
        category_extend: 1
        idenfity: 30
        idenfity_extend: 1
    -device:31
        disable: false
        out_type: empty
        category: 31
        category_extend: 1
        idenfity: 31
        idenfity_extend: 1
    -device:32
        disable: false
        out_type: empty
        category: 32
        category_extend: 1
        idenfity: 32
        idenfity_extend: 1
    -device:33
        disable: false
        out_type: empty
        category: 33
        category_extend: 1
        idenfity: 33
        idenfity_extend: 1
    -device:34
        disable: false
        out_type: empty
        category: 34
        category_extend: 1
        idenfity: 34
        idenfity_extend: 1
    -device:35
        disable: false
        out_type: empty
        category: 35
        category_extend: 1
        idenfity: 35
        idenfity_extend: 1





)----";

std::string ChannelDesc(int channel_type)
{
    switch (channel_type)
    {
    case FNLog::CHANNEL_ASYNC:
        return "async thread write";
    case FNLog::CHANNEL_SYNC:
        return "sync write";
    }
    return "invalid channel";
}

#define Now()  std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count()

int main(int argc, char *argv[])
{
    int ret = FNLog::FastStartDefaultLogger(example_config_text);
    if (ret != 0)
    {
        return ret;
    }


    FNLog::Logger& logger = FNLog::GetDefaultLogger();

    unsigned int total_count = 0;
    for (int channel_id = 1; channel_id < logger.shm_->channel_size_; channel_id++)
    {
        total_count = 0;
        FNLog::Channel& channel = logger.shm_->channels_[channel_id];
        do
        {
            int filter = rand() % 64;
            LOG_STREAM_DEFAULT_LOGGER(channel_id, FNLog::PRIORITY_DEBUG, filter, filter, FNLog::LOG_PREFIX_NULL)
                .write_buffer("rrrrrrrrrrrrrrrrrrrradfads33333333333333rrd",
                    sizeof("rrrrrrrrrrrrrrrrrrrradfads33333333333333rrd") - 1);

            if (total_count %5000000 == 0)
            {
                static double last = Now();
                double now = Now();
                if (total_count > 0)
                {
                    LogInfo() << "channel:<" << (long long)channel_id << "> "
                        << ChannelDesc(channel.channel_type_) << " emtpy write :<"
                        << channel.device_size_ << "> test " << 5000000 / (now - last) << " line/sec. ";
                    last = now;
                    break;
                }
            }
        } while (++total_count);
    }


    for (int channel_id = 1; channel_id < logger.shm_->channel_size_; channel_id++)
    {
        total_count = 0;
        FNLog::Channel& channel = logger.shm_->channels_[channel_id];
        do
        {
            int filter = 20;
            LOG_STREAM_DEFAULT_LOGGER(channel_id, FNLog::PRIORITY_DEBUG, filter, filter, FNLog::LOG_PREFIX_NULL)
                .write_buffer("rrrrrrrrrrrrrrrrrrrradfads33333333333333rrd",
                    sizeof("rrrrrrrrrrrrrrrrrrrradfads33333333333333rrd") - 1);

            if (total_count % 5000000 == 0)
            {
                static double last = Now();
                double now = Now();
                if (total_count > 0)
                {
                    LogInfo() << "channel:<" << (long long)channel_id << "> "
                        << ChannelDesc(channel.channel_type_) << " emtpy write :<"
                        << channel.device_size_ << "> test " << 5000000 / (now - last) << " line/sec. ";
                    last = now;
                    break;
                }
            }
        } while (++total_count);
    }

    for (int channel_id = 1; channel_id < logger.shm_->channel_size_; channel_id++)
    {
        total_count = 0;
        FNLog::Channel& channel = logger.shm_->channels_[channel_id];
        do
        {
            int filter = 20;
            LOG_STREAM_DEFAULT_LOGGER(channel_id, FNLog::PRIORITY_FATAL, filter, filter, FNLog::LOG_PREFIX_NULL)
                .write_buffer("rrrrrrrrrrrrrrrrrrrradfads33333333333333rrd",
                    sizeof("rrrrrrrrrrrrrrrrrrrradfads33333333333333rrd") - 1);

            if (total_count % 5000000 == 0)
            {
                static double last = Now();
                double now = Now();
                if (total_count > 0)
                {
                    LogInfo() << "channel:<" << (long long)channel_id << "> "
                        << ChannelDesc(channel.channel_type_) << " emtpy write :<"
                        << channel.device_size_ << "> test " << 5000000 / (now - last) << " line/sec. ";
                    last = now;
                    break;
                }
            }
        } while (++total_count);
    }

    for (int channel_id = 0; channel_id < logger.shm_->channel_size_; channel_id++)
    {
        FNLog::Channel& channel = logger.shm_->channels_[channel_id];
        for (int field = 0; field < FNLog::CHANNEL_LOG_MAX_ID; field++)
        {
            if (FNLog::AtomicLoadChannelLog(channel, field) <= 0)
            {
                continue;
            }
            LogInfoStream(0, 1, 0) << "channel[" << channel_id << "] log field[" << field << "]:" << FNLog::AtomicLoadChannelLog(channel, field);
        }
    }
    LogAlarmStream(0, 1, 0) << "finish";
    return 0;
}

