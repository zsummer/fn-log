
/*
* Copyright (C) 2019 YaweiZhang <yawei.zhang@foxmail.com>.
* All rights reserved
* This file is part of the fn-log, used MIT License.
*/

#pragma once
#ifndef _FN_LOG_OUT_FILE_DEVICE_H_
#define _FN_LOG_OUT_FILE_DEVICE_H_

#include "fn_data.h"

namespace FNLog
{

    //support
    inline std::string PreFmtName(const std::string& fmt_name)
    {
        if (fmt_name.empty())
        {
            return fmt_name;
        }
        std::string name = fmt_name;
        size_t pos = 0;
        do
        {
            bool has_error = false;
            pos = name.find('%', pos);
            if (pos == std::string::npos)
            {
                break;
            }
            if (name.length() - pos < 2)//min(escape) 
            {
                break;
            }

            switch (name[pos + 1])
            {
            case 'F':
                if (true)
                {
                    name.replace(pos, 2, "$YEAR-$MON-$DAY");
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
                pos++;
            }
        } while (true);
        return name;
    }
    // 
    inline std::string FmtName(const std::string& fmt_name, int channel_id, int device_id, const struct tm& t)
    {
        (void)device_id;
        (void)channel_id;
        if (fmt_name.empty())
        {
            return fmt_name;
        }

        std::string name = PreFmtName(fmt_name);

        size_t pos = 0;
        do
        {
            bool has_error = false;
            pos = name.find('$', pos);
            if (pos == std::string::npos)
            {
                break;
            }
            if (name.length() - pos <2)//min(escape) 
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
                    char buff[30] = { 0 };
                    sprintf(buff, "%04d", t.tm_year + 1900);
                    name.replace(pos, 5, buff);
                    break;
                }
                has_error = true;
                break;
            case 'M':
                if (name.substr(pos + 2, 2) == "ON")
                {
                    char buff[30] = { 0 };
                    sprintf(buff, "%02d", t.tm_mon + 1);
                    name.replace(pos, 4, buff);
                    break;
                }
                if (name.substr(pos + 2, 2) == "IN")
                {
                    char buff[30] = { 0 };
                    sprintf(buff, "%02d", t.tm_min);
                    name.replace(pos, 4, buff);
                    break;
                }
                has_error = true;
                break;
            case 'D':
                if (name.substr(pos + 2, 2) == "AY")
                {
                    char buff[30] = { 0 };
                    sprintf(buff, "%02d", t.tm_mday);
                    name.replace(pos, 4, buff);
                    break;
                }
                has_error = true;
                break;
            case 'H':
                if (name.substr(pos + 2, 3) == "OUR")
                {
                    char buff[30] = { 0 };
                    sprintf(buff, "%02d", t.tm_hour);
                    name.replace(pos, 5, buff);
                    break;
                }
                has_error = true;
                break;
            case 'S':
                if (name.substr(pos + 2, 2) == "EC")
                {
                    char buff[30] = { 0 };
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
                pos++;
            }
        } while (true);
        return name;
    }


    //[$PNAME $PID $YEAR $MON $DAY $HOUR $MIN $SEC]
    inline std::string MakeFileName(const std::string& fmt_name, int channel_id, int device_id, const struct tm& t)
    {
        std::string name = fmt_name;
        if (name.empty())
        {
            name = "$PNAME_$YEAR$MON$DAY_$PID.";
            name += std::to_string(channel_id);
            name += std::to_string(device_id);
        }
        name += ".log";
        return FmtName(name, channel_id, device_id, t);
    }
    //[$PNAME $PID $YEAR $MON $DAY $HOUR $MIN $SEC]
    inline std::string MakePathName(const std::string& fmt_name, int channel_id, int device_id, const struct tm& t)
    {
        return FmtName(fmt_name, channel_id, device_id, t);
    }


    inline void OpenFileDevice(Logger & logger, Channel & channel, Device & device, FileHandler & writer, LogData & log)
    {
        (void)logger;

        bool close_file = false;
        bool limit_out = false;

        

        do
        {
            //rollback only limit size && rollback > 0 
            if (AtomicLoadC(device, DEVICE_CFG_FILE_LIMIT_SIZE) > 0 && AtomicLoadC(device, DEVICE_CFG_FILE_ROLLBACK) > 0
                && AtomicLoadL(device, DEVICE_LOG_CUR_FILE_SIZE) + log.content_len_ > AtomicLoadC(device, DEVICE_CFG_FILE_LIMIT_SIZE))
            {
                close_file = true;
                limit_out = true;
                break;
            }

            //daily rolling
            if (AtomicLoadC(device, DEVICE_CFG_FILE_ROLLDAILY))
            {
                if (log.timestamp_ < AtomicLoadL(device, DEVICE_LOG_CUR_FILE_CREATE_DAY)
                    || log.timestamp_ >= AtomicLoadL(device, DEVICE_LOG_CUR_FILE_CREATE_DAY) + 24 * 3600)
                {
                    close_file = true;
                }
                break;
            }

            //hourly rolling 
            if (AtomicLoadC(device, DEVICE_CFG_FILE_ROLLHOURLY))
            {
                if (log.timestamp_ < AtomicLoadL(device, DEVICE_LOG_CUR_FILE_CREATE_HOUR)
                    || log.timestamp_ >= AtomicLoadL(device, DEVICE_LOG_CUR_FILE_CREATE_HOUR) + 3600)
                {
                    close_file = true;
                }
                break;
            }

        } while (false);


        if (close_file)
        {
            AtomicStoreL(device, DEVICE_LOG_CUR_FILE_SIZE, 0);
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
        long long create_hour = 0;
        tm t = FileHandler::time_to_tm(log.timestamp_);
        if (true) //process day time   
        {
            tm day = t;
            day.tm_min = 0;
            day.tm_sec = 0;
            create_hour = mktime(&day);
            day.tm_hour = 0;
            create_day = mktime(&day);
        }

        std::string name = MakeFileName(device.out_file_, channel.channel_id_, device.device_id_, t);
        std::string path = MakePathName(device.out_path_, channel.channel_id_, device.device_id_, t);
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

        if (path.length() >= Device::MAX_PATH_LEN + Device::MAX_LOGGER_NAME_LEN)
        {
            AtomicStoreL(device, DEVICE_LOG_LAST_TRY_CREATE_ERROR, 1);
            AtomicStoreL(device, DEVICE_LOG_LAST_TRY_CREATE_TIMESTAMP, log.timestamp_);
            return;
        }

        if (AtomicLoadC(device, DEVICE_CFG_FILE_ROLLBACK) > 0 || AtomicLoadC(device, DEVICE_CFG_FILE_LIMIT_SIZE) > 0)
        {
            bool stuff_up = (bool)AtomicLoadC(device, DEVICE_CFG_FILE_STUFF_UP);
            if (!stuff_up || limit_out)
            {
                //when no rollback but has limit size. need try rollback once.
                long long limit_roll = device.config_fields_[DEVICE_CFG_FILE_ROLLBACK];
                limit_roll = limit_roll > 0 ? limit_roll : 1;
                FileHandler::rollback(path, 1, (int)limit_roll);
            }
        }

        struct stat file_stat;
        long writed_byte = writer.open(path.c_str(), "ab", file_stat);
        if (!writer.is_open())
        {
            AtomicStoreL(device, DEVICE_LOG_LAST_TRY_CREATE_ERROR, 2);
            AtomicStoreL(device, DEVICE_LOG_LAST_TRY_CREATE_TIMESTAMP, log.timestamp_);
            return;
        }
        
        AtomicAddL(device, DEVICE_LOG_LAST_TRY_CREATE_CNT);

        AtomicStoreL(device, DEVICE_LOG_LAST_TRY_CREATE_ERROR, 0);
        AtomicStoreL(device, DEVICE_LOG_LAST_TRY_CREATE_TIMESTAMP, 0);
        AtomicStoreL(device, DEVICE_LOG_CUR_FILE_CREATE_TIMESTAMP, log.timestamp_);
        AtomicStoreL(device, DEVICE_LOG_CUR_FILE_CREATE_DAY, create_day);
        AtomicStoreL(device, DEVICE_LOG_CUR_FILE_CREATE_HOUR, create_hour);
        AtomicStoreL(device, DEVICE_LOG_CUR_FILE_SIZE, writed_byte);
    }



    inline void EnterProcOutFileDevice(Logger& logger, int channel_id, int device_id, LogData& log)
    {
        Channel& channel = logger.shm_->channels_[channel_id];
        Device& device = channel.devices_[device_id];
        FileHandler& writer = logger.file_handles_[channel_id * Channel::MAX_DEVICE_SIZE + device_id];

        if (!writer.is_open() && AtomicLoadL(device, DEVICE_LOG_LAST_TRY_CREATE_TIMESTAMP) + 5 > log.timestamp_)
        {
            return;
        }
        OpenFileDevice(logger, channel, device, writer, log);
        if (!writer.is_open())
        {
            return;
        }
        writer.write(log.content_, log.content_len_);
        AtomicAddL(device, DEVICE_LOG_TOTAL_WRITE_LINE);
        AtomicAddLV(device, DEVICE_LOG_TOTAL_WRITE_BYTE, log.content_len_);
        AtomicAddLV(device, DEVICE_LOG_CUR_FILE_SIZE, log.content_len_);
        AtomicAddLV(device, DEVICE_LOG_PRIORITY + log.priority_, log.content_len_);
    }


}


#endif
