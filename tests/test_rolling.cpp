


#define FN_LOG_MAX_CHANNEL_SIZE 4
#define FN_LOG_MAX_LOG_SIZE 1000
#define FN_LOG_MAX_LOG_QUEUE_SIZE 100000

#include "fn_log.h"

#define FNLOG_ASSERT(expr, prefix) \
{ \
    if (expr) \
    { \
        LogInfo() << "test " << prefix << " success."; \
    } \
    else \
    {   \
        LogError() << "test " << prefix << " failed. warn: need to clean all log file when begin this test!"; \
        return __LINE__ * -1; \
    } \
}


static const std::string rollback1 =
R"----(
 - channel: 0
    sync: sync
    -device: 0
        disable: false
        out_type: screen

 - channel: 1
    sync: sync
    -device: 0
        disable: false
        out_type: file
        priority: trace
        path: "./"
        file: "$PNAME"
        rollback: 1
        stuff: false
        limit_size: 2 m #only support M byte
)----";

static const std::string rollback2 =
R"----(
 - channel: 0
    sync: sync
    -device: 0
        disable: false
        out_type: screen

 - channel: 1
    sync: sync
    -device: 0
        disable: false
        out_type: file
        priority: trace
        path: "./"
        file: "$PNAME"
        rollback: 1
        stuff: true
        limit_size: 2 m #only support M byte
)----";

static const std::string daily_rolling_stuff =
R"----(
 - channel: 0
    sync: sync
    -device: 0
        disable: false
        out_type: screen

 - channel: 1
    sync: sync
    -device: 0
        disable: false
        out_type: file
        priority: trace
        path: "./"
        file: "$PNAME$DAY$HOUR"
        rolldaily: true
        stuff: true
        limit_size: 2 m #only support M byte
)----";

static const std::string daily_rolling_unstuff =
R"----(
 - channel: 0
    sync: sync
    -device: 0
        disable: false
        out_type: screen

 - channel: 1
    sync: sync
    -device: 0
        disable: false
        out_type: file
        priority: trace
        path: "./"
        file: "$PNAME$DAY$HOUR"
        rolldaily: true
        stuff: false
        limit_size: 2 m #only support M byte
)----";


static const std::string hourly_rolling_stuff =
R"----(
 - channel: 0
    sync: sync
    -device: 0
        disable: false
        out_type: screen

 - channel: 1
    sync: sync
    -device: 0
        disable: false
        out_type: file
        priority: trace
        path: "./"
        file: "$PNAME$DAY$HOUR$MIN"
        rollhourly: true
        stuff: true
        limit_size: 2 m #only support M byte
)----";

static const std::string hourly_rolling_unstuff =
R"----(
 - channel: 0
    sync: sync
    -device: 0
        disable: false
        out_type: screen

 - channel: 1
    sync: sync
    -device: 0
        disable: false
        out_type: file
        priority: trace
        path: "./"
        file: "$PNAME$DAY$HOUR$MIN"
        rollhourly: true
        stuff: false
        limit_size: 2 m #only support M byte
)----";

static const std::string daily_dir =
R"----(
 - channel: 0
    sync: sync
    -device: 0
        disable: false
        out_type: screen

 - channel: 1
    sync: sync
    -device: 0
        disable: false
        out_type: file
        priority: trace
        path: "./$PNAME_$YEAR-$MON-$DAY"
        file: "$PNAME$DAY$HOUR$MIN"
        rolldaily: true
        stuff: false
        limit_size: 2 m #only support M byte
)----";


static const std::string daily_dir_stuff =
R"----(
 - channel: 0
    sync: sync
    -device: 0
        disable: false
        out_type: screen

 - channel: 1
    sync: sync
    -device: 0
        disable: false
        out_type: file
        priority: trace
        path: "./$PNAME_$YEAR-$MON-$DAY"
        file: "$PNAME"
        rolldaily: true
        stuff: true
        limit_size: 2 m #only support M byte
)----";


void StufFileData()
{
    char buf[1500];
    memset(buf, 0, 1500);
    for (int i = 0; i < 1400; i++)
    {
        buf[i] = '0' + (i % 10);
    }
    for (int i = 0; i < 1200; i++)
    {
        LOG_STREAM_DEFAULT_LOGGER(1, FNLog::PRIORITY_DEBUG, 0, 0, 0).write_buffer(buf, 1200);
    }
}

int main(int argc, char* argv[])
{
   
    std::string rollback_file = "./test_rolling.log";
    FNLog::FileHandler::remove_file(rollback_file);
    FNLog::FileHandler::remove_file(rollback_file + ".1");
    FNLog::FileHandler::remove_file(rollback_file + ".2");

    //只滚一个  
    int ret = FNLog::FastStartDefaultLogger(rollback1);
    FNLOG_ASSERT(ret == 0, "");
    FNLOG_ASSERT(!FNLog::FileHandler::is_file(rollback_file), "");
    FNLOG_ASSERT(!FNLog::FileHandler::is_file(rollback_file + ".1"), "");
    FNLOG_ASSERT(!FNLog::FileHandler::is_file(rollback_file + ".2"), "");
    StufFileData();
    FNLOG_ASSERT(FNLog::FileHandler::is_file(rollback_file), "");
    FNLOG_ASSERT(!FNLog::FileHandler::is_file(rollback_file + ".1"), "");
    FNLOG_ASSERT(!FNLog::FileHandler::is_file(rollback_file + ".2"), "");
    StufFileData();
    FNLOG_ASSERT(FNLog::FileHandler::is_file(rollback_file), "");
    FNLOG_ASSERT(FNLog::FileHandler::is_file(rollback_file + ".1"), "");
    FNLOG_ASSERT(!FNLog::FileHandler::is_file(rollback_file + ".2"), "");
    StufFileData();
    StufFileData();
    StufFileData();
    StufFileData();
    StufFileData();
    FNLOG_ASSERT(FNLog::FileHandler::is_file(rollback_file), "");
    FNLOG_ASSERT(FNLog::FileHandler::is_file(rollback_file + ".1"), "");
    FNLOG_ASSERT(!FNLog::FileHandler::is_file(rollback_file + ".2"), "");



    //残留日志写满载回滚 
    FNLog::StopLogger(FNLog::GetDefaultLogger());
    FNLog::FileHandler::remove_file(rollback_file);
    FNLog::FileHandler::remove_file(rollback_file + ".1");
    FNLog::FileHandler::remove_file(rollback_file + ".2");

    ret = FNLog::FastStartDefaultLogger(rollback1);
    FNLOG_ASSERT(ret == 0, "");
    StufFileData();
    FNLog::StopLogger(FNLog::GetDefaultLogger());
    ret = FNLog::FastStartDefaultLogger(rollback1);
    FNLOG_ASSERT(ret == 0, "");
    LOG_STREAM_DEFAULT_LOGGER(1, FNLog::PRIORITY_DEBUG, 0, 0, 0) << "any"; //new log file
    FNLOG_ASSERT(FNLog::FileHandler::is_file(rollback_file), "");
    FNLOG_ASSERT(FNLog::FileHandler::is_file(rollback_file + ".1"), "");
    FNLOG_ASSERT(!FNLog::FileHandler::is_file(rollback_file + ".2"), "");
    StufFileData();
    FNLOG_ASSERT(FNLog::FileHandler::is_file(rollback_file), "");
    FNLOG_ASSERT(FNLog::FileHandler::is_file(rollback_file + ".1"), "");
    FNLOG_ASSERT(!FNLog::FileHandler::is_file(rollback_file + ".2"), "");
    StufFileData();
    FNLOG_ASSERT(FNLog::FileHandler::is_file(rollback_file), "");
    FNLOG_ASSERT(FNLog::FileHandler::is_file(rollback_file + ".1"), "");
    FNLOG_ASSERT(!FNLog::FileHandler::is_file(rollback_file + ".2"), "");


    //残留日志写满载回滚 
    FNLog::StopLogger(FNLog::GetDefaultLogger());
    FNLog::FileHandler::remove_file(rollback_file);
    FNLog::FileHandler::remove_file(rollback_file + ".1");
    FNLog::FileHandler::remove_file(rollback_file + ".2");

    ret = FNLog::FastStartDefaultLogger(rollback2);
    FNLOG_ASSERT(ret == 0, "");
    StufFileData();
    FNLog::StopLogger(FNLog::GetDefaultLogger());
    ret = FNLog::FastStartDefaultLogger(rollback2);
    FNLOG_ASSERT(ret == 0, "");
    LOG_STREAM_DEFAULT_LOGGER(1, FNLog::PRIORITY_DEBUG, 0, 0, 0) << "any";
    FNLOG_ASSERT(FNLog::FileHandler::is_file(rollback_file), "");
    FNLOG_ASSERT(!FNLog::FileHandler::is_file(rollback_file + ".1"), "");
    FNLOG_ASSERT(!FNLog::FileHandler::is_file(rollback_file + ".2"), "");
    StufFileData();
    FNLOG_ASSERT(FNLog::FileHandler::is_file(rollback_file), "");
    FNLOG_ASSERT(FNLog::FileHandler::is_file(rollback_file + ".1"), "");
    FNLOG_ASSERT(!FNLog::FileHandler::is_file(rollback_file + ".2"), "");

    FNLog::StopLogger(FNLog::GetDefaultLogger());

    //
    if (true)
    {
        std::string daily_file;
        ret = FNLog::FastStartDefaultLogger(daily_rolling_stuff);
        FNLOG_ASSERT(ret == 0, "");
        LOG_STREAM_DEFAULT_LOGGER(1, FNLog::PRIORITY_DEBUG, 0, 0, 0) << "any";

        FNLog::FileHandler& writer = FNLog::GetDefaultLogger().file_handles_[1 * FNLog::Channel::MAX_DEVICE_SIZE + 0];
        FNLOG_ASSERT(writer.is_open(), "");
        long long writed = FNLog::AtomicLoadDeviceLog(FNLog::GetDefaultLogger().shm_->channels_[1], 0, FNLog::DEVICE_LOG_CUR_FILE_SIZE);
        if (true)
        {
            FNLog::LogStream ls = std::move( LOG_STREAM_DEFAULT_LOGGER(1, FNLog::PRIORITY_DEBUG, 0, 0, 0) <<"any");
            ls.log_data_->timestamp_ += 24 * 3600;
        }
        FNLOG_ASSERT(writer.is_open(), "");
        long long now_writed = FNLog::AtomicLoadDeviceLog(FNLog::GetDefaultLogger().shm_->channels_[1], 0, FNLog::DEVICE_LOG_CUR_FILE_SIZE);

        FNLOG_ASSERT(now_writed == writed, "");

    }

    FNLog::StopLogger(FNLog::GetDefaultLogger());

    //
    if (true)
    {
        std::string daily_file;
        ret = FNLog::FastStartDefaultLogger(daily_rolling_unstuff);
        FNLOG_ASSERT(ret == 0, "");
        LOG_STREAM_DEFAULT_LOGGER(1, FNLog::PRIORITY_DEBUG, 0, 0, 0) << "any";

        FNLog::FileHandler& writer = FNLog::GetDefaultLogger().file_handles_[1 * FNLog::Channel::MAX_DEVICE_SIZE + 0];
        FNLOG_ASSERT(writer.is_open(), "");
        long long writed = FNLog::AtomicLoadDeviceLog(FNLog::GetDefaultLogger().shm_->channels_[1], 0, FNLog::DEVICE_LOG_CUR_FILE_SIZE);
        if (true)
        {
            FNLog::LogStream ls = std::move(LOG_STREAM_DEFAULT_LOGGER(1, FNLog::PRIORITY_DEBUG, 0, 0, 0) << "any");
            ls.log_data_->timestamp_ += 24 * 3600;
        }
        FNLOG_ASSERT(writer.is_open(), "");
        long long now_writed = FNLog::AtomicLoadDeviceLog(FNLog::GetDefaultLogger().shm_->channels_[1], 0, FNLog::DEVICE_LOG_CUR_FILE_SIZE);

        FNLOG_ASSERT(now_writed <= writed, "");

    }

    FNLog::StopLogger(FNLog::GetDefaultLogger());
    //
    if (true)
    {
        std::string daily_file;
        ret = FNLog::FastStartDefaultLogger(hourly_rolling_stuff);
        FNLOG_ASSERT(ret == 0, "");
        LOG_STREAM_DEFAULT_LOGGER(1, FNLog::PRIORITY_DEBUG, 0, 0, 0) << "any";

        FNLog::FileHandler& writer = FNLog::GetDefaultLogger().file_handles_[1 * FNLog::Channel::MAX_DEVICE_SIZE + 0];
        FNLOG_ASSERT(writer.is_open(), "");
        long long writed = FNLog::AtomicLoadDeviceLog(FNLog::GetDefaultLogger().shm_->channels_[1], 0, FNLog::DEVICE_LOG_CUR_FILE_SIZE);
        if (true)
        {
            FNLog::LogStream ls = std::move(LOG_STREAM_DEFAULT_LOGGER(1, FNLog::PRIORITY_DEBUG, 0, 0, 0) << "any");
            ls.log_data_->timestamp_ += 3600;
        }
        FNLOG_ASSERT(writer.is_open(), "");
        long long now_writed = FNLog::AtomicLoadDeviceLog(FNLog::GetDefaultLogger().shm_->channels_[1], 0, FNLog::DEVICE_LOG_CUR_FILE_SIZE);

        FNLOG_ASSERT(now_writed == writed, "");

    }


    FNLog::StopLogger(FNLog::GetDefaultLogger());
    //
    if (true)
    {
        std::string daily_file;
        ret = FNLog::FastStartDefaultLogger(hourly_rolling_unstuff);
        FNLOG_ASSERT(ret == 0, "");
        LOG_STREAM_DEFAULT_LOGGER(1, FNLog::PRIORITY_DEBUG, 0, 0, 0) << "any";

        FNLog::FileHandler& writer = FNLog::GetDefaultLogger().file_handles_[1 * FNLog::Channel::MAX_DEVICE_SIZE + 0];
        FNLOG_ASSERT(writer.is_open(), "");
        long long writed = FNLog::AtomicLoadDeviceLog(FNLog::GetDefaultLogger().shm_->channels_[1], 0, FNLog::DEVICE_LOG_CUR_FILE_SIZE);
        if (true)
        {
            FNLog::LogStream ls = std::move(LOG_STREAM_DEFAULT_LOGGER(1, FNLog::PRIORITY_DEBUG, 0, 0, 0) << "any");
            ls.log_data_->timestamp_ += 3600;
        }
        FNLOG_ASSERT(writer.is_open(), "");
        long long now_writed = FNLog::AtomicLoadDeviceLog(FNLog::GetDefaultLogger().shm_->channels_[1], 0, FNLog::DEVICE_LOG_CUR_FILE_SIZE);

        FNLOG_ASSERT(now_writed <= writed, "");

    }


    FNLog::StopLogger(FNLog::GetDefaultLogger());
    //
    if (true)
    {
        std::string daily_file;
        ret = FNLog::FastStartDefaultLogger(daily_dir);
        FNLOG_ASSERT(ret == 0, "");
        LOG_STREAM_DEFAULT_LOGGER(1, FNLog::PRIORITY_DEBUG, 0, 0, 0) << "any";

        FNLog::FileHandler& writer = FNLog::GetDefaultLogger().file_handles_[1 * FNLog::Channel::MAX_DEVICE_SIZE + 0];
        FNLOG_ASSERT(writer.is_open(), "");
        long long writed = FNLog::AtomicLoadDeviceLog(FNLog::GetDefaultLogger().shm_->channels_[1], 0, FNLog::DEVICE_LOG_CUR_FILE_SIZE);
        if (true)
        {
            FNLog::LogStream ls = std::move(LOG_STREAM_DEFAULT_LOGGER(1, FNLog::PRIORITY_DEBUG, 0, 0, 0) << "any");
            ls.log_data_->timestamp_ += 24*3600;
        }
        FNLOG_ASSERT(writer.is_open(), "");
        long long now_writed = FNLog::AtomicLoadDeviceLog(FNLog::GetDefaultLogger().shm_->channels_[1], 0, FNLog::DEVICE_LOG_CUR_FILE_SIZE);

        FNLOG_ASSERT(now_writed <= writed, "");

    }


    FNLog::StopLogger(FNLog::GetDefaultLogger());
    //dir date daily check  
    if (true)
    {
        std::string daily_file;
        ret = FNLog::FastStartDefaultLogger(daily_dir_stuff);
        FNLOG_ASSERT(ret == 0, "");
        LOG_STREAM_DEFAULT_LOGGER(1, FNLog::PRIORITY_DEBUG, 0, 0, 0) << "any";

        FNLog::FileHandler& writer = FNLog::GetDefaultLogger().file_handles_[1 * FNLog::Channel::MAX_DEVICE_SIZE + 0];
        FNLOG_ASSERT(writer.is_open(), "");
        long long writed = FNLog::AtomicLoadDeviceLog(FNLog::GetDefaultLogger().shm_->channels_[1], 0, FNLog::DEVICE_LOG_CUR_FILE_SIZE);
        if (true)
        {
            FNLog::LogStream ls = std::move(LOG_STREAM_DEFAULT_LOGGER(1, FNLog::PRIORITY_DEBUG, 0, 0, 0) << "any");
            ls.log_data_->timestamp_ += 24 * 3600;
        }
        FNLOG_ASSERT(writer.is_open(), "");
        long long now_writed = FNLog::AtomicLoadDeviceLog(FNLog::GetDefaultLogger().shm_->channels_[1], 0, FNLog::DEVICE_LOG_CUR_FILE_SIZE);

        FNLOG_ASSERT(now_writed <= writed, "");

    }
    volatile long long cnt = FNLog::AtomicLoadDeviceLog(FNLog::GetDefaultLogger().shm_->channels_[1], 0, FNLog::DEVICE_LOG_LAST_TRY_CREATE_CNT);   
    (void)cnt;
    return 0;
}

