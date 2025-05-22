
/*
* Copyright (C) 2019 YaweiZhang <yawei.zhang@foxmail.com>.
* All rights reserved
* This file is part of the fn-log, used MIT License.
*/


#pragma once
#ifndef _FN_LOG_STREAM_H_
#define _FN_LOG_STREAM_H_

#include "fn_data.h"
#include "fn_parse.h"
#include "fn_load.h"
#include "fn_core.h"

namespace FNLog
{
    template<int BLANK_SIZE>
    struct LogBlankAlign
    {
        //static const int blank_size = BLANK_SIZE;
    };


    struct LogPercent
    {
        LogPercent(float v) :v_(v) {}
        LogPercent(double v) :v_(static_cast<float>(v)) {}
        float v_;
    };

    struct LogBinText
    {
        LogBinText(const void* bin, int len) { text_bin = bin; bin_len = len; }
        template<class T>
        LogBinText(const T& t) { text_bin = &t; bin_len = sizeof(T); }
        const void* text_bin;
        int bin_len;
    };

    struct LogHexText
    {
        LogHexText(const void* bin, int len) { text_bin = bin; bin_len = len; }
        template<class T>
        LogHexText(const T& t) { text_bin = &t; bin_len = sizeof(T); }
        const void* text_bin;
        int bin_len;
    };


    struct LogTimestamp
    {
        LogTimestamp()
        {
#ifdef WIN32
            FILETIME ft;
            GetSystemTimeAsFileTime(&ft);
            unsigned long long now = ft.dwHighDateTime;
            now <<= 32;
            now |= ft.dwLowDateTime;
            now /= 10;
            now -= 11644473600000000ULL;
            now /= 1000;
            ts_ = now / 1000;
            precise_ = (unsigned int)(now % 1000);
#else
            struct timeval tm;
            gettimeofday(&tm, nullptr);
            ts_ = tm.tv_sec;
            precise_ = tm.tv_usec / 1000;
#endif
        }
        LogTimestamp(long long ts, int precise)
        {
            ts_ = ts;
            precise_ = precise;
        }
        LogTimestamp(long long ts_ms)
        {
            ts_ = ts_ms/1000;
            precise_ = ts_ms%1000;
        }
        long long ts_;
        int precise_;
    };

    class LogStream
    {
    public:
        static const int MAX_CONTAINER_DEPTH = 5;
    public:
        LogStream(const LogStream& other) = delete;
        LogStream(LogStream&& other) noexcept
        {
            logger_ = other.logger_;
            log_data_ = other.log_data_;
            hold_idx_ = other.hold_idx_;
            tick_ = other.tick_;
            other.logger_ = nullptr;
            other.log_data_ = nullptr;
            other.hold_idx_ = -1;
            other.tick_ = 0;
        }
        long long get_tick()
        {
#ifdef WIN32
            //_mm_lfence();
            return (long long)__rdtsc();
#elif defined(__GCC_ASM_FLAG_OUTPUTS__) && defined(__x86_64__)
            unsigned int lo = 0;
            unsigned int hi = 0;
 //           __asm__ __volatile__("lfence;rdtsc" : "=a" (lo), "=d" (hi) ::);
            __asm__ __volatile__("rdtsc" : "=a" (lo), "=d" (hi) ::);
            unsigned long long val = ((unsigned long long)hi << 32) | lo;
            return (long long)val;
#else
            return 0;
#endif
        }

        explicit LogStream(Logger& logger, int channel_id, int priority, int category, long long identify,
            const char * const file_name, int file_name_len, int line,
            const char * const func_name, int func_name_len, unsigned int prefix)
        {
            if (BlockInput(logger, channel_id, priority, category, identify))
            {
                return;
            }

#ifdef FN_LOG_CPU_COST_STAT
            tick_ = get_tick();
#endif 

            int hold_idx = HoldChannel(logger, channel_id, priority, category, identify);
            if (hold_idx < 0)
            {
                return;
            }

            try
            {
                InitLogData(logger, logger.shm_->ring_buffers_[channel_id].buffer_[hold_idx], channel_id, priority, category, identify, prefix);
            }
            catch (const std::exception&)
            {
                printf("%s", "alloc log error. no more memory.");
                return;
            }
            logger_ = &logger;
            log_data_ = &logger.shm_->ring_buffers_[channel_id].buffer_[hold_idx];
            hold_idx_ = hold_idx;
            log_data_->code_line_ = line;
            log_data_->code_func_ = func_name;
            log_data_->code_func_len_ = func_name_len;
            log_data_->code_file_ = file_name;
            log_data_->code_file_len_ = file_name_len;
            if (prefix == LOG_PREFIX_NULL)
            {
                return;
            }
            if (prefix & LOG_PREFIX_TIMESTAMP)
            {
                log_data_->content_len_ += write_date_unsafe(log_data_->content_ + log_data_->content_len_, log_data_->timestamp_, log_data_->precise_);
            }
            if (prefix & LOG_PREFIX_PRIORITY)
            {
                log_data_->content_len_ += write_log_priority_unsafe(log_data_->content_ + log_data_->content_len_, log_data_->priority_);
            }
            if (prefix & LOG_PREFIX_THREAD)
            {
                log_data_->content_len_ += write_log_thread_unsafe(log_data_->content_ + log_data_->content_len_, log_data_->thread_);
            }
            if (prefix & LOG_PREFIX_NAME)
            {
                write_char_unsafe('[');
                write_buffer_unsafe(logger.name_, logger.name_len_);
                write_char_unsafe(']');
            }
            if (prefix & LOG_PREFIX_DESC)
            {
                write_char_unsafe('[');
                write_buffer_unsafe(logger.desc_, logger.desc_len_);
                write_char_unsafe(']');
            }
            if (prefix & LOG_PREFIX_FILE)
            {
                write_char_unsafe('[');
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
                *this << (unsigned long long)line;
                write_char_unsafe(']');
            }
            if (prefix & LOG_PREFIX_FUNCTION)
            {
                write_char_unsafe('(');
                if (func_name && func_name_len > 0)
                {
                    write_buffer_unsafe(func_name, func_name_len);
                }
                else
                {
                    write_buffer_unsafe("null", 4);
                }
                write_char_unsafe(')');
            }
            write_char_unsafe(' ');
            log_data_->prefix_len_ = log_data_->content_len_;
        }
        
        ~LogStream()
        {
            if (log_data_) 
            {
                if (RefVirtualDevice() != NULL)
                {
                    Channel& channel = logger_->shm_->channels_[log_data_->channel_id_];
                    if (channel.virtual_device_id_ >= 0)
                    {
                        Device& device = channel.devices_[channel.virtual_device_id_];
                        if (log_data_->priority_ >= device.config_fields_[DEVICE_CFG_PRIORITY])
                        {
                            //more block check in the proc 
                            EnterProcOutVirtualDevice(*logger_, log_data_->channel_id_, channel.virtual_device_id_, *log_data_);
                            //(*RefVirtualDevice())(*log_data_);
                        }
                        
                    }
                }
                PushLog(*logger_, log_data_->channel_id_, hold_idx_);
                hold_idx_ = -1;

#ifdef FN_LOG_CPU_COST_STAT
                tick_ = get_tick() - tick_;
                logger_->tick_sum_ += tick_;
                logger_->tick_count_++;
#endif 
                log_data_ = nullptr;
                logger_ = nullptr;
            }
        }
        //trans LogStream (temporary values) to  LogStream& (left values) 
        //all user's LogStream operator  only care about LogStream& without temporary.   
        LogStream& self() { return *this; }
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
            if (log_data_ && src && src_len > 0 && log_data_->content_len_ < LogData::LOG_SIZE)
            {
                src_len = FN_MIN(src_len, LogData::LOG_SIZE - log_data_->content_len_);
                memcpy(log_data_->content_ + log_data_->content_len_, src, src_len);
                log_data_->content_len_ += src_len;
            }
            return *this;
        }


        template<size_t Wide>
        LogStream& write_longlong(long long n)
        {
            if (log_data_ && log_data_->content_len_ + 30 <= LogData::LOG_SIZE)
            {
                log_data_->content_len_ += write_dec_unsafe<Wide>(log_data_->content_ + log_data_->content_len_, n);
            }
            return *this;
        }
        template<size_t Wide>
        LogStream& write_ulonglong(unsigned long long n)
        {
            if (log_data_ && log_data_->content_len_ + 30 <= LogData::LOG_SIZE)
            {
                log_data_->content_len_ += write_dec_unsafe<Wide>(log_data_->content_ + log_data_->content_len_, n);
            }
            return *this;
        }

        template<size_t Wide, class N>
        LogStream& write_number(N n)
        {
            if (std::is_signed<N>::value)
            {
                return write_longlong<Wide>((long long)n);
            }
            return write_ulonglong<Wide>((unsigned long long)n);
        }

        LogStream& write_pointer(const void* ptr)
        {
            if (log_data_ && log_data_->content_len_ + 30 <= LogData::LOG_SIZE)
            {
                log_data_->content_len_ += FNLog::write_pointer_unsafe(log_data_->content_ + log_data_->content_len_, ptr);
            }
            return *this;
        }

        LogStream& write_bin_text(const char* dst, int len)
        {
            if (!log_data_)
            {
                return *this;
            }
            write_buffer("\r\n\t[", sizeof("\r\n\t[") - 1);
            for (int i = 0; i < (len / 16) + 1; i++)
            {
                if (i*16 >= len)
                {
                    continue;
                }
                write_buffer("\r\n\t[", sizeof("\r\n\t[") - 1);
                *this << (void*)(dst + (size_t)i * 16);
                write_buffer(": ", sizeof(": ") - 1);
                for (int j = i * 16; j < (i + 1) * 16 && j < len; j++)
                {
                    if (log_data_->content_len_ + 30 >= LogData::LOG_SIZE)
                    {
                        break;
                    }
                    log_data_->content_len_ += FNLog::write_bin_unsafe<8>(log_data_->content_ + log_data_->content_len_,
                        (unsigned long long)(unsigned char)dst[j]);
                    write_buffer(" ", sizeof(" ") - 1);
                }
            }
            write_buffer("\r\n\t]\r\n\t", sizeof("\r\n\t]\r\n\t") - 1);
            return *this;
        }
        LogStream& write_hex_text(const char* dst, int len)
        {
            if (!log_data_)
            {
                return *this;
            }
            write_buffer("\r\n\t[", sizeof("\r\n\t[")-1);
            for (int i = 0; i < (len / 32) + 1; i++)
            {
                if (i * 32 >= len)
                {
                    continue;
                }
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
                if (log_data_->content_len_ + sizeof(void*) <= LogData::LOG_SIZE)
                {
                    write_pointer(dst + (size_t)i * 32);
                }
                write_buffer(": ", sizeof(": ") - 1);
                for (int j = i * 32; j < (i + 1) * 32 && j < len; j++)
                {
                    if (log_data_->content_len_ + 30 >= LogData::LOG_SIZE)
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
        
        LogStream& operator << (long long integer){ return write_longlong<0>(integer);}

        LogStream& operator << (unsigned long long integer){return write_ulonglong<0>(integer);}

        LogStream& operator << (float f)
        {
            if (log_data_ && log_data_->content_len_ + 30 <= LogData::LOG_SIZE)
            {
                log_data_->content_len_ += write_float_unsafe(log_data_->content_ + log_data_->content_len_, f);
            }
            return *this;
        }
        LogStream& operator << (double df)
        {
            if (log_data_ && log_data_->content_len_ + 30 <= LogData::LOG_SIZE)
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
            int input_count = 0;
            for (auto iter = container.begin(); iter != container.end(); iter++)
            {
                if (input_count >= MAX_CONTAINER_DEPTH)
                {
                    *this << "..., ";
                    break;
                }
                if(input_count > 0)
                {
                    *this << ", ";
                }
                *this << *iter;
                input_count++;
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
        template<int BLANK_SIZE>
        LogStream & operator <<(const LogBlankAlign<BLANK_SIZE>& blanks)
        {
            if (log_data_ && log_data_->content_len_ + BLANK_SIZE < LogData::LOG_SIZE)
            {
                for (int i = log_data_->content_len_;  i < BLANK_SIZE; i++)
                {
                    write_char_unsafe(' ');
                }
            }
            return *this;
        }
        LogStream & operator <<(const LogPercent& blanks)
        {
            if (log_data_ && log_data_->content_len_ + 40 < LogData::LOG_SIZE)
            {
                if (blanks.v_ < 0.000001)
                {
                    write_buffer("00.00%", (int)strlen("00.00%"));
                }
                else
                {
                    log_data_->content_len_ += write_double_unsafe(log_data_->content_ + log_data_->content_len_, blanks.v_*100.0);
                    write_char_unsafe('%');
                }
            }
            return *this;
        }

        LogStream & operator <<(const LogHexText& text)
        {
            return write_hex_text((const char *)text.text_bin, text.bin_len);
        }

        LogStream& operator <<(const LogBinText& text)
        {
            return write_bin_text((const char*)text.text_bin, text.bin_len);
        }

        LogStream & operator <<(const LogTimestamp& date)
        {
            if (log_data_ && log_data_->content_len_ + 40 < LogData::LOG_SIZE)
            {
                int write_bytes = write_date_unsafe(log_data_->content_ + log_data_->content_len_, date.ts_, date.precise_);
                log_data_->content_len_ += write_bytes;
            }
            return *this;
        }
        


    public:
        LogData * log_data_ = nullptr;
        Logger* logger_ = nullptr;
        int hold_idx_ = -1;//ring buffer  
        long long tick_ = 0;
    };



}


#endif
