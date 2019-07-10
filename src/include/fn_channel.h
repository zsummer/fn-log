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
#include "fn_mem.h"
#include "fn_fmt.h"

namespace FNLog
{
    
    inline void EnterProcDevice(Logger& logger, int channel_id, int device_id, LogData & log)
    {
        Channel& channel = logger.channels_[channel_id];
        Device& device = channel.devices_[device_id];
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
        default:
            break;
        }
    }
    
    inline void DispatchLog(Logger & logger, Channel& channel, LogData& log)
    {
        for (int device_id = 0; device_id < channel.device_size_; device_id++)
        {
            Device& device = channel.devices_[device_id];
            if (!device.config_fields_[DEVICE_CFG_ABLE].num_)
            {
                continue;
            }
            if (log.priority_ < device.config_fields_[DEVICE_CFG_PRIORITY].num_)
            {
                continue;
            }
            if (device.config_fields_[DEVICE_CFG_CATEGORY].num_ > 0)
            {
                if (log.category_ < device.config_fields_[DEVICE_CFG_CATEGORY].num_
                    || log.category_ > device.config_fields_[DEVICE_CFG_CATEGORY].num_ + device.config_fields_[DEVICE_CFG_CATEGORY_EXTEND].num_)
                {
                    continue;
                }
            }
            EnterProcDevice(logger, channel.channel_id_, device_id, log);
        }
    }
    
    inline bool EnterProcAsyncChannel(Logger & logger, int channel_id)
    {
        Channel& channel = logger.channels_[channel_id];
        std::mutex& write_lock = logger.syncs_[channel_id].write_lock_;

        do
        {
            if (channel.red_black_queue_[channel.write_red_].log_count_)
            {
                int revert_color = (channel.write_red_ + 1) % 2;

                auto & local_que = channel.red_black_queue_[channel.write_red_];

                write_lock.lock();
                channel.write_red_ = revert_color;
                write_lock.unlock();


                //consume all log from local queue
                for (int cur_log_id = 0; cur_log_id < local_que.log_count_; cur_log_id++)
                {
                    auto& cur_log = local_que.log_queue_[cur_log_id];
                    LogData& log = *cur_log;
                    DispatchLog(logger, channel, log);
                    FreeLogData(logger, channel_id, cur_log);
                    channel.log_fields_[CHANNEL_LOG_PROCESSED].num_++;
                }
                local_que.log_count_ = 0;
            }
            if (!channel.actived_ && !logger.waiting_close_)
            {
                channel.actived_ = true;
            }

            if (!channel.red_black_queue_[channel.write_red_].log_count_)
            {
                for (int i = 0; i < channel.device_size_; i++)
                {
                    if (channel.devices_[i].out_type_ == DEVICE_OUT_FILE)
                    {
                        logger.file_handles_[channel_id + channel_id * i].flush();
                    }
                }
                HotUpdateLogger(logger, channel.channel_id_);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        } while (channel.actived_ || channel.red_black_queue_[channel.write_red_].log_count_);
        return true;
    }
    
    
    inline bool EnterProcSyncChannel(Logger & logger, int channel_id)
    {
        Channel& channel = logger.channels_[channel_id];
        auto & local_que = channel.red_black_queue_[channel.write_red_];
        
        if (local_que.log_count_ > 0)
        {
            //consume all log from local queue
            for (int cur_log_id = 0; cur_log_id < local_que.log_count_; cur_log_id++)
            {
                auto& cur_log = local_que.log_queue_[cur_log_id];
                LogData& log = *cur_log;
                DispatchLog(logger, channel, log);
                FreeLogData(logger, channel_id, cur_log);
                channel.log_fields_[CHANNEL_LOG_PROCESSED].num_++;
            }
            local_que.log_count_ = 0;
        }
        for (int i = 0; i < channel.device_size_; i++)
        {
            if (channel.devices_[i].out_type_ == DEVICE_OUT_FILE)
            {
                logger.file_handles_[channel_id + channel_id * i].flush();
            }
        }
        HotUpdateLogger(logger, channel.channel_id_);
        return true;
    }
    
    inline bool EnterProcRingChannel(Logger & logger, int channel_id)
    {
        Channel& channel = logger.channels_[channel_id];
        auto & local_que = channel.red_black_queue_[channel.write_red_];
        do
        {
            while (local_que.write_count_ != local_que.read_count_)
            {
                auto& cur_log = local_que.log_queue_[local_que.read_count_];
                LogQueue::SizeType next_read = (local_que.read_count_ + 1) % LogQueue::MAX_LOG_QUEUE_SIZE;
                LogData& log = *cur_log;
                DispatchLog(logger, channel, log);
                FreeLogData(logger, channel_id, cur_log);
                local_que.read_count_ = next_read;
                channel.log_fields_[CHANNEL_LOG_PROCESSED].num_++;
            }
            
            if (!channel.actived_ && !logger.waiting_close_)
            {
                channel.actived_ = true;
            }

            if (local_que.write_count_ == local_que.read_count_)
            {
                for (int i = 0; i < channel.device_size_; i++)
                {
                    if (channel.devices_[i].out_type_ == DEVICE_OUT_FILE)
                    {
                        logger.file_handles_[channel_id + channel_id * i].flush();
                    }
                }
                HotUpdateLogger(logger, channel.channel_id_);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        } while (channel.actived_ || local_que.write_count_ != local_que.read_count_);
        return true;
    }
    
    inline bool EnterProcChannel(Logger & logger, int channel_id)
    {
        Channel& channel = logger.channels_[channel_id];
        switch (channel.channel_type_)
        {
            case CHANNEL_MULTI:
                return EnterProcAsyncChannel(logger, channel_id);
            case CHANNEL_RING:
                return EnterProcRingChannel(logger, channel_id);
            case CHANNEL_SYNC:
                return EnterProcSyncChannel(logger, channel_id);
        }
        return false;
    }
    

    inline int PushLogToChannel(Logger& logger, LogData* plog)
    {
        LogData& log = *plog;
        Channel& channel = logger.channels_[log.channel_id_];
        if (!channel.actived_)
        {
            return -1;
        }
        switch (channel.channel_type_)
        {
        case CHANNEL_MULTI:
        {
            unsigned int state = 0;
            do
            {
                if (state > 0)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
                state++;
                std::lock_guard<std::mutex> l(logger.syncs_[log.channel_id_].write_lock_);
                LogQueue& local_que = channel.red_black_queue_[channel.write_red_];
                if (local_que.log_count_ >= LogQueue::MAX_LOG_QUEUE_SIZE)
                {
                    continue;
                }
                local_que.log_queue_[local_que.log_count_++] = plog;
                return 0;
            } while (true);
        }
        break;
        case CHANNEL_RING:
        {
            unsigned int state = 0;
            do
            {
                if (state > 0)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
                state++;
                LogQueue& local_que = channel.red_black_queue_[channel.write_red_];
                LogQueue::SizeType next_write = (local_que.write_count_ + 1) % LogQueue::MAX_LOG_QUEUE_SIZE;
                if (next_write != local_que.read_count_)
                {
                    local_que.log_queue_[local_que.write_count_] = plog;
                    local_que.write_count_ = next_write;
                    return 0;
                }
            } while (true);
        }
        break;
        case CHANNEL_SYNC:
        {
            LogQueue& local_que = channel.red_black_queue_[channel.write_red_];
            if (local_que.log_count_ >= LogQueue::MAX_LOG_QUEUE_SIZE)
            {
                return -3;
            }
            local_que.log_queue_[local_que.log_count_++] = plog;
            EnterProcChannel(logger, log.channel_id_);
            return 0;
        }
        break;
        }
        return -2;
    }
}


#endif
