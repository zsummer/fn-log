
/*
* Copyright (C) 2019 YaweiZhang <yawei.zhang@foxmail.com>.
* All rights reserved
* This file is part of the fn-log, used MIT License.
*/

#pragma once
#ifndef _FN_LOG_LOAD_H_
#define _FN_LOG_LOAD_H_

#include "fn_data.h"
#include "fn_parse.h"
#include <sstream>

namespace FNLog
{
#if __GNUG__ && __GNUC__ >= 6
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess"
#endif
    

    inline int LoadSharedMemory(Logger& logger)
    {
#if !defined(WIN32)
        if (logger.shm_key_ <= 0)
        {
            logger.shm_ = new SHMLogger();
            memset(logger.shm_, 0, sizeof(SHMLogger));
            return 0;
        }
        SHMLogger* shm = nullptr;
        int idx = shmget(logger.shm_key_, 0, 0);
        if (idx < 0 && errno != ENOENT)
        {
            printf("shmget error. key:<0x%llx>, errno:<%d>. can use 'ipcs -m', 'ipcrm -m' to view and clear.\n",
                logger.shm_key_, errno);
            return E_SHMGET_PROBE_ERROR;
        }

        if (idx < 0)
        {
            idx = shmget(logger.shm_key_, sizeof(SHMLogger), IPC_CREAT | IPC_EXCL | 0600);
            if (idx < 0)
            {
                printf("new shm. shmget error. key:<0x%llx>, errno:<%d>.\n", logger.shm_key_, errno);
                return E_SHMGET_CREATE_ERROR;
            }
            void* addr = shmat(idx, nullptr, 0);
            if (addr == nullptr || addr == (void*)-1)
            {
                printf("new shm. shmat error. key:<0x%llx>, idx:<%d>, errno:<%d>.\n", logger.shm_key_, idx, errno);
                return E_SHMAT_ERROR;
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
                printf("shmat error. key:<%llx>, idx:<%d>, errno:<%d>.\n", logger.shm_key_, idx, errno);
                return E_SHMAT_ERROR;
            }
            shm = (SHMLogger*)addr;
        }

        if (shm->shm_size_ != sizeof(SHMLogger) || shm->shm_id_ != idx)
        {
            printf("shm version error. key:<0x%llx>, old id:<%d>, new id:<%d>, old size:<%d> new size:<%d>. "
                "can use 'ipcs -m', 'ipcrm -m' to view and clear.\n",
                logger.shm_key_, shm->shm_id_, idx, shm->shm_size_, (int)sizeof(SHMLogger));
            return E_SHM_VERSION_WRONG;
        }
        for (int i = 0; i < shm->channel_size_; i++)
        {
            if (i >= SHMLogger::MAX_CHANNEL_SIZE)
            {
                return E_SHM_VERSION_WRONG;
            }

            if (shm->ring_buffers_[i].write_idx_ >= RingBuffer::BUFFER_LEN
                || shm->ring_buffers_[i].write_idx_ < 0)
            {
                return E_SHM_VERSION_WRONG;
            }

            while (shm->ring_buffers_[i].write_idx_.load() != shm->ring_buffers_[i].hold_idx_.load())
            {
                auto& log = shm->ring_buffers_[i].buffer_[shm->ring_buffers_[i].write_idx_];
                log.data_mark_ = 2;
                log.priority_ = PRIORITY_FATAL;
                std::string core_desc = "!!!core recover!!!";
                log.content_len_ = FN_MIN(log.content_len_, LogData::LOG_SIZE - (int)core_desc.length() - 2);
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
                return -10;
            }
            shm->ring_buffers_[i].proc_idx_ = shm->ring_buffers_[i].read_idx_.load();
            if (shm->ring_buffers_[i].read_idx_ != 0 || shm->ring_buffers_[i].write_idx_ != 0)
            {
                printf("attach shm key:<0x%llx> channel:<%d>, write:<%d>, read:<%d> \n", logger.shm_key_,
                    i, shm->ring_buffers_[i].write_idx_.load(), (int)shm->ring_buffers_[i].read_idx_.load());
            }
        }
        logger.shm_ = shm;
#else
        logger.shm_ = new SHMLogger();
        memset(logger.shm_, 0, sizeof(SHMLogger));
#endif
        return 0;
    }
    inline void UnloadSharedMemory(Logger& logger)
    {
#if !defined(WIN32)
        if (logger.shm_ && logger.shm_key_ > 0)
        {
            int idx = logger.shm_->shm_id_;
            shmdt(logger.shm_);
            shmctl(idx, IPC_RMID, nullptr);
            logger.shm_ = nullptr;
        }
#endif
        if (logger.shm_)
        {
            delete logger.shm_;
            logger.shm_ = nullptr;
        }
    }


    inline int InitFromYMAL(Logger& logger, std::string text, const std::string& path)
    {
        Logger::StateLockGuard state_guard(logger.state_lock);
        if (logger.logger_state_ != LOGGER_STATE_UNINIT)
        {
            printf("InitFromYMAL:<%s> text error\n", path.c_str());
            return E_LOGGER_STATE_NOT_UNINIT;
        }

        std::unique_ptr<LexState> ls(new LexState);
        int ret = Preparse(text);
        if (ret != PEC_NONE)
        {
            printf("InitFromYMAL has error:%d,  yaml:%s\n", ret, text.c_str());
            return ret;
        }
        ret = ParseLogger(*ls, text);
        if (ret != PEC_NONE)
        {
            std::stringstream os;
            os << "ParseLogger has error:<" << ret << "> in line:[" << ls->line_number_ << "], line type:" << ls->line_.line_type_;
            if (ls->current_ != nullptr)
            {
                os << " before:";
                int limit = 0;
                while (limit < 30 && ls->current_ + limit < ls->end_ && ls->current_[limit] != '\0')
                {
                    limit++;
                }
                os.write(ls->current_, limit);
            }
            printf("%s\n", os.str().c_str());
            return ret;
        }
        if (ls->name_len_ > 0)
        {
            memcpy(logger.name_, ls->name_, ls->name_len_+1);
            logger.name_len_ = ls->name_len_;
        }
        if (ls->desc_len_ > 0)
        {
            memcpy(logger.desc_, ls->desc_, ls->desc_len_+1);
            logger.desc_len_ = ls->desc_len_;
        }
        logger.yaml_path_ = path;
        logger.hot_update_ = ls->hot_update_;
        logger.shm_key_ = ls->shm_key_;
        if (logger.shm_  == NULL)
        {
            ret = LoadSharedMemory(logger);
            if (ret != 0)
            {
                printf("InitFromYMAL has error:%d,  yaml:%s\n", ret, text.c_str());
                return ret;
            }
        }
        logger.shm_->channel_size_ = ls->channel_size_;
        for (int i = 0; i < ls->channel_size_; i++)
        {
            memcpy(&ls->channels_[i].log_fields_, &logger.shm_->channels_[i].log_fields_,
                sizeof(ls->channels_[i].log_fields_));
            for (int j = 0; j < ls->channels_[i].device_size_; j++)
            {
                memcpy(&ls->channels_[i].devices_[j].log_fields_, 
                    &logger.shm_->channels_[i].devices_[j].log_fields_,
                    sizeof(ls->channels_[i].devices_[j].log_fields_));
            }
        }
        memcpy(&logger.shm_->channels_, &ls->channels_, sizeof(logger.shm_->channels_));

        if (logger.shm_->channel_size_ > Logger::MAX_CHANNEL_SIZE || logger.shm_->channel_size_ <= 0)
        {
            printf("InitFromYMAL channel size:%d out channel max%d. \n", logger.shm_->channel_size_, Logger::MAX_CHANNEL_SIZE);
            return E_CONFIG_OUT_CHANNEL_MAX;
        }
        return 0;
    }

    inline int InitFromYMALFile(Logger& logger, const std::string& path)
    {
        std::unique_ptr<LexState> ls(new LexState);
        FileHandler config;
        static_assert(std::is_same<decltype(logger.shm_->channels_), decltype(ls->channels_)>::value, "");
        //static_assert(std::is_trivial<decltype(logger.shm_->channels_)>::value, "");

        struct stat file_stat;
        config.open(path.c_str(), "rb", file_stat);
        if (!config.is_open())
        {
            printf("InitFromYMALFile:<%s> open file error\n", path.c_str());
            return E_NOT_FIND_CONFIG_FILE;
        }
        int ret = InitFromYMAL(logger, config.read_content(), path);
        if (ret != 0)
        {
            printf("InitFromYMALFile:<%s> has parse/init error\n", path.c_str());
            return ret;
        }

        for (int i = 0; i < logger.shm_->channel_size_; i++)
        {
            logger.shm_->channels_[i].yaml_mtime_ = file_stat.st_mtime;
        }
        return 0;
    }

    inline int HotUpdateLogger(Logger& logger, int channel_id)
    {
        if (logger.shm_->channel_size_ <= channel_id)
        {
            return E_OUT_CHANNEL_SIZE;
        }
        if (!logger.hot_update_)
        {
            return E_CONFIG_DISABLE_HOTUPDATE;
        }
        if (logger.yaml_path_.empty())
        {
            return E_CONFIG_NOT_FROM_PATHFILE;
        }

        Channel& dst_chl = logger.shm_->channels_[channel_id];
        time_t now = time(nullptr);
        if (now - dst_chl.last_hot_check_ < Logger::HOTUPDATE_INTERVEL)
        {
            return 0;
        }
        dst_chl.last_hot_check_ = now;

        FileHandler config;
        struct stat file_stat;
        config.open(logger.yaml_path_.c_str(), "rb", file_stat);
        if (!config.is_open())
        {
            return E_NOT_FIND_CONFIG_FILE;
        }
        if (file_stat.st_mtime == dst_chl.yaml_mtime_)
        {
            return E_CONFIG_FILE_NOT_CHANGE;
        }

        Logger::StateLockGuard state_guard(logger.state_lock);
        if (logger.logger_state_ != LOGGER_STATE_RUNNING)
        {
            return E_LOGGER_STATE_NOT_RUNNING;
        }

        dst_chl.yaml_mtime_ = file_stat.st_mtime;

        std::unique_ptr<LexState> ls(new LexState);
        static_assert(std::is_same<decltype(logger.shm_->channels_), decltype(ls->channels_)>::value, "");
        //static_assert(std::is_trivial<decltype(logger.shm_->channels_)>::value, "");

        std::string text = config.read_content();
        int ret = Preparse(text);
        if (ret != PEC_NONE)
        {
            return ret;
        }
        ret = ParseLogger(*ls, text);
        if (ret != PEC_NONE)
        {
            return ret;
        }
        if (!logger.hot_update_)
        {
            return E_CONFIG_DISABLE_HOTUPDATE;
        }
        logger.hot_update_ = ls->hot_update_;

        static_assert(std::is_same<decltype(logger.shm_->channels_[channel_id].config_fields_), decltype(ls->channels_[channel_id].config_fields_)>::value, "");
        


        Channel& src_chl = ls->channels_[channel_id];
        if (dst_chl.channel_id_ != src_chl.channel_id_ || src_chl.channel_id_ != channel_id)
        {
            return E_CONFIG_VERSION_MISMATCH;
        }
        for (int field_id = 0; field_id < CHANNEL_CFG_MAX_ID; field_id++)
        {
            //this is multi-thread safe op. 
            dst_chl.config_fields_[field_id] = AtomicLoadC(src_chl, field_id);
        }

        //single thread op.
        for (int device_id = 0; device_id < src_chl.device_size_; device_id++)
        {
            Device& src_dvc = src_chl.devices_[device_id];
            if (src_dvc.device_id_ != device_id)
            {
                return E_CONFIG_VERSION_MISMATCH;
            }
            if (device_id < dst_chl.device_size_)
            {
                Device& dst_dvc = dst_chl.devices_[device_id];
                if (dst_dvc.device_id_ != device_id)
                {
                    return E_CONFIG_VERSION_MISMATCH;
                }
                memcpy(&dst_dvc.config_fields_, &src_dvc.config_fields_, sizeof(dst_dvc.config_fields_));
                continue;
            }
            if (dst_chl.device_size_ != device_id)
            {
                return E_CONFIG_VERSION_MISMATCH;
            }
            memcpy(&dst_chl.devices_[dst_chl.device_size_++], &src_dvc, sizeof(src_dvc));
            
        }

        return 0;
    }

#if __GNUG__ && __GNUC__ >= 6
#pragma GCC diagnostic pop
#endif
}


#endif
