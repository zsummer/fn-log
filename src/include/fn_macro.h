
/*
* Copyright (C) 2019 YaweiZhang <yawei.zhang@foxmail.com>.
* All rights reserved
* This file is part of the fn-log, used MIT License.
*/


#pragma once
#ifndef _FN_LOG_MACRO_H_
#define _FN_LOG_MACRO_H_

#include "fn_data.h"
#include "fn_parse.h"
#include "fn_load.h"
#include "fn_core.h"
#include "fn_stream.h"

namespace FNLog
{

    inline Logger& GetDefaultLogger()
    {
        static Logger logger;
        return logger;
    }

    inline int LoadAndStartDefaultLogger(const std::string& config_path)
    {
        int ret = LoadAndStartLogger(GetDefaultLogger(), config_path);
        if (ret != 0)
        {
            printf("load auto start default logger error. ret:<%d>.\n", ret);
            return ret;
        }
        return 0;
    }

    inline int FastStartDefaultLogger(const std::string& config_text)
    {
        int ret = ParseAndStartLogger(GetDefaultLogger(), config_text);
        if (ret != 0)
        {
            printf("fast start default logger error. ret:<%d>.\n", ret);
            return ret;
        }
        return 0;
    }


    inline int FastStartDefaultLogger()
    {
        static const std::string default_config_text =
            R"----(
 # default channel 0
   # write full log to pname.log 
   # write error log to pname_error.log 
   # view  info log to screen 
 # sync channel 1 
   # write full log to pname.log
   # write info log to pname_info.log
   # view  info log to screen 

 - channel: 0
    sync: async
    -device: 0
        disable: false
        out_type: file
        file: "$PNAME"
        rollback: 4
        limit_size: 100 m #only support M byte
    -device: 1
        disable: false
        out_type: file
        priority: error
        file: "$PNAME_error"
        rollback: 4
        limit_size: 100 m #only support M byte
    -device:2
        disable: false
        out_type: screen
        priority: info
 - channel: 1
    sync: sync
    -device: 0
        disable: false
        out_type: file
        file: "$PNAME_sync"
        rollback: 4
        limit_size: 100 m #only support M byte
    -device: 1
        disable: false
        out_type: file
        priority: info
        file: "$PNAME_sync_info"
        rollback: 4
        limit_size: 100 m #only support M byte
    -device:2
        disable: false
        out_type: screen
        priority: info 
)----";
        return FastStartDefaultLogger(default_config_text);
    }

    inline int FastStartDebugLogger()
    {
        int ret = FastStartDefaultLogger();
        if (ret == 0)
        {
            BatchSetDeviceConfig(GetDefaultLogger(), DEVICE_OUT_FILE, DEVICE_CFG_PRIORITY, PRIORITY_TRACE);
            SetDeviceConfig(GetDefaultLogger(), 0, 1, DEVICE_CFG_PRIORITY, PRIORITY_ERROR); //error file is still error file    
            BatchSetDeviceConfig(GetDefaultLogger(), DEVICE_OUT_SCREEN, DEVICE_CFG_PRIORITY, PRIORITY_DEBUG);
        }
        return ret;
    }
}

//--------------------BASE STREAM MACRO ---------------------------

//temporary LogStream  
#define LOG_STREAM_ORIGIN(logger, channel, priority, category, identify, prefix, ignore_check) \
FNLog::LogStream(logger, channel, priority, category, identify,\
__FILE__, sizeof(__FILE__) - 1, \
__LINE__, __FUNCTION__, sizeof(__FUNCTION__) -1, prefix, ignore_check)

//
#define LOG_STREAM_ORIGIN_REF(logger, channel, priority, category, identify, prefix, ignore_check) \
    LOG_STREAM_ORIGIN(logger, channel, priority, category, identify, prefix, ignore_check).self()

#define LOG_STREAM_DEFAULT_LOGGER(channel, priority, category, identify, prefix) \
    LOG_STREAM_ORIGIN_REF(FNLog::GetDefaultLogger(), channel, priority, category, identify, prefix, false)

#define LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel, priority, category, identify) \
    LOG_STREAM_ORIGIN_REF(FNLog::GetDefaultLogger(), channel, priority, category, identify, FNLog::LOG_PREFIX_DEFAULT, false)


//--------------------CPP STREAM STYLE FORMAT ---------------------------
#define LogTraceStream(channel, category, identify) LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel, FNLog::PRIORITY_TRACE, category, identify)
#define LogDebugStream(channel, category, identify) LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel, FNLog::PRIORITY_DEBUG, category, identify)
#define LogInfoStream(channel,  category, identify) LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel, FNLog::PRIORITY_INFO,  category, identify)
#define LogWarnStream(channel,  category, identify) LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel, FNLog::PRIORITY_WARN,  category, identify)
#define LogErrorStream(channel, category, identify) LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel, FNLog::PRIORITY_ERROR, category, identify)
#define LogAlarmStream(channel, category, identify) LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel, FNLog::PRIORITY_ALARM, category, identify)
#define LogFatalStream(channel, category, identify) LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel, FNLog::PRIORITY_FATAL, category, identify)

#define LogTrace() LogTraceStream(0, 0, 0)
#define LogDebug() LogDebugStream(0, 0, 0)
#define LogInfo()  LogInfoStream(0, 0, 0)
#define LogWarn()  LogWarnStream(0, 0, 0)
#define LogError() LogErrorStream(0, 0, 0)
#define LogAlarm() LogAlarmStream(0, 0, 0)
#define LogFatal() LogFatalStream(0, 0, 0)


//--------------------CPP TEMPLATE STYLE FORMAT ---------------------------
inline FNLog::LogStream& LogTemplatePack(FNLog::LogStream& ls)
{
    return ls;
}
template<typename ... Args>
FNLog::LogStream& LogTemplatePack(FNLog::LogStream& ls, Args&& ... args)
{
    char buff[] = { (ls << args, '\0') ... };
    (void)buff;
    return ls;
}

#define LogTracePack(channel, category, identify, ...)  LogTemplatePack(LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel, FNLog::PRIORITY_TRACE, category, identify), ##__VA_ARGS__)
#define LogDebugPack(channel, category, identify, ...)  LogTemplatePack(LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel, FNLog::PRIORITY_DEBUG, category, identify), ##__VA_ARGS__)
#define LogInfoPack(channel,  category, identify, ...)  LogTemplatePack(LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel, FNLog::PRIORITY_INFO,  category, identify), ##__VA_ARGS__)
#define LogWarnPack(channel,  category, identify, ...)  LogTemplatePack(LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel, FNLog::PRIORITY_WARN,  category, identify), ##__VA_ARGS__)
#define LogErrorPack(channel, category, identify, ...)  LogTemplatePack(LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel, FNLog::PRIORITY_ERROR, category, identify), ##__VA_ARGS__)
#define LogAlarmPack(channel, category, identify, ...)  LogTemplatePack(LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel, FNLog::PRIORITY_ALARM, category, identify), ##__VA_ARGS__)
#define LogFatalPack(channel, category, identify, ...)  LogTemplatePack(LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel, FNLog::PRIORITY_FATAL, category, identify), ##__VA_ARGS__)

#define PackTrace(...) LogTracePack(0, 0, 0, ##__VA_ARGS__)
#define PackDebug(...) LogDebugPack(0, 0, 0, ##__VA_ARGS__)
#define PackInfo( ...) LogInfoPack( 0, 0, 0, ##__VA_ARGS__)
#define PackWarn( ...) LogWarnPack( 0, 0, 0, ##__VA_ARGS__)
#define PackError(...) LogErrorPack(0, 0, 0, ##__VA_ARGS__)
#define PackAlarm(...) LogAlarmPack(0, 0, 0, ##__VA_ARGS__)
#define PackFatal(...) LogFatalPack(0, 0, 0, ##__VA_ARGS__)


//--------------------CPP MACRO STREAM STYLE FORMAT ---------------------------
#define LOG_STREAM_FUNC(logger, channel, priority, category, identify, prefix, log) \
do{ \
    if (FNLog::BlockInput(FNLog::GetDefaultLogger(), channel, priority, category, identify))  \
    { \
        break;   \
    } \
    FNLog::LogStream __log_stream(LOG_STREAM_ORIGIN(FNLog::GetDefaultLogger(), channel, priority, category, identify, prefix, true));\
    if (__log_stream.log_data_)\
    {\
        __log_stream << log;\
    }\
} while (0)

#define LOG_TRACE(channel, category, identify, log) LOG_STREAM_FUNC(FNLog::GetDefaultLogger(), channel, FNLog::PRIORITY_TRACE, category, identify, FNLog::LOG_PREFIX_DEFAULT, log)
#define LOG_DEBUG(channel, category, identify, log) LOG_STREAM_FUNC(FNLog::GetDefaultLogger(), channel, FNLog::PRIORITY_DEBUG, category, identify, FNLog::LOG_PREFIX_DEFAULT, log)
#define LOG_INFO(channel,  category, identify, log) LOG_STREAM_FUNC(FNLog::GetDefaultLogger(), channel, FNLog::PRIORITY_INFO,  category, identify, FNLog::LOG_PREFIX_DEFAULT, log)
#define LOG_WARN(channel,  category, identify, log) LOG_STREAM_FUNC(FNLog::GetDefaultLogger(), channel, FNLog::PRIORITY_WARN,  category, identify, FNLog::LOG_PREFIX_DEFAULT, log)
#define LOG_ERROR(channel, category, identify, log) LOG_STREAM_FUNC(FNLog::GetDefaultLogger(), channel, FNLog::PRIORITY_ERROR, category, identify, FNLog::LOG_PREFIX_DEFAULT, log)
#define LOG_ALARM(channel, category, identify, log) LOG_STREAM_FUNC(FNLog::GetDefaultLogger(), channel, FNLog::PRIORITY_ALARM, category, identify, FNLog::LOG_PREFIX_DEFAULT, log)
#define LOG_FATAL(channel, category, identify, log) LOG_STREAM_FUNC(FNLog::GetDefaultLogger(), channel, FNLog::PRIORITY_FATAL, category, identify, FNLog::LOG_PREFIX_DEFAULT, log)

#define LOGT(log) LOG_TRACE(0, 0, 0, log)
#define LOGD(log) LOG_DEBUG(0, 0, 0, log)
#define LOGI(log) LOG_INFO(0,  0, 0, log)
#define LOGW(log) LOG_WARN(0,  0, 0, log)
#define LOGE(log) LOG_ERROR(0, 0, 0, log)
#define LOGA(log) LOG_ALARM(0, 0, 0, log)
#define LOGF(log) LOG_FATAL(0, 0, 0, log)


//--------------------C STYLE FORMAT ---------------------------

// function format warn:   void(int x1, int x2, const char *args, ...) __attribute__((format(printf, 3, 4)));    
#define LOG_FORMAT(channel, priority, category, identify, prefix, logformat, ...) \
do{ \
    if (FNLog::BlockInput(FNLog::GetDefaultLogger(), channel, priority, category, identify))  \
    { \
        break;   \
    } \
    FNLog::LogStream __log_stream(LOG_STREAM_ORIGIN(FNLog::GetDefaultLogger(), channel, priority, category, identify, prefix, true));\
    if (__log_stream.log_data_)\
    {\
        int __log_len = snprintf(__log_stream.log_data_ ->content_ + __log_stream.log_data_ ->content_len_, FNLog::LogData::LOG_SIZE - __log_stream.log_data_->content_len_, logformat, ##__VA_ARGS__); \
        if (__log_len < 0) __log_len = 0; \
        if (__log_len >= FNLog::LogData::LOG_SIZE - __log_stream.log_data_->content_len_) __log_len = FNLog::LogData::LOG_SIZE - __log_stream.log_data_->content_len_ -1; \
        __log_stream.log_data_ ->content_len_ += __log_len; \
    }\
} while (0)


#define LOGFMT_TRACE(channel, category, identify, fmt, ...)  LOG_FORMAT(channel, FNLog::PRIORITY_TRACE, category, identify, FNLog::LOG_PREFIX_DEFAULT, fmt, ##__VA_ARGS__)
#define LOGFMT_DEBUG(channel, category, identify, fmt, ...)  LOG_FORMAT(channel, FNLog::PRIORITY_DEBUG, category, identify, FNLog::LOG_PREFIX_DEFAULT, fmt, ##__VA_ARGS__)
#define LOGFMT_INFO( channel, category, identify, fmt, ...)  LOG_FORMAT(channel, FNLog::PRIORITY_INFO,  category, identify, FNLog::LOG_PREFIX_DEFAULT, fmt, ##__VA_ARGS__)
#define LOGFMT_WARN( channel, category, identify, fmt, ...)  LOG_FORMAT(channel, FNLog::PRIORITY_WARN,  category, identify, FNLog::LOG_PREFIX_DEFAULT, fmt, ##__VA_ARGS__)
#define LOGFMT_ERROR(channel, category, identify, fmt, ...)  LOG_FORMAT(channel, FNLog::PRIORITY_ERROR, category, identify, FNLog::LOG_PREFIX_DEFAULT, fmt, ##__VA_ARGS__)
#define LOGFMT_ALARM(channel, category, identify, fmt, ...)  LOG_FORMAT(channel, FNLog::PRIORITY_ALARM, category, identify, FNLog::LOG_PREFIX_DEFAULT, fmt, ##__VA_ARGS__)
#define LOGFMT_FATAL(channel, category, identify, fmt, ...)  LOG_FORMAT(channel, FNLog::PRIORITY_FATAL, category, identify, FNLog::LOG_PREFIX_DEFAULT, fmt, ##__VA_ARGS__)
#define LOGFMTT(fmt, ...) LOGFMT_TRACE(0, 0, 0, fmt,  ##__VA_ARGS__)
#define LOGFMTD(fmt, ...) LOGFMT_DEBUG(0, 0, 0, fmt,  ##__VA_ARGS__)
#define LOGFMTI(fmt, ...) LOGFMT_INFO( 0, 0, 0, fmt,  ##__VA_ARGS__)
#define LOGFMTW(fmt, ...) LOGFMT_WARN( 0, 0, 0, fmt,  ##__VA_ARGS__)
#define LOGFMTE(fmt, ...) LOGFMT_ERROR(0, 0, 0, fmt,  ##__VA_ARGS__)
#define LOGFMTA(fmt, ...) LOGFMT_ALARM(0, 0, 0, fmt,  ##__VA_ARGS__)
#define LOGFMTF(fmt, ...) LOGFMT_FATAL(0, 0, 0, fmt,  ##__VA_ARGS__)


#endif
