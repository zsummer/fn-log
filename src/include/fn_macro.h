/*
 *
 * MIT License
 *
 * Copyright (C) 2019 YaweiZhang <yawei.zhang@foxmail.com>.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * ===============================================================================
 *
 * (end of COPYRIGHT)
 */


 /*
  * AUTHORS:  YaweiZhang <yawei.zhang@foxmail.com>
  * VERSION:  1.0.0
  * PURPOSE:  fn-log is a cpp-based logging utility.
  * CREATION: 2019.4.20
  * RELEASED: 2019.6.27
  * QQGROUP:  524700770
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
   # write info log to pname_info.log 
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
        priority: info
        file: "$PNAME_info"
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

    inline int FastStartSimpleLogger()
    {
        static const std::string default_config_text =
            R"----(
 # default channel 0
   # write full log to pname.log 
   # view  info log to screen 
 # sync channel 1 
   # write full log to pname.log
   # view  info log to screen 

 - channel: 0
    sync: async
    -device: 0
        disable: false
        out_type: file
        file: "$PNAME"
        rollback: 4
        limit_size: 100 m #only support M byte
    -device:1
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
    -device:1
        disable: false
        out_type: screen
        priority: info 
)----";
        return FastStartDefaultLogger(default_config_text);
    }

    inline int FastStartDebugLogger()
    {
        int ret = FastStartSimpleLogger();
        if (ret == 0)
        {
            SetDeviceConfig(GetDefaultLogger(), 0, 1, DEVICE_CFG_PRIORITY, PRIORITY_TRACE);
            SetDeviceConfig(GetDefaultLogger(), 1, 1, DEVICE_CFG_PRIORITY, PRIORITY_TRACE);
        }
        return ret;
    }
}

//--------------------BASE STREAM MACRO ---------------------------

#define LOG_STREAM_ORIGIN(logger, channel, priority, category, prefix) \
FNLog::LogStream(logger, channel, priority, category, \
__FILE__, sizeof(__FILE__) - 1, \
__LINE__, __FUNCTION__, sizeof(__FUNCTION__) -1, prefix)

#define LOG_STREAM_DEFAULT_LOGGER(channel, priority, category, prefix) \
    LOG_STREAM_ORIGIN(FNLog::GetDefaultLogger(), channel, priority, category, prefix)

#define LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel, priority, category) \
    LOG_STREAM_DEFAULT_LOGGER(channel, priority, category, FNLog::LOG_PREFIX_ALL)


//--------------------CPP STREAM STYLE FORMAT ---------------------------
#define LogTraceStream(channel_id, category) LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_TRACE, category)
#define LogDebugStream(channel_id, category) LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_DEBUG, category)
#define LogInfoStream(channel_id,  category) LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_INFO,  category)
#define LogWarnStream(channel_id,  category) LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_WARN,  category)
#define LogErrorStream(channel_id, category) LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_ERROR, category)
#define LogAlarmStream(channel_id, category) LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_ALARM, category)
#define LogFatalStream(channel_id, category) LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_FATAL, category)

#define LogTrace() LogTraceStream(0, 0)
#define LogDebug() LogDebugStream(0, 0)
#define LogInfo()  LogInfoStream(0, 0)
#define LogWarn()  LogWarnStream(0, 0)
#define LogError() LogErrorStream(0, 0)
#define LogAlarm() LogAlarmStream(0, 0)
#define LogFatal() LogFatalStream(0, 0)


//--------------------CPP TEMPLATE STYLE FORMAT ---------------------------
inline FNLog::LogStream& LogTemplatePack(FNLog::LogStream&& ls)
{
    return ls;
}
template<typename ... Args>
FNLog::LogStream& LogTemplatePack(FNLog::LogStream&& ls, Args&& ... args)
{
    char buff[] = { (ls << args, '\0') ... };
    (void)buff;
    return ls;
}

#define LogTracePack(channel_id, category, ...)  LogTemplatePack(LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_TRACE, category), ##__VA_ARGS__)
#define LogDebugPack(channel_id, category, ...)  LogTemplatePack(LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_DEBUG, category), ##__VA_ARGS__)
#define LogInfoPack(channel_id,  category, ...)  LogTemplatePack(LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_INFO,  category), ##__VA_ARGS__)
#define LogWarnPack(channel_id,  category, ...)  LogTemplatePack(LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_WARN,  category), ##__VA_ARGS__)
#define LogErrorPack(channel_id, category, ...)  LogTemplatePack(LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_ERROR, category), ##__VA_ARGS__)
#define LogAlarmPack(channel_id, category, ...)  LogTemplatePack(LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_ALARM, category), ##__VA_ARGS__)
#define LogFatalPack(channel_id, category, ...)  LogTemplatePack(LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_FATAL, category), ##__VA_ARGS__)

#define PackTrace(...) LogTracePack(0, 0, ##__VA_ARGS__)
#define PackDebug(...) LogDebugPack(0, 0, ##__VA_ARGS__)
#define PackInfo( ...) LogInfoPack( 0, 0, ##__VA_ARGS__)
#define PackWarn( ...) LogWarnPack( 0, 0, ##__VA_ARGS__)
#define PackError(...) LogErrorPack(0, 0, ##__VA_ARGS__)
#define PackAlarm(...) LogAlarmPack(0, 0, ##__VA_ARGS__)
#define PackFatal(...) LogFatalPack(0, 0, ##__VA_ARGS__)


//--------------------CPP MACRO STREAM STYLE FORMAT ---------------------------

#define LOG_TRACE(channel_id, category, log) LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_TRACE, category) << log
#define LOG_DEBUG(channel_id, category, log) LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_DEBUG, category) << log
#define LOG_INFO(channel_id,  category, log) LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_INFO,  category) << log
#define LOG_WARN(channel_id,  category, log) LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_WARN,  category) << log
#define LOG_ERROR(channel_id, category, log) LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_ERROR, category) << log
#define LOG_ALARM(channel_id, category, log) LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_ALARM, category) << log
#define LOG_FATAL(channel_id, category, log) LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_FATAL, category) << log

#define LOGT(log) LOG_TRACE(0, 0, log)
#define LOGD(log) LOG_DEBUG(0, 0, log)
#define LOGI(log) LOG_INFO(0, 0, log)
#define LOGW(log) LOG_WARN(0, 0, log)
#define LOGE(log) LOG_ERROR(0, 0, log)
#define LOGA(log) LOG_ALARM(0, 0, log)
#define LOGF(log) LOG_FATAL(0, 0, log)


//--------------------C STYLE FORMAT ---------------------------
#ifdef _WIN32
#define LOG_FORMAT(channel_id, priority, category, prefix, logformat, ...) \
do{ \
    FNLog::LogStream __log_stream(LOG_STREAM_DEFAULT_LOGGER(channel_id, priority, category, prefix));\
    if (__log_stream.log_data_)\
    {\
        int __log_len = _snprintf_s(__log_stream.log_data_ ->content_ + __log_stream.log_data_ ->content_len_, FNLog::LogData::LOG_SIZE - __log_stream.log_data_ ->content_len_, _TRUNCATE, logformat, ##__VA_ARGS__); \
        if (__log_len < 0) __log_len = 0; \
        __log_stream.log_data_ ->content_len_ += __log_len; \
    }\
} while (0)
#else
#define LOG_FORMAT(channel_id, priority, category, prefix, logformat, ...) \
do{ \
    FNLog::LogStream __log_stream(LOG_STREAM_DEFAULT_LOGGER(channel_id, priority, category, prefix));\
    if (__log_stream.log_data_)\
    {\
        int __log_len = snprintf(__log_stream.log_data_ ->content_ + __log_stream.log_data_ ->content_len_, FNLog::LogData::LOG_SIZE - __log_stream.log_data_ ->content_len_, logformat, ##__VA_ARGS__); \
        if (__log_len < 0) __log_len = 0; \
        __log_stream.log_data_ ->content_len_ += __log_len; \
    }\
} while (0)
#endif

#define LOGFMT_TRACE(channel_id, category, fmt, ...)  LOG_FORMAT(channel_id, FNLog::PRIORITY_TRACE, category, FNLog::LOG_PREFIX_ALL, fmt, ##__VA_ARGS__)
#define LOGFMT_DEBUG(channel_id, category, fmt, ...)  LOG_FORMAT(channel_id, FNLog::PRIORITY_DEBUG, category, FNLog::LOG_PREFIX_ALL, fmt, ##__VA_ARGS__)
#define LOGFMT_INFO( channel_id, category, fmt, ...)  LOG_FORMAT(channel_id, FNLog::PRIORITY_INFO,  category, FNLog::LOG_PREFIX_ALL, fmt, ##__VA_ARGS__)
#define LOGFMT_WARN( channel_id, category, fmt, ...)  LOG_FORMAT(channel_id, FNLog::PRIORITY_WARN,  category, FNLog::LOG_PREFIX_ALL, fmt, ##__VA_ARGS__)
#define LOGFMT_ERROR(channel_id, category, fmt, ...)  LOG_FORMAT(channel_id, FNLog::PRIORITY_ERROR, category, FNLog::LOG_PREFIX_ALL, fmt, ##__VA_ARGS__)
#define LOGFMT_ALARM(channel_id, category, fmt, ...)  LOG_FORMAT(channel_id, FNLog::PRIORITY_ALARM, category, FNLog::LOG_PREFIX_ALL, fmt, ##__VA_ARGS__)
#define LOGFMT_FATAL(channel_id, category, fmt, ...)  LOG_FORMAT(channel_id, FNLog::PRIORITY_FATAL, category, FNLog::LOG_PREFIX_ALL, fmt, ##__VA_ARGS__)
#define LOGFMTT(fmt, ...) LOGFMT_TRACE(0, 0, fmt,  ##__VA_ARGS__)
#define LOGFMTD(fmt, ...) LOGFMT_DEBUG(0, 0, fmt,  ##__VA_ARGS__)
#define LOGFMTI(fmt, ...) LOGFMT_INFO( 0, 0, fmt,  ##__VA_ARGS__)
#define LOGFMTW(fmt, ...) LOGFMT_WARN( 0, 0, fmt,  ##__VA_ARGS__)
#define LOGFMTE(fmt, ...) LOGFMT_ERROR(0, 0, fmt,  ##__VA_ARGS__)
#define LOGFMTA(fmt, ...) LOGFMT_ALARM(0, 0, fmt,  ##__VA_ARGS__)
#define LOGFMTF(fmt, ...) LOGFMT_FATAL(0, 0, fmt,  ##__VA_ARGS__)


#endif
