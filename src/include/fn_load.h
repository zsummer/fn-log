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
#ifndef _FN_LOG_LOAD_H_
#define _FN_LOG_LOAD_H_

#include "fn_data.h"
#include "fn_parse.h"
#include <sstream>

namespace FNLog
{

    inline int InitFromYMAL(const std::string& text, const std::string& path, Logger& logger)
    {
        std::unique_ptr<LexState> ls(new LexState);
        int ret = ParseLogger(*ls, text);
        if (ret != 0)
        {
            std::stringstream os;
            os << "load has error:<" << ret << "> in line:[" << ls->line_number_ << "], line type:" << ls->line_.line_type_;
            if (ls->current_ != nullptr)
            {
                os << " before:";
                int limit = 0;
                while (limit < 10 && ls->current_[limit] != '\0')
                {
                    limit++;
                }
                os.write(ls->current_, limit);
            }
            printf("%s\n", os.str().c_str());
            return ret;
        }

        
        logger.last_error_ = 0;
        logger.yaml_path_ = path;
        logger.hot_update_ = ls->hot_update_;
        logger.channel_size_ = ls->channel_size_;
        memcpy(&logger.channels_, &ls->channels_, sizeof(logger.channels_));
        return 0;
    }

    inline int InitFromYMALFile(const std::string& path, Logger& logger)
    {
        std::unique_ptr<LexState> ls(new LexState);
        FileHandler config;
        static_assert(std::is_same<decltype(logger.channels_), decltype(ls->channels_)>::value, "");
        //static_assert(std::is_trivial<decltype(logger.channels_)>::value, "");

        struct stat file_stat;
        config.open(path.c_str(), "rb", file_stat);
        if (!config.is_open())
        {
            return -1;
        }
        int ret = InitFromYMAL(config.read_content(), path, logger);
        if (ret != 0)
        {
            return ret;
        }

        for (int i = 0; i < logger.channel_size_; i++)
        {
            logger.channels_[i].yaml_mtime_ = file_stat.st_mtime;
        }
        return 0;
    }

    inline int HotUpdateLogger(Logger& logger, int channel_id)
    {
        if (logger.channel_size_ <= channel_id)
        {
            return -1;
        }
        if (!logger.hot_update_)
        {
            return -2;
        }
        if (logger.yaml_path_.empty())
        {
            return -3;
        }

        Channel& dst_chl = logger.channels_[channel_id];
        time_t now = time(nullptr);
        if (now - dst_chl.last_hot_check_ < 5)
        {
            return 0;
        }
        dst_chl.last_hot_check_ = now;

        FileHandler config;
        struct stat file_stat;
        config.open(logger.yaml_path_.c_str(), "rb", file_stat);
        if (!config.is_open())
        {
            return -5;
        }
        if (file_stat.st_mtime == dst_chl.yaml_mtime_)
        {
            return -6;
        }
        dst_chl.yaml_mtime_ = file_stat.st_mtime;

        std::unique_ptr<LexState> ls(new LexState);
        static_assert(std::is_same<decltype(logger.channels_), decltype(ls->channels_)>::value, "");
        //static_assert(std::is_trivial<decltype(logger.channels_)>::value, "");

        std::string text = config.read_content();
        int ret = ParseLogger(*ls, text);
        if (ret != 0)
        {
            return ret+100;
        }
        logger.hot_update_ = ls->hot_update_;

        static_assert(std::is_same<decltype(logger.channels_[channel_id].config_fields_), decltype(ls->channels_[channel_id].config_fields_)>::value, "");
        
        Channel& src_chl = ls->channels_[channel_id];
        if (dst_chl.channel_id_ != src_chl.channel_id_ || src_chl.channel_id_ != channel_id)
        {
            return - 7;
        }
        for (int field_id = 0; field_id < CHANNEL_CFG_MAX_ID; field_id++)
        {
            //this is multi-thread safe op. 
            dst_chl.config_fields_[field_id] = src_chl.config_fields_[field_id];
        }

        //single thread op.
        for (int device_id = 0; device_id < src_chl.device_size_; device_id++)
        {
            Device& src_dvc = src_chl.devices_[device_id];
            if (src_dvc.device_id_ != device_id)
            {
                return -8;
            }
            if (device_id < dst_chl.device_size_)
            {
                Device& dst_dvc = dst_chl.devices_[device_id];
                if (dst_dvc.device_id_ != device_id)
                {
                    return -9;
                }
                memcpy(&dst_dvc.config_fields_, &src_dvc.config_fields_, sizeof(dst_dvc.config_fields_));
                continue;
            }
            if (dst_chl.device_size_ != device_id)
            {
                return -10;
            }
            memcpy(&dst_chl.devices_[dst_chl.device_size_++], &src_dvc, sizeof(src_dvc));
        }

        return 0;
    }
}


#endif
