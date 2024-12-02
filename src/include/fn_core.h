
/*
* Copyright (C) 2019 YaweiZhang <yawei.zhang@foxmail.com>.
* All rights reserved
* This file is part of the fn-log, used MIT License.
*/


#pragma once
#ifndef _FN_LOG_CORE_H_
#define _FN_LOG_CORE_H_

#include "fn_channel.h"

namespace FNLog
{

#if FNLOG_GCC && __GNUC__ >= 6
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess"
#endif

    inline int PushLog(Logger& logger, int channel_id, int hold_idx, bool state_safly_env = false)
    {
        (void)state_safly_env;
        return PushChannel(logger, channel_id, hold_idx);
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
                    printf("StartChannels %s", "start async log thread has error.\n");
                    return E_CHANNEL_THREAD_FAILED;
                }
                int state = 0;
                while (channel.channel_state_ == CHANNEL_STATE_NULL && state < 100)
                {
                    state++;
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
                if (channel.channel_state_ == CHANNEL_STATE_NULL)
                {
                    printf("StartChannels %s", "start async log thread timeout.\n");
                    return E_CHANNEL_THREAD_FAILED;
                }
                if (channel.channel_state_ != CHANNEL_STATE_RUNNING)
                {
                    printf("StartChannels %s", "start async log thread has inner error.\n");
                    return E_INVALID_CHANNEL_STATE;
                }
            }
            break;
            default:
                printf("%s", "unknown channel type");
                return E_INVALID_CHANNEL_SYNC;
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
                printf("StopChannels %s", "unknown channel type");
                return E_INVALID_CHANNEL_SYNC;
            }
        }
        return 0;
    }

    inline int StartLogger(Logger& logger)
    {
        if (logger.logger_state_ != LOGGER_STATE_UNINIT)
        {
            printf("StartLogger error. state:<%u> not uninit:<%u>.\n", logger.logger_state_, LOGGER_STATE_UNINIT);
            return E_LOGGER_IN_USE;
        }
        if (logger.shm_ == NULL)
        {
            printf("StartLogger error. logger not init %p.\n", logger.shm_);
            return E_LOGGER_NOT_INIT;
        }
        if (logger.shm_->channel_size_ > Logger::MAX_CHANNEL_SIZE || logger.shm_->channel_size_ <= 0)
        {
            printf("StartLogger error. logger invalid channel size: %d.\n", logger.shm_->channel_size_);
            return E_INVALID_CHANNEL_SIZE;
        }
        Logger::StateLockGuard state_guard(logger.state_lock_);
        if (logger.logger_state_ != LOGGER_STATE_UNINIT)
        {
            printf("StartLogger error. state:<%u> not uninit:<%u>.\n", logger.logger_state_, LOGGER_STATE_UNINIT);
            return E_LOGGER_IN_USE;
        }
        if (logger.shm_->channel_size_ > Logger::MAX_CHANNEL_SIZE || logger.shm_->channel_size_ <= 0)
        {
            printf("StartLogger error. channel size:<%d> invalid.\n", logger.shm_->channel_size_);
            return E_INVALID_CHANNEL_SIZE;
        }
        logger.logger_state_ = LOGGER_STATE_INITING;
        int ret = StartChannels(logger);
        if (ret != 0)
        {
            StopChannels(logger);
            logger.logger_state_ = LOGGER_STATE_UNINIT;
            printf("StartLogger error. StartChannels failed. channel size:<%d>. \n", logger.shm_->channel_size_);
            return ret;
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
            printf("StopLogger error. channel size:<%d> invalid.\n", logger.shm_->channel_size_);
            return E_INVALID_CHANNEL_SIZE;
        }
        if (logger.logger_state_ != LOGGER_STATE_RUNNING)
        {
            printf("StopLogger logger error. state:<%u> not running:<%u>.\n", logger.logger_state_, LOGGER_STATE_RUNNING);
            return E_LOGGER_NOT_RUNNING;
        }
        Logger::StateLockGuard state_guard(logger.state_lock_);
        
        if (logger.logger_state_ != LOGGER_STATE_RUNNING)
        {
            printf("StopLogger logger error. state:<%u> not running:<%u>.\n", logger.logger_state_, LOGGER_STATE_RUNNING);
            return E_LOGGER_NOT_RUNNING;
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
            printf("ParseAndStartLogger error. state:<%u> not uninit:<%u> by fast try.\n", logger.logger_state_, LOGGER_STATE_UNINIT);
            return E_LOGGER_IN_USE;
        }
        Logger::StateLockGuard state_guard(logger.state_lock_);
        if (logger.logger_state_ != LOGGER_STATE_UNINIT)
        {
            printf("ParseAndStartLogger error. state:<%u> not uninit:<%u> in locked check.\n", logger.logger_state_, LOGGER_STATE_UNINIT);
            return E_LOGGER_IN_USE;
        }
        int ret = InitFromYMAL(logger, config_content, "");
        if (ret != 0)
        {
            printf("ParseAndStartLogger error. ret:<%d>:%s.\n", ret, DebugErrno(ret).c_str());
            return ret;
        }
        ret = StartLogger(logger);
        if (ret != 0)
        {
            printf("ParseAndStartLogger error. ret:<%d>.\n", ret);
            return ret;
        }
        return 0;
    }

    inline int LoadAndStartLogger(Logger& logger, const std::string& confg_path)
    {
        if (logger.logger_state_ != LOGGER_STATE_UNINIT)
        {
            printf("LoadAndStartLogger error. state:<%u> not uninit:<%u>.\n", logger.logger_state_, LOGGER_STATE_UNINIT);
            return E_LOGGER_IN_USE;
        }
        Logger::StateLockGuard state_guard(logger.state_lock_);
        int ret = InitFromYMALFile(logger, confg_path);
        if (ret != 0)
        {
            printf("LoadAndStartLogger error. ret:<%d %s>.\n", ret, DebugErrno(ret).c_str());
            return ret;
        }
        ret = StartLogger(logger);
        if (ret != 0)
        {
            printf("LoadAndStartLogger error. ret:<%d>.\n", ret);
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
        return AtomicLoadChannelLog(channel, field);
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
        return AtomicLoadDeviceLog(channel, device_id, field);
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
            channel.config_fields_[cce] = v;
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
                if (device.out_type_ == (unsigned int)out_type || out_type == DEVICE_OUT_NULL)
                {
                    device.config_fields_[dce] = v;
                }
            }
        }
    }



    inline void InitLogger(Logger& logger)
    {
        logger.hot_update_ = false;
        logger.logger_state_ = LOGGER_STATE_UNINIT;
        memset(logger.desc_, 0, Logger::MAX_LOGGER_DESC_LEN);
        logger.desc_len_ = 0;
        memset(logger.name_, 0, Logger::MAX_LOGGER_NAME_LEN);
        logger.name_len_ = 0;
        memset(logger.freq_limits_, 0, sizeof(logger.freq_limits_));
        std::string name = FileHandler::process_name();
        name = name.substr(0, Logger::MAX_LOGGER_NAME_LEN - 1);
        memcpy(logger.name_, name.c_str(), name.length() + 1);
        logger.name_len_ = (int)name.length();
        logger.shm_key_ = 0;
        logger.shm_ = NULL;
        

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

#if FNLOG_GCC && __GNUC__ >= 6
#pragma GCC diagnostic pop
#endif

}


#endif
