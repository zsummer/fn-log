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
#ifndef _FN_LOG_CORE_H_
#define _FN_LOG_CORE_H_

#include "fn_channel.h"

namespace FNLog
{

#if __GNUG__ && __GNUC__ >= 6
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess"
#endif

    inline int PushLog(Logger& logger, int channel_id, int hold_idx, bool state_safly_env = false)
    {
        return PushChannel(logger, channel_id, hold_idx);
    }

    //not thread-safe
    inline Channel* NewChannel(Logger& logger, int channel_type)
    {
        Channel * channel = nullptr;
        if (logger.shm_->channel_size_ < Logger::MAX_CHANNEL_SIZE) 
        {
            int channel_id = logger.shm_->channel_size_;
            logger.shm_->channel_size_++;
            channel = &logger.shm_->channels_[channel_id];
            channel->channel_id_ = channel_id;
            channel->channel_type_ = channel_type;
            return channel;
        }
        return channel;
    }

    //not thread-safe
    inline Device* NewDevice(Logger& logger, Channel& channel, int out_type)
    {
        Device* device = nullptr;
        if (channel.device_size_ < Channel::MAX_DEVICE_SIZE) {
            int device_id = channel.device_size_;
            channel.device_size_++;
            device = &channel.devices_[device_id];
            device->device_id_ = device_id;
            device->out_type_ = out_type;
            device->config_fields_[DEVICE_CFG_ABLE] = 1;
            return device;
        }
        return device;
    }

    inline int StartChannels(Logger& logger)
    {
        for (int channel_id = 0; channel_id < logger.shm_->channel_size_; channel_id++)
        {
            static_assert(LogData::LOG_SIZE > Device::MAX_PATH_SYS_LEN * 2 + 100, "");
            Channel& channel = logger.shm_->channels_[channel_id];
            std::thread& thd = logger.async_threads[channel_id];
            switch (channel.channel_type_)
            {
            case CHANNEL_SYNC:
                channel.channel_state_ = CHANNEL_STATE_RUNNING;
                break;
            case CHANNEL_ASYNC:
            {
                thd = std::thread(EnterProcChannel, std::ref(logger), channel_id);
                if (!thd.joinable())
                {
                    printf("%s", "start async log thread has error.\n");
                    return -1;
                }
                int state = 0;
                while (channel.channel_state_ == CHANNEL_STATE_NULL && state < 100)
                {
                    state++;
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
                if (channel.channel_state_ == CHANNEL_STATE_NULL)
                {
                    printf("%s", "start async log thread timeout.\n");
                    return -2;
                }
                if (channel.channel_state_ != CHANNEL_STATE_RUNNING)
                {
                    printf("%s", "start async log thread has inner error.\n");
                    return -3;
                }
            }
            break;
            default:
                printf("%s", "unknown channel type");
                return -10;
            }
        }
        return 0;
    }

    inline int StopChannels(Logger& logger)
    {
        for (int channel_id = 0; channel_id < logger.shm_->channel_size_; channel_id++)
        {
            static_assert(LogData::LOG_SIZE > Device::MAX_PATH_SYS_LEN * 2 + 100, "");
            Channel& channel = logger.shm_->channels_[channel_id];
            std::thread& thd = logger.async_threads[channel_id];
            switch (channel.channel_type_)
            {
            case CHANNEL_SYNC:
                channel.channel_state_ = CHANNEL_STATE_NULL;
                break;
            case CHANNEL_ASYNC:
            {
                if (thd.joinable())
                {
                    if (channel.channel_state_ == CHANNEL_STATE_RUNNING)
                    {
                        channel.channel_state_ = CHANNEL_STATE_WAITING_FINISH;
                    }
                    thd.join();
                }
                channel.channel_state_ = CHANNEL_STATE_NULL;
            }
            break;
            default:
                printf("%s", "unknown channel type");
                return -10;
            }
        }
        return 0;
    }

    inline int StartLogger(Logger& logger)
    {
        if (logger.logger_state_ != LOGGER_STATE_UNINIT)
        {
            printf("start error. state:<%u> not uninit:<%u>.\n", logger.logger_state_, LOGGER_STATE_UNINIT);
            return -1;
        }
        if (logger.shm_->channel_size_ > Logger::MAX_CHANNEL_SIZE || logger.shm_->channel_size_ <= 0)
        {
            printf("start error. channel size:<%d> invalid.\n", logger.shm_->channel_size_);
            return -2;
        }
        Logger::StateLockGuard state_guard(logger.state_lock);
        if (logger.logger_state_ != LOGGER_STATE_UNINIT)
        {
            printf("start error. state:<%u> not uninit:<%u>.\n", logger.logger_state_, LOGGER_STATE_UNINIT);
            return -3;
        }
        if (logger.shm_->channel_size_ > Logger::MAX_CHANNEL_SIZE || logger.shm_->channel_size_ <= 0)
        {
            printf("start error. channel size:<%d> invalid.\n", logger.shm_->channel_size_);
            return -4;
        }
        logger.logger_state_ = LOGGER_STATE_INITING;
        if (StartChannels(logger) != 0)
        {
            StopChannels(logger);
            logger.logger_state_ = LOGGER_STATE_UNINIT;
            return -5;
        }
        logger.logger_state_ = LOGGER_STATE_RUNNING;
        return 0;
    }

    inline int CleanChannels(Logger& logger)
    {
        for (int channel_id = 0; channel_id < logger.shm_->channel_size_; channel_id++)
        {
            RingBuffer& ring_buffer = logger.shm_->ring_buffers_[channel_id];

            while (ring_buffer.read_idx_ != ring_buffer.write_idx_)
            {
                ring_buffer.buffer_[ring_buffer.read_idx_].data_mark_ = 0;
                ring_buffer.read_idx_ = (ring_buffer.read_idx_ + 1) % RingBuffer::BUFFER_LEN;
            }
            ring_buffer.read_idx_ = 0;
            ring_buffer.proc_idx_ = 0;
            ring_buffer.write_idx_ = 0;
            ring_buffer.hold_idx_ = 0;
        }
        return 0;
    }


    inline int StopLogger(Logger& logger)
    {
        if (logger.shm_->channel_size_ > Logger::MAX_CHANNEL_SIZE || logger.shm_->channel_size_ <= 0)
        {
            printf("try stop error. channel size:<%d> invalid.\n", logger.shm_->channel_size_);
            return -1;
        }
        if (logger.logger_state_ != LOGGER_STATE_RUNNING)
        {
            printf("try stop logger error. state:<%u> not running:<%u>.\n", logger.logger_state_, LOGGER_STATE_RUNNING);
            return -2;
        }
        Logger::StateLockGuard state_guard(logger.state_lock);
        
        if (logger.logger_state_ != LOGGER_STATE_RUNNING)
        {
            printf("try stop logger error. state:<%u> not running:<%u>.\n", logger.logger_state_, LOGGER_STATE_RUNNING);
            return -3;
        }
        logger.logger_state_ = LOGGER_STATE_CLOSING;
        StopChannels(logger);
        CleanChannels(logger);
        
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
        logger.logger_state_ = LOGGER_STATE_UNINIT;
        return 0;
    }


    inline int ParseAndStartLogger(Logger& logger, const std::string& config_content)
    {
        if (logger.logger_state_ != LOGGER_STATE_UNINIT)
        {
            printf("parse and start error. state:<%u> not uninit:<%u>.\n", logger.logger_state_, LOGGER_STATE_UNINIT);
            return -1;
        }
        Logger::StateLockGuard state_guard(logger.state_lock);
        int ret = InitFromYMAL(logger, config_content, "");
        if (ret != 0)
        {
            printf("init and load default logger error. ret:<%d>.\n", ret);
            return ret;
        }
        ret = StartLogger(logger);
        if (ret != 0)
        {
            printf("start default logger error. ret:<%d>.\n", ret);
            return ret;
        }
        return 0;
    }

    inline int LoadAndStartLogger(Logger& logger, const std::string& confg_path)
    {
        if (logger.logger_state_ != LOGGER_STATE_UNINIT)
        {
            printf("load and start error. state:<%u> not uninit:<%u>.\n", logger.logger_state_, LOGGER_STATE_UNINIT);
            return -1;
        }
        Logger::StateLockGuard state_guard(logger.state_lock);
        int ret = InitFromYMALFile(logger, confg_path);
        if (ret != 0)
        {
            printf("init and load default logger error. ret:<%d>.\n", ret);
            return ret;
        }
        ret = StartLogger(logger);
        if (ret != 0)
        {
            printf("start default logger error. ret:<%d>.\n", ret);
            return ret;
        }
        return 0;
    }


    inline long long GetChannelLog(Logger& logger, int channel_id, ChannelLogEnum field)
    {
        if (logger.shm_->channel_size_ <= channel_id || channel_id < 0)
        {
            return 0;
        }
        Channel& channel = logger.shm_->channels_[channel_id];
        if (field >= CHANNEL_LOG_MAX_ID)
        {
            return 0;
        }
        return AtomicLoadL(channel, field);
    }

    inline void SetChannelConfig(Logger& logger, int channel_id, ChannelConfigEnum field, long long val)
    {
        if (logger.shm_->channel_size_ <= channel_id || channel_id < 0)
        {
            return;
        }
        Channel& channel = logger.shm_->channels_[channel_id];
        if (field >= CHANNEL_CFG_MAX_ID)
        {
            return;
        }
        channel.config_fields_[field] = val;
    }

    inline long long GetDeviceLog(Logger& logger, int channel_id, int device_id, DeviceLogEnum field)
    {
        if (logger.shm_->channel_size_ <= channel_id || channel_id < 0)
        {
            return 0;
        }
        Channel& channel = logger.shm_->channels_[channel_id];
        if (field >= DEVICE_LOG_MAX_ID)
        {
            return 0;
        }
        if (channel.device_size_ <= device_id || device_id < 0)
        {
            return 0;
        }
        return AtomicLoadL(channel.devices_[device_id], field);
    }

    inline void SetDeviceConfig(Logger& logger, int channel_id, int device_id, DeviceConfigEnum field, long long val)
    {
        if (logger.shm_->channel_size_ <= channel_id || channel_id < 0)
        {
            return;
        }
        if (field >= DEVICE_CFG_MAX_ID)
        {
            return;
        }
        Channel& channel = logger.shm_->channels_[channel_id];
        if (channel.device_size_ <= device_id || device_id < 0)
        {
            return;
        }
        channel.devices_[device_id].config_fields_[field] = val;
    }

    inline long long GetDeviceConfig(Logger& logger, int channel_id, int device_id, DeviceConfigEnum field)
    {
        if (logger.shm_->channel_size_ <= channel_id || channel_id < 0)
        {
            return 0;
        }
        if (field >= DEVICE_CFG_MAX_ID)
        {
            return 0;
        }
        Channel& channel = logger.shm_->channels_[channel_id];
        if (channel.device_size_ <= device_id || device_id < 0)
        {
            return 0;
        }
        return  AtomicLoadC(channel.devices_[device_id], field);
    }

    inline void BatchSetChannelConfig(Logger& logger, ChannelConfigEnum cce, long long v)
    {
        for (int i = 0; i < logger.shm_->channel_size_; i++)
        {
            auto& channel = logger.shm_->channels_[i];
            channel.config_fields_[cce].store(v);
        }
    }

    inline void BatchSetDeviceConfig(Logger& logger, DeviceOutType out_type, DeviceConfigEnum dce, long long v)
    {
        for (int i = 0; i < logger.shm_->channel_size_; i++)
        {
            auto& channel = logger.shm_->channels_[i];
            for (int j = 0; j < channel.device_size_; j++)
            {
                auto& device = channel.devices_[j];
                if (device.out_type_ == out_type || out_type == DEVICE_OUT_NULL)
                {
                    device.config_fields_[dce].store(v);
                }
            }
        }
    }


    inline void LoadSharedMemory(Logger& logger)
    {
#if FN_LOG_USE_SHM && !defined(WIN32)
        SHMLogger* shm = nullptr;
        int idx = shmget(FN_LOG_SHM_KEY, 0, 0);
        if (idx < 0 && errno != ENOENT)
        {
            printf("shmget error. key:<0x%x>, errno:<%d>. can use 'ipcs -m', 'ipcrm -m' to view and clear.\n", 
                FN_LOG_SHM_KEY, errno);
            return;
        }

        if (idx < 0)
        {
            idx = shmget(FN_LOG_SHM_KEY, sizeof(SHMLogger), IPC_CREAT | IPC_EXCL | 0600);
            if (idx < 0)
            {
                printf("new shm. shmget error. key:<0x%x>, errno:<%d>.\n", FN_LOG_SHM_KEY, errno);
                return;
            }
            void* addr = shmat(idx, nullptr, 0);
            if (addr == nullptr || addr == (void*)-1)
            {
                printf("new shm. shmat error. key:<0x%x>, idx:<%d>, errno:<%d>.\n", FN_LOG_SHM_KEY, idx, errno);
                return;
            }
            memset(addr, 0, sizeof(SHMLogger));
            shm = (SHMLogger*)addr;
            shm->shm_size_ = sizeof(SHMLogger);
            shm->shm_id_ = idx;
        }
        else
        {
            void* addr = shmat(idx, nullptr, 0);
            if (addr == nullptr || addr == (void*)-1)
            {
                printf("shmat error. key:<%x>, idx:<%d>, errno:<%d>.\n", FN_LOG_SHM_KEY, idx, errno);
                return;
            }
            shm = (SHMLogger*)addr;
        }

        if (shm->shm_size_ != sizeof(SHMLogger) || shm->shm_id_ != idx)
        {
            printf("shm version error. key:<0x%x>, old id:<%d>, new id:<%d>, old size:<%d> new size:<%d>. "
                "can use 'ipcs -m', 'ipcrm -m' to view and clear.\n",
                FN_LOG_SHM_KEY, shm->shm_id_, idx, shm->shm_size_, (int)sizeof(SHMLogger));
            return;
        }
        for (int i = 0; i < shm->channel_size_; i++)
        {
            if (i >= SHMLogger::MAX_CHANNEL_SIZE)
            {
                return;
            }

            if (shm->ring_buffers_[i].write_idx_ >= RingBuffer::BUFFER_LEN
                || shm->ring_buffers_[i].write_idx_ < 0)
            {
                return;
            }

            while (shm->ring_buffers_[i].write_idx_.load() != shm->ring_buffers_[i].hold_idx_.load())
            {
                auto& log = shm->ring_buffers_[i].buffer_[shm->ring_buffers_[i].write_idx_];
                log.data_mark_ = 2;
                log.priority_ = PRIORITY_FATAL;
                std::string core_desc = "!!!core recover!!!";
                log.content_len_ = FN_MIN(log.content_len_, LogData::LOG_SIZE - (int)core_desc.length() -2 );
                memcpy(&log.content_[log.content_len_], core_desc.c_str(), core_desc.length());

                log.content_len_ += core_desc.length();
                log.content_[log.content_len_++] = '\n';
                log.content_[log.content_len_] = '\0';

                shm->ring_buffers_[i].write_idx_ = (shm->ring_buffers_[i].write_idx_ + 1) % RingBuffer::BUFFER_LEN;
            }
            shm->ring_buffers_[i].hold_idx_ = shm->ring_buffers_[i].write_idx_.load();

            if (shm->ring_buffers_[i].read_idx_ >= RingBuffer::BUFFER_LEN
                || shm->ring_buffers_[i].read_idx_ < 0)
            {
                return;
            }
            shm->ring_buffers_[i].proc_idx_ = shm->ring_buffers_[i].read_idx_.load();
            if (shm->ring_buffers_[i].read_idx_ != 0 || shm->ring_buffers_[i].write_idx_ != 0)
            {
                printf("attach shm channel:<%d>, write:<%d>, read:<%d> \n",
                    i, shm->ring_buffers_[i].write_idx_.load(), (int)shm->ring_buffers_[i].read_idx_.load());
            }
        }
        logger.shm_ = shm;
#else
        logger.shm_ = new SHMLogger();
        memset(logger.shm_, 0, sizeof(SHMLogger));
        
#endif
    }
    inline void UnloadSharedMemory(Logger& logger)
    {
#if FN_LOG_USE_SHM && !defined(WIN32)
        if (logger.shm_)
        {
            int idx = logger.shm_->shm_id_;
            shmdt(logger.shm_);
            shmctl(idx, IPC_RMID, nullptr);
            logger.shm_ = nullptr;
        }
#else
        if (logger.shm_)
        {
            delete logger.shm_;
            logger.shm_ = nullptr;
        }
#endif
    }

    inline void InitLogger(Logger& logger)
    {
        logger.hot_update_ = false;
        logger.logger_state_ = LOGGER_STATE_UNINIT;
        memset(logger.desc_, 0, Logger::MAX_LOGGER_DESC_LEN);
        logger.desc_len_ = 0;
        memset(logger.name_, 0, Logger::MAX_LOGGER_NAME_LEN);
        logger.name_len_ = 0;
        std::string name = FileHandler::process_name();
        name = name.substr(0, Logger::MAX_LOGGER_NAME_LEN - 1);
        memcpy(logger.name_, name.c_str(), name.length() + 1);
        logger.name_len_ = (int)name.length();
        LoadSharedMemory(logger);

#if ((defined WIN32) && !KEEP_INPUT_QUICK_EDIT)
        HANDLE input_handle = ::GetStdHandle(STD_INPUT_HANDLE);
        if (input_handle != INVALID_HANDLE_VALUE)
        {
            DWORD mode = 0;
            if (GetConsoleMode(input_handle, &mode))
            {
                mode &= ~ENABLE_QUICK_EDIT_MODE;
                mode &= ~ENABLE_INSERT_MODE;
                mode &= ~ENABLE_MOUSE_INPUT;
                SetConsoleMode(input_handle, mode);
            }
        }
#endif
    }

    inline Logger::Logger()
    {
        InitLogger(*this);
    }

    inline Logger::~Logger()
    {
        while (logger_state_ != LOGGER_STATE_UNINIT)
        {
            int ret = StopLogger(*this);
            if (ret != 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
            }
        } ;
        UnloadSharedMemory(*this);
    }

#if __GNUG__ && __GNUC__ >= 6
#pragma GCC diagnostic pop
#endif

}


#endif
