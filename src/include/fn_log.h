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
  * VERSION:  0.0.1
  * PURPOSE:  fn-log is a cpp-based logging utility.
  * CREATION: 2019.4.20
  * RELEASED: 2019.6.27
  * QQGROUP:  524700770
  */


#pragma once
#ifndef _FN_LOG_LOG_H_
#define _FN_LOG_LOG_H_

#include "fn_data.h"
#include "fn_parse.h"
#include "fn_load.h"
#include "fn_core.h"

namespace FNLog
{

    inline Logger& GetDefaultLogger()
    {
        static Logger logger;
        static GuardLogger gl(logger);
        return logger;
    }

    inline int LoadAndStartDefaultLogger(const std::string& path)
    {
        InitLogger(GetDefaultLogger());
        int ret = InitFromYMALFile(path, GetDefaultLogger());
        if (ret != 0)
        {
            printf("init and load default logger error. ret:<%d>.", ret);
            return ret;
        }
        ret = AutoStartLogger(GetDefaultLogger());
        if (ret != 0)
        {
            printf("auto start default logger error. ret:<%d>.", ret);
            return ret;
        }
        return 0;
    }

    inline int FastStartDefaultLogger(const std::string& config_text)
    {
        InitLogger(GetDefaultLogger());
        int ret = InitFromYMAL(config_text, "", GetDefaultLogger());
        if (ret != 0)
        {
            printf("init default logger error. ret:<%d>.", ret);
            return ret;
        }
        ret = AutoStartLogger(GetDefaultLogger());
        if (ret != 0)
        {
            printf("auto start default logger error. ret:<%d>.", ret);
            return ret;
        }
        return 0;
    }

    inline int FastStartDefaultLogger()
    {
        static const std::string default_config_text =
R"----(
 # default is mult-thread - async write channel.  
 # the first device is write rollback file  
 # the second device is print to screen.  
 - channel: 0
    sync: null
    -device: 0
        disable: false
        out_type: file
        file: "$PNAME_$YEAR$MON$DAY"
        rollback: 4
        limit_size: 100 m #only support M byte
    -device:1
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
 # default is mult-thread - async write channel.  
 # the first device is write rollback file  
 # the second device is print to screen.  
 - channel: 0
    sync: null
    -device: 0
        disable: false
        out_type: file
        file: "$PNAME"
        rollback: 1
        limit_size: 100 m #only support M byte
    -device:1
        disable: false
        out_type: screen
        priority: info
)----";
        return FastStartDefaultLogger(default_config_text);
    }

    class LogStream
    {
    public:
        static const int MAX_CONTAINER_DEPTH = 5;
    public:
        explicit LogStream(LogStream&& ls) noexcept
        {
            logger_ = ls.logger_;
            log_data_ = ls.log_data_;
            ls.logger_ = nullptr;
            ls.log_data_ = nullptr;
        }

        explicit LogStream(Logger& logger, int channel_id, int priority, int category, 
            const char * const file_name, int file_name_len, int line,
            const char * const func_name, int func_name_len, unsigned int prefix)
        {
            logger_ = nullptr;
            log_data_ = nullptr;
            if (CanPushLog(logger, channel_id, priority, category) != 0)
            {
                return;
            }
            logger_ = &logger;
            log_data_ = AllocLogData(logger, channel_id, priority, category, prefix);
            if (prefix == LOG_PREFIX_NULL)
            {
                return;
            }
            if (prefix & LOG_PREFIX_FILE)
            {
                write_char_unsafe(' ');
                if (file_name && file_name_len > 0)
                {
                    int jump_bytes = short_path(file_name, file_name_len);
                    write_buffer_unsafe(file_name + jump_bytes, file_name_len - jump_bytes);
                }
                else
                {
                    write_buffer_unsafe("nofile", 6);
                }
                write_char_unsafe(':');
                write_char_unsafe('<');
                *this << (unsigned long long)line;
                write_char_unsafe('>');
                write_char_unsafe(' ');
            }
            if (prefix & LOG_PREFIX_FUNCTION)
            {
                if (func_name && func_name_len > 0)
                {
                    write_buffer_unsafe(func_name, func_name_len);
                }
                else
                {
                    write_buffer_unsafe("null", 4);
                }
                write_char_unsafe(' ');
            }
        }
        
        ~LogStream()
        {
            if (log_data_) 
            {
                PushLog(*logger_, log_data_);
                log_data_ = nullptr;
            }
        }
        
        LogStream& set_category(int category) { if (log_data_) log_data_->category_ = category;  return *this; }
        LogStream& write_char_unsafe(char ch)
        {
            log_data_->content_[log_data_->content_len_] = ch;
            log_data_->content_len_++;
            return *this;
        }
        LogStream& write_buffer_unsafe(const char* src, int src_len)
        {
            memcpy(log_data_->content_ + log_data_->content_len_, src, src_len);
            log_data_->content_len_ += src_len;
            return *this;
        }

        LogStream& write_buffer(const char* src, int src_len)
        {
            if (log_data_ && src && src_len > 0 && log_data_->content_len_ < LogData::MAX_LOG_SIZE)
            {
                src_len = FN_MIN(src_len, LogData::MAX_LOG_SIZE - log_data_->content_len_);
                memcpy(log_data_->content_ + log_data_->content_len_, src, src_len);
                log_data_->content_len_ += src_len;
            }
            return *this;
        }

        LogStream& write_pointer(const void* ptr)
        {
            if (log_data_ && log_data_->content_len_ + 30 <= LogData::MAX_LOG_SIZE)
            {
                log_data_->content_len_ += FNLog::write_pointer_unsafe(log_data_->content_ + log_data_->content_len_, ptr);
            }
            return *this;
        }

        LogStream& write_binary(const char* dst, int len)
        {
            if (!log_data_)
            {
                return *this;
            }
            write_buffer("\r\n\t[", sizeof("\r\n\t[")-1);
            for (int i = 0; i < (len / 32) + 1; i++)
            {
                write_buffer("\r\n\t[", sizeof("\r\n\t[") - 1);
                *this << (void*)(dst + (size_t)i * 32);
                write_buffer(": ", sizeof(": ") - 1);
                for (int j = i * 32; j < (i + 1) * 32 && j < len; j++)
                {
                    if (isprint((unsigned char)dst[j]))
                    {
                        write_buffer(" ", sizeof(" ") - 1);
                        write_buffer(dst + j, 1);
                        write_buffer(" ", sizeof(" ") - 1);
                    }
                    else
                    {
                        write_buffer(" . ", sizeof(" . ") - 1);
                    }
                }
                write_buffer("\r\n\t[", sizeof("\r\n\t[") - 1);
                if (log_data_->content_len_ + sizeof(void*) <= LogData::MAX_LOG_SIZE)
                {
                    write_pointer(dst + (size_t)i * 32);
                }
                write_buffer(": ", sizeof(": ") - 1);
                for (int j = i * 32; j < (i + 1) * 32 && j < len; j++)
                {
                    if (log_data_->content_len_ + 30 >= LogData::MAX_LOG_SIZE)
                    {
                        break;
                    }
                    log_data_->content_len_ += FNLog::write_hex_unsafe<2>(log_data_->content_ + log_data_->content_len_,
                        (unsigned long long)(unsigned char)dst[j]);
                    write_buffer(" ", sizeof(" ") - 1);
                }
            }
            write_buffer("\r\n\t]\r\n\t", sizeof("\r\n\t]\r\n\t") - 1);
            return *this;
        }

        LogStream& operator <<(const char* cstr)
        {
            if (log_data_)
            {
                if (cstr)
                {
                    write_buffer(cstr, (int)strlen(cstr));
                }
                else
                {
                    write_buffer("null", 4);
                }
            }
            return *this;
        }
        LogStream& operator <<(const void* ptr)
        {
            write_pointer(ptr);
            return *this;
        }
        
        LogStream& operator <<(std::nullptr_t) 
        {
            return write_pointer(nullptr);
            return *this;
        }

        LogStream& operator << (char ch) { return write_buffer(&ch, 1);}
        LogStream & operator << (unsigned char ch) { return (*this << (unsigned long long)ch); }

        LogStream& operator << (bool val) { return (val ? write_buffer("true", 4) : write_buffer("false", 5)); }

        LogStream & operator << (short val) { return (*this << (long long)val); }
        LogStream & operator << (unsigned short val) { return (*this << (unsigned long long)val); }
        LogStream & operator << (int val) { return (*this << (long long)val); }
        LogStream & operator << (unsigned int val) { return (*this << (unsigned long long)val); }
        LogStream & operator << (long val) { return (*this << (long long)val); }
        LogStream & operator << (unsigned long val) { return (*this << (unsigned long long)val); }
        
        LogStream& operator << (long long integer)
        {
            if (log_data_ && log_data_->content_len_ + 30 <= LogData::MAX_LOG_SIZE)
            {
                log_data_->content_len_ += write_dec_unsafe<0>(log_data_->content_ + log_data_->content_len_, (long long)integer);
            }
            return *this;
        }

        LogStream& operator << (unsigned long long integer)
        {
            if (log_data_ && log_data_->content_len_ + 30 <= LogData::MAX_LOG_SIZE)
            {
                log_data_->content_len_ += write_dec_unsafe<0>(log_data_->content_ + log_data_->content_len_, (unsigned long long)integer);
            }
            return *this;
        }

        LogStream& operator << (float f)
        {
            if (log_data_ && log_data_->content_len_ + 30 <= LogData::MAX_LOG_SIZE)
            {
                log_data_->content_len_ += write_float_unsafe(log_data_->content_ + log_data_->content_len_, f);
            }
            return *this;
        }
        LogStream& operator << (double df)
        {
            if (log_data_ && log_data_->content_len_ + 30 <= LogData::MAX_LOG_SIZE)
            {
                log_data_->content_len_ += write_double_unsafe(log_data_->content_ + log_data_->content_len_, df);
            }
            return *this;
        }


        template<class _Ty1, class _Ty2>
        inline LogStream & operator <<(const std::pair<_Ty1, _Ty2> & val){ return *this << '<' <<val.first << ':' << val.second << '>'; }

        template<class Container>
        LogStream& write_container(const Container& container, const char* name, int len)
        {
            write_buffer(name, len);
            write_buffer("(", 1);
            *this << container.size();
            write_buffer(")[", 2);
            int inputCount = 0;
            for (auto iter = container.begin(); iter != container.end(); iter++)
            {
                if (inputCount >= MAX_CONTAINER_DEPTH)
                {
                    *this << "..., ";
                    break;
                }
                if(inputCount > 0)
                {
                    *this << ", ";
                }
                *this << *iter;
                inputCount++;
            }
            return *this << "]";
        }

        template<class _Elem, class _Alloc>
        LogStream & operator <<(const std::vector<_Elem, _Alloc> & val) { return write_container(val, "vector:", sizeof("vector:") - 1);}
        template<class _Elem, class _Alloc>
        LogStream & operator <<(const std::list<_Elem, _Alloc> & val) { return write_container(val, "list:", sizeof("list:") - 1);}
        template<class _Elem, class _Alloc>
        LogStream & operator <<(const std::deque<_Elem, _Alloc> & val) { return write_container(val, "deque:", sizeof("deque:") - 1);}
        template<class _Key, class _Tp, class _Compare, class _Allocator>
        LogStream & operator <<(const std::map<_Key, _Tp, _Compare, _Allocator> & val) { return write_container(val, "map:", sizeof("map:") - 1);}
        template<class _Key, class _Compare, class _Allocator>
        LogStream & operator <<(const std::set<_Key, _Compare, _Allocator> & val) { return write_container(val, "set:", sizeof("set:") - 1);}
        template<class _Key, class _Tp, class _Hash, class _Compare, class _Allocator>
        LogStream& operator <<(const std::unordered_map<_Key, _Tp, _Hash, _Compare, _Allocator>& val)
        {return write_container(val, "unordered_map:", sizeof("unordered_map:") - 1);}
        template<class _Key, class _Hash, class _Compare, class _Allocator>
        LogStream& operator <<(const std::unordered_set<_Key, _Hash, _Compare, _Allocator> & val)
        {return write_container(val, "unordered_set:", sizeof("unordered_set:") - 1);}
        template<class _Traits, class _Allocator>
        LogStream & operator <<(const std::basic_string<char, _Traits, _Allocator> & str) { return write_buffer(str.c_str(), (int)str.length());}
        
    public:
        LogData * log_data_ = nullptr;
        Logger* logger_ = nullptr;
    };
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
inline FNLog::LogStream& TLOG_TEMPLATE(FNLog::LogStream& ls)
{
    return ls;
}
template <typename ... Args>
FNLog::LogStream& LogTemplatePack(FNLog::LogStream& ls, Args&& ... args)
{
    char buff[] = { (ls << args, 0) ... };
    return ls;
}

#define LogTracePack(channel_id, category, ...)  LogTemplatePack(LOG_STREAM_DEFAULT_LOGGER(channel_id, FNLog::PRIORITY_TRACE, category, FNLog::LOG_PREFIX_ALL, ##__VA_ARGS__)
#define LogDebugPack(channel_id, category, ...)  LogTemplatePack(LOG_STREAM_DEFAULT_LOGGER(channel_id, FNLog::PRIORITY_DEBUG, category, FNLog::LOG_PREFIX_ALL, ##__VA_ARGS__)
#define LogInfoPack(channel_id,  category, ...)  LogTemplatePack(LOG_STREAM_DEFAULT_LOGGER(channel_id, FNLog::PRIORITY_INFO,  category, FNLog::LOG_PREFIX_ALL, ##__VA_ARGS__)
#define LogWarnPack(channel_id,  category, ...)  LogTemplatePack(LOG_STREAM_DEFAULT_LOGGER(channel_id, FNLog::PRIORITY_WARN,  category, FNLog::LOG_PREFIX_ALL, ##__VA_ARGS__)
#define LogErrorPack(channel_id, category, ...)  LogTemplatePack(LOG_STREAM_DEFAULT_LOGGER(channel_id, FNLog::PRIORITY_ERROR, category, FNLog::LOG_PREFIX_ALL, ##__VA_ARGS__)
#define LogAlarmPack(channel_id, category, ...)  LogTemplatePack(LOG_STREAM_DEFAULT_LOGGER(channel_id, FNLog::PRIORITY_ALARM, category, FNLog::LOG_PREFIX_ALL, ##__VA_ARGS__)
#define LogFatalPack(channel_id, category, ...)  LogTemplatePack(LOG_STREAM_DEFAULT_LOGGER(channel_id, FNLog::PRIORITY_FATAL, category, FNLog::LOG_PREFIX_ALL, ##__VA_ARGS__)



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
#ifdef WIN32
#define LOG_FORMAT(channel_id, priority, category, prefix, logformat, ...) \
do{ \
    FNLog::LogStream __log_stream(LOG_STREAM_DEFAULT_LOGGER(channel_id, priority, category, prefix));\
    if (__log_stream.log_data_)\
    {\
        int __log_len = _snprintf_s(__log_stream.log_data_ ->content_ + __log_stream.log_data_ ->content_len_, FNLog::LogData::MAX_LOG_SIZE - __log_stream.log_data_ ->content_len_, _TRUNCATE, logformat, ##__VA_ARGS__); \
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
        int __log_len = snprintf(__log_stream.log_data_ ->content_ + __log_stream.log_data_ ->content_len_, FNLog::LogData::MAX_LOG_SIZE - __log_stream.log_data_ ->content_len_, logformat, ##__VA_ARGS__); \
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
