/*
* Copyright (C) 2019 YaweiZhang <yawei.zhang@foxmail.com>.
* All rights reserved
* This file is part of the fn-log, used MIT License.
*/



#pragma once
#ifndef _FN_LOG_CHANNEL_H_
#define _FN_LOG_CHANNEL_H_

#include "fn_data.h"
#include "fn_out_file_device.h"
#include "fn_out_screen_device.h"
#include "fn_out_udp_device.h"
#include "fn_out_virtual_device.h"
#include "fn_out_empty_device.h"
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
        case DEVICE_OUT_SCREEN:
            EnterProcOutScreenDevice(logger, channel_id, device_id, log);
            break;
        case DEVICE_OUT_FILE:
            EnterProcOutFileDevice(logger, channel_id, device_id, log);
            break;
        case DEVICE_OUT_UDP:
            EnterProcOutUDPDevice(logger, channel_id, device_id, log);
            break;
        case DEVICE_OUT_VIRTUAL:
            //EnterProcOutVirtualDevice(logger, channel_id, device_id, log);
            break;        
        case DEVICE_OUT_EMPTY:
            EnterProcOutEmptyDevice(logger, channel_id, device_id, log);
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
            if (device.in_type_ != DEVICE_IN_NULL)
            {
                continue;
            }
            if (log.priority_ < AtomicLoadC(device, DEVICE_CFG_PRIORITY))
            {
                continue;
            }
            long long begin_category = AtomicLoadC(device, DEVICE_CFG_CATEGORY);
            long long category_count = AtomicLoadC(device, DEVICE_CFG_CATEGORY_EXTEND);
            unsigned long long category_mask = (unsigned long long)AtomicLoadC(device, DEVICE_CFG_CATEGORY_MASK);
            long long begin_identify = AtomicLoadC(device, DEVICE_CFG_IDENTIFY);
            long long identify_count =AtomicLoadC(device, DEVICE_CFG_IDENTIFY_EXTEND);
            unsigned long long identify_mask = (unsigned long long)AtomicLoadC(device, DEVICE_CFG_IDENTIFY_MASK);

            if (category_count > 0 && (log.category_ < begin_category || log.category_ >= begin_category + category_count))
            {
                continue;
            }
            if (identify_count > 0 && (log.identify_ < begin_identify || log.identify_ >= begin_identify + identify_count))
            {
                continue;
            }
            if (category_mask && (category_mask & ((1ULL) << (unsigned int)log.category_)) == 0)
            {
                continue;
            }
            if (identify_mask && (identify_mask & ((1ULL) << (unsigned int)log.identify_)) == 0)
            {
                continue;
            }
            EnterProcDevice(logger, channel.channel_id_, device_id, log);
        }
    }

    inline void InitLogData(Logger& logger, LogData& log, int channel_id, int priority, int category, unsigned long long identify, unsigned int prefix);
 
    inline void EnterProcChannel(Logger& logger, int channel_id)
    {
        Channel& channel = logger.shm_->channels_[channel_id];
        RingBuffer& ring_buffer = logger.shm_->ring_buffers_[channel_id];
        do
        {
            int flush_count = 0;
            bool empty_tick = true;
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
                AtomicIncChannelLog(channel, CHANNEL_LOG_PROCESSED, 1);
                AtomicIncChannelLog(channel, CHANNEL_LOG_PROCESSED_BYTES, cur_log.content_len_);
                flush_count ++;
                empty_tick = false;
                int write_id = ring_buffer.write_idx_.load(std::memory_order_acquire);
                int proc_que_size = 0;
                if (old_idx <= write_id)
                {
                    proc_que_size = write_id - old_idx;
                }
                else
                {
                    proc_que_size = write_id + RingBuffer::BUFFER_LEN - old_idx;
                }

                if (proc_que_size > AtomicLoadChannelLog(channel, CHANNEL_LOG_MAX_PROC_QUE_SIZE))
                {
                    AtomicStoreChannelLog(channel, CHANNEL_LOG_MAX_PROC_QUE_SIZE, proc_que_size);
                }

                if (cur_log.timestamp_ > 0)
                {
                    long long now = (long long)time(NULL);
                    long long diff = now - 1 - cur_log.timestamp_;
                    if (diff > AtomicLoadChannelLog(channel, CHANNEL_LOG_MAX_DELAY_TIME_S) )
                    {
                        AtomicStoreChannelLog(channel, CHANNEL_LOG_MAX_DELAY_TIME_S, diff);
                    }
                }
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
                if (flush_count > FN_LOG_FORCE_FLUSH_QUE)
                {
                    //break;
                    flush_count = 0;
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

            if (flush_count != 0)
            {
                for (int i = 0; i < channel.device_size_; i++)
                {
                    if (channel.devices_[i].out_type_ == DEVICE_OUT_FILE)
                    {
                        logger.file_handles_[channel_id * Channel::MAX_DEVICE_SIZE + i].flush();
                    }
                }
            }

            for (int i = 0; i < channel.device_size_; i++)
            {
                Device& device = channel.devices_[i];
                if (device.in_type_ != DEVICE_IN_UDP)
                {
                    continue;
                }

                UDPHandler& udp = logger.udp_handles_[channel_id * Channel::MAX_DEVICE_SIZE + i];
                if (!device.config_fields_[DEVICE_CFG_ABLE])
                {
                    if (udp.is_open())
                    {
                        udp.close();
                    }
                    continue;
                }
                
                if (!udp.is_open())
                {
                    udp.open();
                    if (udp.is_open())
                    {
                        int ret = udp.bind((unsigned int)AtomicLoadC(device, DEVICE_CFG_UDP_IP), (unsigned short)AtomicLoadC(device, DEVICE_CFG_UDP_PORT));
                        if (ret != 0)
                        {
                            udp.close();
                        }
                    }
                }

                if (!udp.is_open())
                {
                    continue;
                }
                
                for (int i = 0; i < 1000; i++)
                {
                    InitLogData(logger, ring_buffer.udp_buffer_, channel.channel_id_, PRIORITY_INFO, 0, 0, LOG_PREFIX_NULL);
                    ring_buffer.udp_buffer_.content_len_ = udp.read(ring_buffer.udp_buffer_.content_, LogData::LOG_SIZE);
                    if (ring_buffer.udp_buffer_.content_len_ == LogData::LOG_SIZE)
                    {
                        ring_buffer.udp_buffer_.content_[ring_buffer.udp_buffer_.content_len_ - 2] = '\n';
                        ring_buffer.udp_buffer_.content_[ring_buffer.udp_buffer_.content_len_ - 1] = '\0';
                    }
                    if (ring_buffer.udp_buffer_.content_len_ == 0)
                    {
                        break;
                    }
                    empty_tick = false; 
                    EnterProcDevice(logger, channel.channel_id_, device.device_id_, ring_buffer.udp_buffer_);
                }
            }

            HotUpdateLogger(logger, channel.channel_id_);
            if (channel.channel_type_ == CHANNEL_ASYNC && empty_tick)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(FN_LOG_MAX_ASYNC_SLEEP_MS));
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
        (void)logger;
        (void)prefix;
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
#if FNLOG_GCC
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
        if (priority >= PRIORITY_MAX)
        {
            static_assert(PRIORITY_MAX == PRIORITY_FATAL + 1, "safety priority to record channel log CHANNEL_LOG_PRIORITY");
            priority = PRIORITY_FATAL;
        }

        long long begin_category = AtomicLoadC(channel, CHANNEL_CFG_CATEGORY);
        long long category_count = AtomicLoadC(channel, CHANNEL_CFG_CATEGORY_EXTEND);
        unsigned long long category_mask = (unsigned long long)AtomicLoadC(channel, CHANNEL_CFG_CATEGORY_MASK);
        long long begin_identify = AtomicLoadC(channel, CHANNEL_CFG_IDENTIFY);
        long long identify_count = AtomicLoadC(channel, CHANNEL_CFG_IDENTIFY_EXTEND);
        unsigned long long identify_mask = (unsigned long long)AtomicLoadC(channel, CHANNEL_CFG_IDENTIFY_MASK);

        if (category_count > 0 && (category < begin_category || category >= begin_category + category_count))
        {
            return true;
        }
        if (identify_count > 0 && (identify < begin_identify || identify >= begin_identify + identify_count))
        {
            return true;
        }
        if (category_mask && (category_mask & ((1ULL) << (unsigned int)category)) == 0)
        {
            return true;
        }
        if (identify_mask && (identify_mask & ((1ULL) << (unsigned int)identify)) == 0)
        {
            return true;
        }

        bool need_write = false;
        
        for (int i = 0; i < channel.device_size_; i++)
        {
            Device::ConfigFields& fields = channel.devices_[i].config_fields_;
            long long field_able = fields[FNLog::DEVICE_CFG_ABLE].load(std::memory_order_relaxed);
            long long field_priority = fields[FNLog::DEVICE_CFG_PRIORITY].load(std::memory_order_relaxed);
            if (!field_able || priority < field_priority)
            {
                continue;
            }
            if (channel.devices_[i].in_type_ != DEVICE_IN_NULL)
            {
                continue;
            }
            long long field_begin_category = fields[FNLog::DEVICE_CFG_CATEGORY].load(std::memory_order_relaxed);
            long long field_category_count = fields[FNLog::DEVICE_CFG_CATEGORY_EXTEND].load(std::memory_order_relaxed);
            unsigned long long field_category_mask = (unsigned long long)fields[FNLog::DEVICE_CFG_CATEGORY_MASK].load(std::memory_order_relaxed);
            long long field_begin_identify = fields[FNLog::DEVICE_CFG_IDENTIFY].load(std::memory_order_relaxed);
            long long field_identify_count = fields[FNLog::DEVICE_CFG_IDENTIFY_EXTEND].load(std::memory_order_relaxed);
            unsigned long long field_identify_mask = (unsigned long long)fields[FNLog::DEVICE_CFG_IDENTIFY_MASK].load(std::memory_order_relaxed);


            if (field_category_count > 0 && (category < field_begin_category || category >= field_begin_category + field_category_count))
            {
                continue;
            }
            if (field_identify_count > 0 && (identify < field_begin_identify || identify >= field_begin_identify + field_identify_count))
            {
                continue;
            }
            if (field_category_mask &&  (field_category_mask & ((1ULL) << (unsigned int)category)) == 0)
            {
                continue;
            }
            if (field_identify_mask &&  (field_identify_mask & ((1ULL) << (unsigned int)identify)) == 0)
            {
                continue;
            }
            need_write = true;
            break;
        }
        if (!need_write)
        {
            return true;
        }
        
        return false;
    }
#if FNLOG_GCC
#pragma GCC pop_options
#endif
    inline int HoldChannel(Logger& logger, int channel_id, int priority, int category, long long identify)
    {
        if (channel_id >= logger.shm_->channel_size_)
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
                AtomicIncChannelLog(channel, CHANNEL_LOG_WAIT_COUNT, 1);
                std::this_thread::sleep_for(std::chrono::milliseconds(FN_LOG_MAX_ASYNC_SLEEP_MS));
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
                    AtomicIncChannelLog(channel, CHANNEL_LOG_HOLD, 1);
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
            return E_INVALID_CHANNEL_SIZE;
        }
        if (hold_idx >= RingBuffer::BUFFER_LEN || hold_idx < 0)
        {
            return E_OUT_RINGBUFFER;
        }
        Channel& channel = logger.shm_->channels_[channel_id];
        RingBuffer& ring_buffer = logger.shm_->ring_buffers_[channel_id];
        if (channel.channel_state_ != CHANNEL_STATE_RUNNING)
        {
            return E_LOGGER_NOT_RUNNING;
        }

        LogData& log = ring_buffer.buffer_[hold_idx];
        log.content_len_ = FN_MIN(log.content_len_, LogData::LOG_SIZE - 2);
        log.content_[log.content_len_++] = '\n';
        log.content_[log.content_len_] = '\0';

        log.data_mark_ = 2;
        AtomicIncChannelLog(channel, CHANNEL_LOG_PRIORITY + log.priority_, log.content_len_);

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
                AtomicIncChannelLog(channel, CHANNEL_LOG_PUSH, 1);
            }
        } while (channel.channel_state_ == CHANNEL_STATE_RUNNING);

        if (channel.channel_type_ == CHANNEL_SYNC && channel.channel_state_ == CHANNEL_STATE_RUNNING)
        {
            EnterProcChannel(logger, channel_id); //no affect channel.single_thread_write_
        }
        return 0;
    }

    //combine virtual device  can transmit log to other channel 
    inline int TransmitChannel(Logger& logger, int channel_id, int category, long long identify, const LogData& log)
    {
        if (log.channel_id_ == channel_id)
        {
            return 0;
        }
        if (BlockInput(logger, channel_id, log.priority_, category, identify))
        {
            return 0;
        }

        int hold_idx = FNLog::HoldChannel(logger, channel_id, log.priority_, category, identify);
        if (hold_idx < 0)
        {
            if (hold_idx == -1)
            {
                return 0;
            }
            return 0;
        }
        LogData& trans_log = logger.shm_->ring_buffers_[channel_id].buffer_[hold_idx];
        trans_log.channel_id_ = channel_id;
        trans_log.priority_ = log.priority_;
        trans_log.category_ = category;
        trans_log.identify_ = identify;
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
