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
#ifndef _FN_LOG_MEM_H_
#define _FN_LOG_MEM_H_

#include "fn_data.h"
#include "fn_parse.h"
#include "fn_load.h"
#include "fn_core.h"

namespace FNLog
{
    inline LogData* AllocLogDataImpl(Logger& logger, int channel_id)
    {
        LogData* plog = nullptr;
        if (channel_id >= logger.channel_size_ || channel_id < 0)
        {
            return plog;
        }

        Channel& channel = logger.channels_[channel_id];
        channel.log_fields_[CHANNEL_LOG_ALLOC_CALL].num_++; //warn: not atom op, will have count loss in multi-thread mod.

        switch (channel.channel_type_)
        {
            case CHANNEL_MULTI:
            if (channel.log_pool_.log_count_ > 0)
            {
                std::lock_guard<std::mutex> l(logger.syncs_[channel_id].pool_lock_);
                if (channel.log_pool_.log_count_ > 0)
                {
                    channel.log_fields_[CHANNEL_LOG_ALLOC_CACHE].num_++;
                    channel.log_pool_.log_count_--;
                    plog = channel.log_pool_.log_queue_[channel.log_pool_.log_count_];
                    channel.log_pool_.log_queue_[channel.log_pool_.log_count_] = nullptr;
                    break;
                }
            }
            break;
            case CHANNEL_SYNC:
                if (channel.log_pool_.log_count_ > 0)
                {
                    channel.log_fields_[CHANNEL_LOG_ALLOC_CACHE].num_++;
                    channel.log_pool_.log_count_--;
                    plog = channel.log_pool_.log_queue_[channel.log_pool_.log_count_];
                    channel.log_pool_.log_queue_[channel.log_pool_.log_count_] = nullptr;
                    break;
                }
                break;
            case CHANNEL_RING:
                if (channel.log_pool_.write_count_ != channel.log_pool_.read_count_)
                {
                    plog = channel.log_pool_.log_queue_[channel.log_pool_.read_count_];
                    channel.log_pool_.log_queue_[channel.log_pool_.read_count_] = nullptr;
                    channel.log_pool_.read_count_ = (channel.log_pool_.read_count_ + 1) % LogQueue::MAX_LOG_QUEUE_CACHE_SIZE;
                    channel.log_fields_[CHANNEL_LOG_ALLOC_CACHE].num_++;
                    break;
                }
                break;
            default:
                return plog;
        }

        if (plog == nullptr)
        {
            if (logger.sys_alloc_)
            {
                plog = logger.sys_alloc_();
            }
            else
            {
                plog = new LogData;
            }
            channel.log_fields_[CHANNEL_LOG_ALLOC_REAL].num_++;
        }

        return plog;
    }

    inline LogData* AllocLogData(Logger& logger, int channel_id, int priority, int category, unsigned int prefix)
    {
        LogData* plog = AllocLogDataImpl(logger, channel_id);
        LogData& log = *plog;
        log.channel_id_ = channel_id;
        log.priority_ = priority;
        log.category_ = category;
        log.content_len_ = 0;
        log.content_[log.content_len_] = '\0';

#ifdef _WIN32
        FILETIME ft;
        GetSystemTimeAsFileTime(&ft);
        unsigned long long now = ft.dwHighDateTime;
        now <<= 32;
        now |= ft.dwLowDateTime;
        now /= 10;
        now -= 11644473600000000ULL;
        now /= 1000;
        log.timestamp_ = now / 1000;
        log.precise_ = (unsigned int)(now % 1000);
#else
        struct timeval tm;
        gettimeofday(&tm, nullptr);
        log.timestamp_ = tm.tv_sec;
        log.precise_ = tm.tv_usec / 1000;
#endif
        log.thread_ = 0;
        if (prefix == LOG_PREFIX_NULL)
        {
            return plog;
        }

#ifdef _WIN32
        static thread_local unsigned int therad_id = GetCurrentThreadId();
        log.thread_ = therad_id;
#elif defined(__APPLE__)
        unsigned long long tid = 0;
        pthread_threadid_np(nullptr, &tid);
        log.thread_ = (unsigned int)tid;
#else
        static thread_local unsigned int therad_id = (unsigned int)syscall(SYS_gettid);
        log.thread_ = therad_id;
#endif
        if (prefix & LOG_PREFIX_TIMESTAMP)
        {
            log.content_len_ += write_date_unsafe(log.content_ + log.content_len_, log.timestamp_, log.precise_);
        }
        if (prefix & LOG_PREFIX_PRIORITY)
        {
            log.content_len_ += write_log_priority_unsafe(log.content_ + log.content_len_, log.priority_);
        }
        if (prefix & LOG_PREFIX_THREAD)
        {
            log.content_len_ += write_log_thread_unsafe(log.content_ + log.content_len_, log.thread_);
        }
        log.content_[log.content_len_] = '\0';
        return &log;
    }

    inline void FreeLogData(Logger& logger, int channel_id, LogData*& plog)
    {
        if (plog == nullptr)
        {
            return;
        }
        if (channel_id < 0 || channel_id >= logger.channel_size_)
        {
            printf("%s", "error");
            return;
        }

        Channel& channel = logger.channels_[channel_id];
        channel.log_fields_[CHANNEL_LOG_FREE_CALL].num_++;

        switch (channel.channel_type_)
        {
        case CHANNEL_MULTI:
            if (channel.log_pool_.log_count_ < LogQueue::MAX_LOG_QUEUE_CACHE_SIZE)
            {
                std::lock_guard<std::mutex> l(logger.syncs_[channel_id].pool_lock_);
                if (channel.log_pool_.log_count_ < LogQueue::MAX_LOG_QUEUE_CACHE_SIZE)
                {
                    channel.log_pool_.log_queue_[channel.log_pool_.log_count_++] = plog;
                    plog = nullptr;
                    channel.log_fields_[CHANNEL_LOG_FREE_CACHE].num_++;
                    return;
                }
            }
            break;
        case CHANNEL_SYNC:
            if (channel.log_pool_.log_count_ < LogQueue::MAX_LOG_QUEUE_CACHE_SIZE)
            {
                channel.log_pool_.log_queue_[channel.log_pool_.log_count_++] = plog;
                plog = nullptr;
                channel.log_fields_[CHANNEL_LOG_FREE_CACHE].num_++;
                return;
            }
            break;
        case CHANNEL_RING:
        {
            LogQueue::SizeType next_write = (channel.log_pool_.write_count_ + 1) % LogQueue::MAX_LOG_QUEUE_CACHE_SIZE;
            if (next_write != channel.log_pool_.read_count_)
            {
                channel.log_pool_.log_queue_[channel.log_pool_.write_count_] = plog;
                channel.log_pool_.write_count_ = next_write;
                plog = nullptr;
                channel.log_fields_[CHANNEL_LOG_FREE_CACHE].num_++;
                return;
            }
        }
        break;
        default:
            break;
        }


        if (logger.sys_free_)
        {
            logger.sys_free_(plog);
            plog = nullptr;
            return;
        }
        delete plog;
        plog = nullptr;
        channel.log_fields_[CHANNEL_LOG_FREE_REAL].num_++;
    }





}


#endif
