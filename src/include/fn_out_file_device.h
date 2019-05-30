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
  * PURPOSE:  fn_log is a cpp-based logging utility.
  * CREATION: 2019.4.20
  * RELEASED: 2019.6.27
  * QQGROUP:  524700770
  */

#pragma once
#ifndef _FN_LOG_OUT_FILE_DEVICE_H_
#define _FN_LOG_OUT_FILE_DEVICE_H_

#include "fn_data.h"

namespace FNLog
{

    //support
    //[$PNAME $PID $YEAR $MON $DAY $HOUR $MIN $SEC]
    inline std::string MakeFileName(Channel& channel, Device& device, const struct tm& t, LogData& log)
    {
        std::string name = device.out_file_;
        if (name.empty())
        {
            name = "$PNAME_$YEAR$MON$DAY_$PID.";
            name += std::to_string(channel.channel_id_);
            name += std::to_string(device.device_id_);
        }
        name += ".log";
        size_t pos = 0;
        do
        {
            bool has_error = false;
            pos = name.find('$', pos);
            if (pos == std::string::npos)
            {
                break;
            }
            if (name.length() - pos < 8)//min(escape) + ".log"
            {
                break;
            }

            switch (name[pos + 1])
            {
            case 'P':
                if (name.substr(pos + 2, 4) == "NAME")
                {
                    name.replace(pos, 6, FileHandler::process_name());
                    break;
                }
                if (name.substr(pos + 2, 2) == "ID")
                {
                    name.replace(pos, 4, FileHandler::process_id());
                    break;
                }
                has_error = true;
                break;
            case 'Y':
                if (name.substr(pos + 2, 3) == "EAR")
                {
                    char buff[8] = { 0 };
                    sprintf(buff, "%04d", t.tm_year + 1900);
                    name.replace(pos, 5, buff);
                    break;
                }
                has_error = true;
                break;
            case 'M':
                if (name.substr(pos + 2, 2) == "ON")
                {
                    char buff[8] = { 0 };
                    sprintf(buff, "%02d", t.tm_mon + 1);
                    name.replace(pos, 4, buff);
                    break;
                }
                if (name.substr(pos + 2, 2) == "IN")
                {
                    char buff[8] = { 0 };
                    sprintf(buff, "%02d", t.tm_min);
                    name.replace(pos, 4, buff);
                    break;
                }
                has_error = true;
                break;
            case 'D':
                if (name.substr(pos + 2, 2) == "AY")
                {
                    char buff[8] = { 0 };
                    sprintf(buff, "%02d", t.tm_mday);
                    name.replace(pos, 4, buff);
                    break;
                }
                has_error = true;
                break;
            case 'H':
                if (name.substr(pos + 2, 3) == "OUR")
                {
                    char buff[8] = { 0 };
                    sprintf(buff, "%02d", t.tm_hour);
                    name.replace(pos, 5, buff);
                    break;
                }
                has_error = true;
                break;
            case 'S':
                if (name.substr(pos + 2, 2) == "EC")
                {
                    char buff[8] = { 0 };
                    sprintf(buff, "%02d", t.tm_sec);
                    name.replace(pos, 4, buff);
                    break;
                }
                has_error = true;
                break;
            default:
                has_error = true;
                break;
            }

            if (has_error)
            {
                break;
            }
        } while (true);
        return name;
    }

    inline void OpenFileDevice(Logger & logger, Channel & channel, Device & device, FileHandler & writer, LogData & log)
    {
        bool sameday = true;
        if (log.timestamp_ < device.log_fields_[DEVICE_LOG_CUR_FILE_CREATE_DAY].num_
            || log.timestamp_ >= device.log_fields_[DEVICE_LOG_CUR_FILE_CREATE_DAY].num_ + 24 * 3600)
        {
            sameday = false;
        }

        bool file_over = false;
        if (device.config_fields_[DEVICE_CFG_FILE_LIMIT_SIZE].num_ > 0 && device.config_fields_[DEVICE_CFG_FILE_ROLLBACK].num_ > 0
            && device.log_fields_[DEVICE_LOG_CUR_FILE_SIZE].num_ + log.content_len_ > device.config_fields_[DEVICE_CFG_FILE_LIMIT_SIZE].num_)
        {
            file_over = true;
        }

        if (!sameday || file_over)
        {
            device.log_fields_[DEVICE_LOG_CUR_FILE_SIZE].num_ = 0;
            if (writer.is_open())
            {
                writer.close();
            }
        }

        if (writer.is_open())
        {
            return;
        }

        long long create_day = 0;
        tm t = FileHandler::time_to_tm(log.timestamp_);
        if (true) //process day time   
        {
            tm day = t;
            day.tm_hour = 0;
            day.tm_min = 0;
            day.tm_sec = 0;
            create_day = mktime(&day);
        }

        std::string name = MakeFileName(channel, device, t, log);

        std::string path = device.out_path_;
        if (!path.empty())
        {
            std::for_each(path.begin(), path.end(), [](char& ch) {if (ch == '\\') { ch = '/'; } });
            if (path.back() != '/') { path.push_back('/'); }

            if (!FileHandler::is_dir(path))
            {
                FileHandler::create_dir(path);
            }
        }

        path += name;

        if (path.length() >= Device::MAX_PATH_LEN + Device::MAX_NAME_LEN)
        {
            logger.last_error_ = -1;
            device.log_fields_[DEVICE_LOG_LAST_TRY_CREATE_TIMESTAMP].num_ = log.timestamp_;
            return;
        }

        if (device.config_fields_[DEVICE_CFG_FILE_ROLLBACK].num_ > 0 || device.config_fields_[DEVICE_CFG_FILE_LIMIT_SIZE].num_ > 0)
        {
            //when no rollback but has limit size. need try rollback once.
            long long limit_roll = device.config_fields_[DEVICE_CFG_FILE_ROLLBACK].num_;
            limit_roll = limit_roll > 0 ? limit_roll : 1;
            FileHandler::rollback(path, 1, (int)limit_roll);
        }

        struct stat file_stat;
        long writed_byte = writer.open(path.c_str(), "ab", file_stat);
        if (!writer.is_open())
        {
            logger.last_error_ = -2;
            device.log_fields_[DEVICE_LOG_LAST_TRY_CREATE_TIMESTAMP].num_ = log.timestamp_;
            return;
        }

        device.log_fields_[DEVICE_LOG_LAST_TRY_CREATE_TIMESTAMP].num_ = 0;
        device.log_fields_[DEVICE_LOG_CUR_FILE_CREATE_TIMESTAMP].num_ = log.timestamp_;
        device.log_fields_[DEVICE_LOG_CUR_FILE_CREATE_DAY].num_ = create_day;
        device.log_fields_[DEVICE_LOG_CUR_FILE_SIZE].num_ = writed_byte;
    }



    inline void EnterProcOutFileDevice(Logger& logger, int channel_id, int device_id, bool loop_end, LogData& log)
    {
        Channel& channel = logger.channels_[channel_id];
        Device& device = channel.devices_[device_id];
        FileHandler& writer = logger.file_handles_[channel_id + channel_id * device_id];

        if (!writer.is_open() && device.log_fields_[DEVICE_LOG_LAST_TRY_CREATE_TIMESTAMP].num_ + 5 > log.timestamp_)
        {
            return;
        }
        OpenFileDevice(logger, channel, device, writer, log);
        if (!writer.is_open())
        {
            return;
        }
        writer.write(log.content_, log.content_len_);
        device.log_fields_[DEVICE_LOG_TOTAL_WRITE_LINE].num_++;
        device.log_fields_[DEVICE_LOG_TOTAL_WRITE_BYTE].num_ += log.content_len_;
        device.log_fields_[DEVICE_LOG_CUR_FILE_SIZE].num_ += log.content_len_;
        if (loop_end)
        {
            writer.flush();
        }
    }


}


#endif
