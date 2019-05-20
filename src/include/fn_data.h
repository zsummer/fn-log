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
#ifndef _FN_LOG_DATA_H_
#define _FN_LOG_DATA_H_

#include "fn_file.h"


namespace FNLog
{
    enum ENUM_LOG_LEVEL
    {
        LOG_LEVEL_TRACE = 0,
        LOG_LEVEL_DEBUG,
        LOG_LEVEL_INFO,
        LOG_LEVEL_WARN,
        LOG_LEVEL_ERROR,
        LOG_LEVEL_ALARM,
        LOG_LEVEL_FATAL,
        LOG_LEVEL_MAX
    };
    

    union AnyVal
    {
        long long num_;
        double float_;
    };

    enum LogType
    {
        LOG_TYPE_NULL,
        LOG_TYPE_MAX,
    };

    struct LogData
    {
    public:
        static const int MAX_LOG_SIZE = 2 * 1000;
    public:
        int    channel_id_;
        int    log_type_;
        int    filter_level_;
        int    filter_cls_;
        long long timestamp_;        //create timestamp
        int precise_; //create time millionsecond suffix
        unsigned int thread_;
        int content_len_;
        char content_[MAX_LOG_SIZE]; //content
    };


    enum DeviceOutType
    {
        DEVICE_OUT_NULL,
        DEVICE_OUT_SCREEN,
        DEVICE_OUT_FILE,
        DEVICE_OUT_UDP,
    };


    enum DeviceConfigEnum
    {
        DEVICE_CFG_ABLE, 
        DEVICE_CFG_FILTER_LEVEL,  
        DEVICE_CFG_VALID_CLS_BEGIN,  
        DEVICE_CFG_VALID_CLS_COUNT, 
        DEVICE_CFG_FILE_LIMIT_SIZE, 
        DEVICE_CFG_FILE_ROLLBACK, 
        DEVICE_CFG_UDP_IP,
        DEVICE_CFG_UDP_PORT,
        DEVICE_CFG_MAX_ID
    };

    enum DeviceLogEnum
    {
        DEVICE_LOG_CUR_FILE_SIZE, 
        DEVICE_LOG_CUR_FILE_CREATE_TIMESTAMP,  
        DEVICE_LOG_CUR_FILE_CREATE_DAY, 
        DEVICE_LOG_LAST_TRY_CREATE_TIMESTAMP, 
        DEVICE_LOG_TOTAL_WRITE_LINE,  
        DEVICE_LOG_TOTAL_WRITE_BYTE,  
        DEVICE_LOG_MAX_ID
    };




    struct Device
    {
    public:
        static const int MAX_PATH_SYS_LEN = 255;
        static const int MAX_PATH_LEN = 200;
        static const int MAX_NAME_LEN = 50;
        static const int MAX_ROLLBACK_LEN = 4;
        static const int MAX_ROLLBACK_PATHS = 5;
        static_assert(MAX_PATH_LEN + MAX_NAME_LEN + MAX_ROLLBACK_LEN < MAX_PATH_SYS_LEN, "");
        static_assert(MAX_ROLLBACK_PATHS < 10, "");
        using ConfigFields = std::array<AnyVal, DEVICE_CFG_MAX_ID>;
        using LogFields = std::array<AnyVal, DEVICE_LOG_MAX_ID>;

    public:
        int device_id_;
        unsigned int out_type_;
        char out_file_[MAX_NAME_LEN];
        char out_path_[MAX_PATH_LEN];
        ConfigFields config_fields_;
        LogFields log_fields_;
    };

    struct LogQueue
    {
    public:
        using LogDataPtr = LogData *;
        using SizeType = unsigned int;
        static const int MAX_LOG_QUEUE_LEN = 80000;
    public:
        char chunk_1_[CHUNK_SIZE];
        long long log_count_;
        char chunk_2_[CHUNK_SIZE];
        volatile SizeType write_count_;
        char chunk_3_[CHUNK_SIZE];
        volatile SizeType read_count_;
        char chunk_4_[CHUNK_SIZE];
        LogDataPtr log_queue_[MAX_LOG_QUEUE_LEN];
    };
   
    enum ChannelType
    {
        CHANNEL_MULTI,
        CHANNEL_RING,
        CHANNEL_SYNC,
    };

    enum ChannelConfigEnum
    {
        CHANNEL_CFG_FILTER_LEVEL, 
        CHANNEL_CFG_VALID_CLS_BEGIN,  
        CHANNEL_CFG_VALID_CLS_COUNT, 
        CHANNEL_CFG_MAX_ID
    };

    enum ChannelLogEnum
    {
        CHANNEL_LOG_WRITE_COUNT,
        CHANNEL_LOG_ALLOC_CALL,
        CHANNEL_LOG_ALLOC_REAL,
        CHANNEL_LOG_ALLOC_CACHE,
        CHANNEL_LOG_FREE_CALL,
        CHANNEL_LOG_FREE_REAL,
        CHANNEL_LOG_FREE_CACHE,
        CHANNEL_LOG_MAX_ID
    };


    struct Channel
    {
    public:
        using ConfigFields = std::array<AnyVal, CHANNEL_CFG_MAX_ID>;
        using LogFields = std::array<AnyVal, CHANNEL_LOG_MAX_ID>;
        static const int MAX_DEVICE_SIZE = 20;
        static const int MAX_FREE_POOL_SIZE = 4000;
        static_assert(MAX_FREE_POOL_SIZE <= LogQueue::MAX_LOG_QUEUE_LEN, "");
    public:
        char chunk_1_[CHUNK_SIZE];

        int  channel_id_;
        int channel_type_;
        bool actived_;
        time_t yaml_mtime_;
        time_t last_hot_check_;

        char chunk_2_[CHUNK_SIZE];
        int write_red_;
        LogQueue red_black_queue_[2];
        LogQueue log_pool_;
        char chunk_3_[CHUNK_SIZE];

        int chunk_;
        int device_size_;
        Device devices_[MAX_DEVICE_SIZE];
        ConfigFields config_fields_;
        LogFields log_fields_;
    };

    struct SyncGroup
    {
        char chunk_1_[CHUNK_SIZE];
        std::thread log_thread_;
        char chunk_2_[CHUNK_SIZE];
        std::mutex write_lock_;
        char chunk_3_[CHUNK_SIZE];
        std::mutex pool_lock_;
    };

    class Logger
    {
    public:
        static const int MAX_CHANNEL_SIZE = 6;
        using Channels = std::array<Channel, MAX_CHANNEL_SIZE>;
        using SyncGroups = std::array<SyncGroup, MAX_CHANNEL_SIZE>;
        using Locks = std::array<std::mutex, MAX_CHANNEL_SIZE>;
        using FileHandles = std::array<FileHandler, MAX_CHANNEL_SIZE* Channel::MAX_DEVICE_SIZE>;
        using UDPHandles = std::array<UDPHandler, MAX_CHANNEL_SIZE* Channel::MAX_DEVICE_SIZE>;
    public:
        using ProcDevice = std::function<void(Logger&, int, int, LogData& log)>;
        using AllocLogData = std::function<LogData* ()>;
        using FreeLogData = std::function<void(LogData*)>;
    public:
        std::atomic_int last_error_;
        
        bool hot_update_;
        std::string yaml_path_;

        bool waiting_close_;

        int channel_size_;
        Channels channels_;
        SyncGroups syncs_;
        SyncGroup screen_;
        FileHandles file_handles_;
        UDPHandles udp_handles_;
    public:
        AllocLogData sys_alloc_;
        FreeLogData sys_free_;
    };


#define FN_MIN(x, y) ((y) < (x) ? (y) :(x))
#define FN_MAX(x, y) ((x) < (y) ? (y) :(x))

}


#endif
