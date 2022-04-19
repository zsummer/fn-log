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
#ifndef _FN_LOG_CHANNEL_H_
#define _FN_LOG_CHANNEL_H_

#include "fn_data.h"
#include "fn_out_file_device.h"
#include "fn_out_screen_device.h"
#include "fn_out_udp_device.h"
#include "fn_out_virtual_device.h"
#include "fn_fmt.h"

namespace FNLog
{
    inline void EnterProcDevice(Logger& logger, int channel_id, int device_id, LogData & log)
    {
        Channel& channel = logger.shm_->channels_[channel_id];
        Device& device = channel.devices_[device_id];
        //async promise only single thread proc. needn't lock.
        Logger::ReadGuard rg(logger.read_locks_[channel_id], channel.channel_type_ == CHANNEL_ASYNC);
        switch (device.out_type_)
        {
        case DEVICE_OUT_FILE:
            EnterProcOutFileDevice(logger, channel_id, device_id, log);
            break;
        case DEVICE_OUT_SCREEN:
            EnterProcOutScreenDevice(logger, channel_id, device_id, log);
            break;
        case DEVICE_OUT_UDP:
            EnterProcOutUDPDevice(logger, channel_id, device_id, log);
            break;
        case DEVICE_OUT_VIRTUAL:
            EnterProcOutVirtualDevice(logger, channel_id, device_id, log);
            break;        
        default:
            break;
        }
    }
    

    inline void DispatchLog(Logger & logger, Channel& channel, LogData& log)
    {
        for (int device_id = 0; device_id < channel.device_size_; device_id++)
        {
            Device& device = channel.devices_[device_id];
            if (!AtomicLoadC(device, DEVICE_CFG_ABLE))
            {
                continue;
            }
            if (log.priority_ < AtomicLoadC(device, DEVICE_CFG_PRIORITY))
            {
                continue;
            }
            long long begin_category = AtomicLoadC(device, DEVICE_CFG_CATEGORY);
            long long category_count = AtomicLoadC(device, DEVICE_CFG_CATEGORY_EXTEND);
            unsigned long long category_filter = (unsigned long long)AtomicLoadC(device, DEVICE_CFG_CATEGORY_FILTER);
            long long begin_identify = AtomicLoadC(device, DEVICE_CFG_IDENTIFY);
            long long identify_count =AtomicLoadC(device, DEVICE_CFG_IDENTIFY_EXTEND);
            unsigned long long identify_filter = (unsigned long long)AtomicLoadC(device, DEVICE_CFG_IDENTIFY_FILTER);

            if (category_count > 0 && (log.category_ < begin_category || log.category_ >= begin_category + category_count))
            {
                continue;
            }
            if (identify_count > 0 && (log.identify_ < begin_identify || log.identify_ >= begin_identify + identify_count))
            {
                continue;
            }
            if (category_filter && (category_filter & ((1ULL) << (unsigned int)log.category_)) == 0)
            {
                continue;
            }
            if (identify_filter && (identify_filter & ((1ULL) << (unsigned int)log.identify_)) == 0)
            {
                continue;
            }
            EnterProcDevice(logger, channel.channel_id_, device_id, log);
        }
    }
    
 
    inline void EnterProcChannel(Logger& logger, int channel_id)
    {
        Channel& channel = logger.shm_->channels_[channel_id];
        RingBuffer& ring_buffer = logger.shm_->ring_buffers_[channel_id];
        do
        {
            int local_write_count = 0;
            do
            {
                int old_idx = ring_buffer.proc_idx_.load(std::memory_order_acquire);
                int next_idx = (old_idx + 1) % RingBuffer::BUFFER_LEN;
                if (old_idx == ring_buffer.write_idx_.load(std::memory_order_acquire))
                {
                    //empty branch    
                    break;
                }

                //set proc index  
                if (!ring_buffer.proc_idx_.compare_exchange_strong(old_idx, next_idx))
                {
                    //only one thread get log. this branch will not hit.   
                    break;
                }
                auto& cur_log = ring_buffer.buffer_[old_idx];
                DispatchLog(logger, channel, cur_log);
                cur_log.data_mark_ = 0;
                AtomicAddL(channel, CHANNEL_LOG_PROCESSED);
                local_write_count ++;


                do
                {
                    //set read index to proc index  
                    old_idx = ring_buffer.read_idx_.load(std::memory_order_acquire);
                    next_idx = (old_idx + 1) % RingBuffer::BUFFER_LEN;
                    if (old_idx == ring_buffer.proc_idx_.load(std::memory_order_acquire))
                    {
                        break;
                    }
                    if (ring_buffer.buffer_[old_idx].data_mark_.load(std::memory_order_acquire) != MARK_INVALID)
                    {
                        break;
                    }
                    ring_buffer.read_idx_.compare_exchange_strong(old_idx, next_idx);
                } while (true);  

                //if want the high log security can reduce this threshold or enable shared memory queue.  
                if (local_write_count > 10000)
                {
                    local_write_count = 0;
                    for (int i = 0; i < channel.device_size_; i++)
                    {
                        if (channel.devices_[i].out_type_ == DEVICE_OUT_FILE)
                        {
                            logger.file_handles_[channel_id * Channel::MAX_DEVICE_SIZE + i].flush();
                        }
                    }
                }
            } while (true);  


            if (channel.channel_state_ == CHANNEL_STATE_NULL)
            {
                channel.channel_state_ = CHANNEL_STATE_RUNNING;
            }

            if (local_write_count)
            {
                for (int i = 0; i < channel.device_size_; i++)
                {
                    if (channel.devices_[i].out_type_ == DEVICE_OUT_FILE)
                    {
                        logger.file_handles_[channel_id * Channel::MAX_DEVICE_SIZE + i].flush();
                    }
                }
            }
            HotUpdateLogger(logger, channel.channel_id_);
            if (channel.channel_type_ == CHANNEL_ASYNC)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            
        } while (channel.channel_type_ == CHANNEL_ASYNC 
            && (channel.channel_state_ == CHANNEL_STATE_RUNNING || ring_buffer.write_idx_ != ring_buffer.read_idx_));

        if (channel.channel_type_ == CHANNEL_ASYNC)
        {
            channel.channel_state_ = CHANNEL_STATE_FINISH;
        }
    }
    
    

    inline void InitLogData(Logger& logger, LogData& log, int channel_id, int priority, int category, unsigned long long identify, unsigned int prefix)
    {
        log.channel_id_ = channel_id;
        log.priority_ = priority;
        log.category_ = category;
        log.identify_ = identify;
        log.code_line_ = 0;
        log.code_func_len_ = 0;
        log.code_file_len_ = 0;
        log.code_file_ = "";
        log.code_func_ = "";
        log.prefix_len_ = 0;
        log.content_len_ = 0;
        log.content_[log.content_len_] = '\0';

#ifdef WIN32
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

#ifdef WIN32
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
        log.content_[log.content_len_] = '\0';
        log.prefix_len_ = log.content_len_;
        return;
    }
#ifdef __GNUG__
#pragma GCC push_options
#pragma GCC optimize ("O2")
#endif
    inline bool BlockInput(Logger& logger, int channel_id, int priority, int category, long long identify)
    {
        if (logger.shm_ == NULL || channel_id >= logger.shm_->channel_size_ || channel_id < 0)
        {
            return true;
        }
        if (logger.logger_state_ != LOGGER_STATE_RUNNING)
        {
            return true;
        }
        Channel& channel = logger.shm_->channels_[channel_id];
        if (channel.channel_state_ != CHANNEL_STATE_RUNNING)
        {
            return true;
        }
        if (priority < AtomicLoadC(channel, CHANNEL_CFG_PRIORITY))
        {
            return true;
        }
        long long begin_category = AtomicLoadC(channel, CHANNEL_CFG_CATEGORY);
        long long category_count = AtomicLoadC(channel, CHANNEL_CFG_CATEGORY_EXTEND);
        unsigned long long category_filter = (unsigned long long)AtomicLoadC(channel, CHANNEL_CFG_CATEGORY_FILTER);
        long long begin_identify = AtomicLoadC(channel, CHANNEL_CFG_IDENTIFY);
        long long identify_count = AtomicLoadC(channel, CHANNEL_CFG_IDENTIFY_EXTEND);
        unsigned long long identify_filter = (unsigned long long)AtomicLoadC(channel, CHANNEL_CFG_IDENTIFY_FILTER);

        if (category_count > 0 && (category < begin_category || category >= begin_category + category_count))
        {
            return true;
        }
        if (identify_count > 0 && (identify < begin_identify || identify >= begin_identify + identify_count))
        {
            return true;
        }
        if (category_filter && (category_filter & ((1ULL) << (unsigned int)category)) == 0)
        {
            return true;
        }
        if (identify_filter && (identify_filter & ((1ULL) << (unsigned int)identify)) == 0)
        {
            return true;
        }

        bool need_write = false;
        
        for (int i = 0; i < channel.device_size_; i++)
        {
            Device::ConfigFields& fields = channel.devices_[i].config_fields_;
            long long field_able = fields[FNLog::DEVICE_CFG_ABLE];
            long long field_priority = fields[FNLog::DEVICE_CFG_PRIORITY];
            long long field_begin_category = fields[FNLog::DEVICE_CFG_CATEGORY];
            long long field_category_count = fields[FNLog::DEVICE_CFG_CATEGORY_EXTEND];
            unsigned long long field_category_filter = (unsigned long long)fields[FNLog::DEVICE_CFG_CATEGORY_FILTER];
            long long field_begin_identify = fields[FNLog::DEVICE_CFG_IDENTIFY];
            long long field_identify_count = fields[FNLog::DEVICE_CFG_IDENTIFY_EXTEND];
            unsigned long long field_identify_filter = (unsigned long long)fields[FNLog::DEVICE_CFG_IDENTIFY_FILTER];

            if (field_able && priority >= field_priority)
            {
                if (field_category_count > 0 && (category < field_begin_category || category >= field_begin_category + field_category_count))
                {
                    continue;
                }
                if (field_identify_count > 0 && (identify < field_begin_identify || identify >= field_begin_identify + field_identify_count))
                {
                    continue;
                }
                if (field_category_filter &&  (field_category_filter & ((1ULL) << (unsigned int)category)) == 0)
                {
                    continue;
                }
                if (field_identify_filter &&  (field_identify_filter & ((1ULL) << (unsigned int)identify)) == 0)
                {
                    continue;
                }
                need_write = true;
                break;
            }
        }
        if (!need_write)
        {
            return true;
        }
        
        return false;
    }
#ifdef __GNUG__
#pragma GCC pop_options
#endif
    inline int HoldChannel(Logger& logger, int channel_id, int priority, int category, long long identify)
    {
        if (BlockInput(logger, channel_id, priority, category, identify))
        {
            return -1;
        }
        Channel& channel = logger.shm_->channels_[channel_id];
        RingBuffer& ring_buffer = logger.shm_->ring_buffers_[channel_id];
        
        int state = 0;
        do
        {
            if (state > 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            state++;

            for (int i = 0; i < FN_MAX(RingBuffer::BUFFER_LEN, 10); i++)
            {
                if (channel.channel_state_ != CHANNEL_STATE_RUNNING)
                {
                    break;
                }
                int old_idx = ring_buffer.hold_idx_.load(std::memory_order_acquire);
                int hold_idx = (old_idx + 1) % RingBuffer::BUFFER_LEN;
                if (hold_idx == ring_buffer.read_idx_.load(std::memory_order_acquire))
                {
                    break;
                }
                if (ring_buffer.hold_idx_.compare_exchange_strong(old_idx, hold_idx))
                {
                    AtomicAddL(channel, CHANNEL_LOG_HOLD);
                    ring_buffer.buffer_[old_idx].data_mark_.store(MARK_HOLD, std::memory_order_release);
                    return old_idx;
                }
                continue;
            }
            if (channel.channel_state_ != CHANNEL_STATE_RUNNING)
            {
                break;
            }
        } while (true);
        return -11;
    }

    inline int PushChannel(Logger& logger, int channel_id, int hold_idx)
    {
        if (channel_id >= logger.shm_->channel_size_ || channel_id < 0)
        {
            return -1;
        }
        if (hold_idx >= RingBuffer::BUFFER_LEN || hold_idx < 0)
        {
            return -2;
        }
        Channel& channel = logger.shm_->channels_[channel_id];
        RingBuffer& ring_buffer = logger.shm_->ring_buffers_[channel_id];
        if (channel.channel_state_ != CHANNEL_STATE_RUNNING)
        {
            return -1;
        }

        LogData& log = ring_buffer.buffer_[hold_idx];
        log.content_len_ = FN_MIN(log.content_len_, LogData::LOG_SIZE - 2);
        log.content_[log.content_len_++] = '\n';
        log.content_[log.content_len_] = '\0';

        log.data_mark_ = 2;


        do
        {
            int old_idx = ring_buffer.write_idx_.load(std::memory_order_acquire);
            int next_idx = (old_idx + 1) % RingBuffer::BUFFER_LEN;
            if (old_idx == ring_buffer.hold_idx_.load(std::memory_order_acquire))
            {
                break;
            }
            if (ring_buffer.buffer_[old_idx].data_mark_.load(std::memory_order_acquire) != 2)
            {
                break;
            }
            if (ring_buffer.write_idx_.compare_exchange_strong(old_idx, next_idx))
            {
                AtomicAddL(channel, CHANNEL_LOG_PUSH);
            }
        } while (channel.channel_state_ == CHANNEL_STATE_RUNNING);

        if (channel.channel_type_ == CHANNEL_SYNC && channel.channel_state_ == CHANNEL_STATE_RUNNING)
        {
            EnterProcChannel(logger, channel_id); //no affect channel.single_thread_write_
        }
        return 0;
    }


    inline int TransmitChannel(Logger& logger, int channel_id, const LogData& log)
    {
        if (log.channel_id_ == channel_id)
        {
            return -1;
        }
        int hold_idx = FNLog::HoldChannel(logger, channel_id, log.priority_, log.category_, log.identify_);
        if (hold_idx < 0)
        {
            return -2;
        }
        LogData& trans_log = logger.shm_->ring_buffers_[channel_id].buffer_[hold_idx];
        trans_log.channel_id_ = log.channel_id_;
        trans_log.priority_ = log.priority_;
        trans_log.category_ = log.category_;
        trans_log.identify_ = log.identify_;
        trans_log.code_line_ = log.code_line_;
        trans_log.code_func_len_ = log.code_func_len_;
        trans_log.code_file_len_ = log.code_file_len_;
        trans_log.code_func_ = log.code_func_;
        trans_log.code_file_ = log.code_file_;
        trans_log.timestamp_ = log.timestamp_;
        trans_log.precise_ = log.precise_;
        trans_log.thread_ = log.thread_;
        trans_log.prefix_len_ = log.prefix_len_;
        trans_log.content_len_ = log.content_len_;
        memcpy(trans_log.content_, log.content_, log.content_len_);
        return PushChannel(logger, channel_id, hold_idx);
    }
}


#endif
