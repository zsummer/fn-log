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
  * PURPOSE:  FNLog is a cpp-based logging utility.
  * CREATION: 2019.4.20
  * RELEASED: 2019.6.27
  * QQGROUP:  524700770
  */


#pragma once
#ifndef _FN_LOG_CORE_H_
#define _FN_LOG_CORE_H_

#include "fn_channel.h"

namespace FNLog
{
    int CanPushLog(Logger& logger, int channel_id, int filter_level, int filter_cls)
    {
        if (channel_id >= logger.channel_size_ || channel_id < 0)
        {
            return -2;
        }
        Channel& channel = logger.channels_[channel_id];

        if (filter_level < channel.config_fields_[CHANNEL_CFG_FILTER_LEVEL].num_)
        {
            return 1;
        }
        if (channel.config_fields_[CHANNEL_CFG_VALID_CLS_BEGIN].num_ > 0)
        {
            if (filter_cls < channel.config_fields_[CHANNEL_CFG_VALID_CLS_BEGIN].num_
                || filter_cls >= channel.config_fields_[CHANNEL_CFG_VALID_CLS_BEGIN].num_ + channel.config_fields_[CHANNEL_CFG_VALID_CLS_COUNT].num_)
            {
                return 2;
            }
        }
        return 0;
    }

    int PushLog(Logger& logger, LogData* plog)
    {
        if (plog == nullptr)
        {
            return -1;
        }
        LogData& log = *plog;
        if (log.channel_id_ >= logger.channel_size_ || log.channel_id_ < 0)
        {
            FreeLogData(logger, log.channel_id_, plog);
            return -2;
        }
        plog->content_len_ = FN_MIN(plog->content_len_, LogData::MAX_LOG_SIZE - 2);
        plog->content_[plog->content_len_++] = '\n';
        plog->content_[plog->content_len_] = '\0';
        int ret = PushLogToChannel(logger, plog);
        if (ret != 0)
        {
            FreeLogData(logger, log.channel_id_, plog);
            return ret;
        }
        return 0;
    }

    void InitLogger(Logger& logger)
    {
        logger.last_error_ = 0;
        logger.hot_update_ = false;
        logger.waiting_close_ = false;
        logger.channel_size_ = 0;
        memset(&logger.channels_, 0, sizeof(logger.channels_));
    }

    //not thread-safe
    Channel* NewChannel(Logger& logger, int channel_type)
    {
        Channel * channel = nullptr;
        if (logger.channel_size_ < Logger::MAX_CHANNEL_SIZE) 
        {
            int channel_id = logger.channel_size_;
            logger.channel_size_++;
            channel = &logger.channels_[channel_id];
            channel->channel_id_ = channel_id;
            channel->channel_type_ = channel_type;
            return channel;
        }
        return channel;
    }

    //not thread-safe
    Device* NewDevice(Logger& logger, Channel& channel, int out_type)
    {
        Device* device = nullptr;
        if (channel.device_size_ < Channel::MAX_DEVICE_SIZE) {
            int device_id = channel.device_size_;
            channel.device_size_++;
            device = &channel.devices_[device_id];
            device->device_id_ = device_id;
            device->out_type_ = out_type;
            device->config_fields_[DEVICE_CFG_ABLE].num_ = 1;
            return device;
        }
        return device;
    }

    int StartLogger(Logger& logger)
    {
        if (logger.channel_size_ > Logger::MAX_CHANNEL_SIZE || logger.channel_size_ <= 0)
        {
            return -1;
        }

        for (int channel_id = 0; channel_id < logger.channel_size_; channel_id++)
        {
            Channel& channel = logger.channels_[channel_id];
            std::thread& thd = logger.syncs_[channel_id].log_thread_;
            LogData* log = AllocLogData(logger, channel_id, LOG_LEVEL_ALARM, 0);
            log->content_len_ += write_cstring(log->content_ + log->content_len_, LogData::MAX_LOG_SIZE - log->content_len_, "channel [");
            log->content_len_ += write_integer<10, 0>(log->content_ + log->content_len_, LogData::MAX_LOG_SIZE - log->content_len_, (long long)channel_id);
            log->content_len_ += write_cstring(log->content_ + log->content_len_, LogData::MAX_LOG_SIZE - log->content_len_, "] start.\n");
            log->content_[log->content_len_] = '\0';
            if (log->content_len_ <= 0)
            {
                log->content_len_ = 0;
                logger.last_error_ = -3;
            }
            PushLog(logger, log);
            if (logger.last_error_ != 0)
            {
                break;
            }
            if (channel.channel_type_ == CHANNEL_SYNC)
            {
                channel.actived_ = true;
            }
            else
            {
                thd = std::thread(EnterProcChannel, std::ref(logger), channel_id);
                int state = 0;
                while (!channel.actived_ && logger.last_error_ == 0 && state < 400)
                {
                    state++;
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
                if (!channel.actived_ && logger.last_error_ == 0)
                {
                    logger.last_error_ = -4;
                }
            }
            if (logger.last_error_ != 0)
            {
                break;
            }
        }
        return logger.last_error_;
    }

    int StopAndCleanLogger(Logger& logger)
    {
        logger.last_error_ = 0;
        if (logger.channel_size_ > Logger::MAX_CHANNEL_SIZE || logger.channel_size_ <= 0)
        {
            logger.last_error_ = -1;
            return logger.last_error_;
        }
        logger.waiting_close_ = true;

        for (int channel_id = 0; channel_id < logger.channel_size_; channel_id++)
        {
            Channel& channel = logger.channels_[channel_id];
            std::thread& thd = logger.syncs_[channel_id].log_thread_;
            if (channel.channel_type_ != CHANNEL_SYNC  && channel.actived_)
            {
                channel.actived_ = false;
                while (thd.joinable())
                {
                    thd.join();
                }
            }
            channel.actived_ = false;
        }
        for (int channel_id = 0; channel_id < logger.channel_size_; channel_id++)
        {
            Channel& channel = logger.channels_[channel_id];
            for (int i = 0; i < channel.red_black_queue_[channel.write_red_].log_count_; i++)
            {
                FreeLogData(logger, channel_id, channel.red_black_queue_[channel.write_red_].log_queue_[i]);
            }
            channel.red_black_queue_[channel.write_red_].log_count_ = 0;
            channel.write_red_ = (channel.write_red_ + 1 ) % 2;
            for (int i = 0; i < channel.red_black_queue_[channel.write_red_].log_count_; i++)
            {
                FreeLogData(logger, channel_id, channel.red_black_queue_[channel.write_red_].log_queue_[i]);
            }
            channel.red_black_queue_[channel.write_red_].log_count_ = 0;

            while (channel.red_black_queue_[channel.write_red_].write_count_ != channel.red_black_queue_[channel.write_red_].read_count_)
            {
                FreeLogData(logger, channel_id, channel.red_black_queue_[channel.write_red_].log_queue_[channel.red_black_queue_[channel.write_red_].read_count_]);
                channel.red_black_queue_[channel.write_red_].read_count_ = (channel.red_black_queue_[channel.write_red_].read_count_ + 1) % LogQueue::MAX_LOG_QUEUE_LEN;
            }
            channel.red_black_queue_[channel.write_red_].write_count_ = 0;
            channel.red_black_queue_[channel.write_red_].read_count_ = 0;

            for (int i = 0; i < channel.log_pool_.log_count_; i++)
            {
                if (logger.sys_free_)
                {
                    logger.sys_free_(channel.log_pool_.log_queue_[i]);
                }
                delete channel.log_pool_.log_queue_[i];
            }
            channel.log_pool_.log_count_ = 0;

            while (channel.log_pool_.write_count_ != channel.log_pool_.read_count_)
            {
                if (logger.sys_free_)
                {
                    logger.sys_free_(channel.log_pool_.log_queue_[channel.log_pool_.read_count_]);
                    channel.log_pool_.log_queue_[channel.log_pool_.read_count_] = nullptr;
                }
                else
                {
                    delete channel.log_pool_.log_queue_[channel.log_pool_.read_count_];
                    channel.log_pool_.log_queue_[channel.log_pool_.read_count_] = nullptr;
                }
                channel.log_pool_.read_count_ = (channel.log_pool_.read_count_ + 1) % Channel::MAX_FREE_POOL_SIZE;
            }
            channel.log_pool_.write_count_ = 0;
            channel.log_pool_.read_count_ = 0;
        }
        
        for (auto& writer : logger.file_handles_)
        {
            if (writer.is_open())
            {
                writer.close();
            }
        }
        for (auto& writer : logger.udp_handles_)
        {
            if (writer.is_open())
            {
                writer.close();
            }
        }
        logger.waiting_close_ = false;
        logger.channel_size_ = 0;
        return logger.last_error_;
    }

    int AutoStartLogger(Logger& logger)
    {
        int ret = StartLogger(logger);
        if (ret != 0)
        {
            StopAndCleanLogger(logger);
            return ret;
        }
        if (logger.last_error_ != 0)
        {
            StopAndCleanLogger(logger);
            return logger.last_error_;
        }
        return 0;
    }

    class GuardLogger
    {
    public:
        GuardLogger() = delete;
        explicit GuardLogger(Logger& logger) :logger_(logger) {}
        ~GuardLogger()
        {
            StopAndCleanLogger(logger_);
        }

    private:
        Logger& logger_;
    };
}


#endif
