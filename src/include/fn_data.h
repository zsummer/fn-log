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
#ifndef _FN_LOG_DATA_H_
#define _FN_LOG_DATA_H_

#include "fn_file.h"

#ifndef FN_LOG_MAX_DEVICE_SIZE
#define FN_LOG_MAX_DEVICE_SIZE 20
#endif

#ifndef FN_LOG_MAX_CHANNEL_SIZE
#define FN_LOG_MAX_CHANNEL_SIZE 2
#endif

#ifndef FN_LOG_MAX_LOG_SIZE
#define FN_LOG_MAX_LOG_SIZE 10000
#endif

#ifndef FN_LOG_MAX_LOG_QUEUE_SIZE //the size need big than push log thread count
#define FN_LOG_MAX_LOG_QUEUE_SIZE 1000
#endif


#ifndef FN_LOG_HOTUPDATE_INTERVEL
#define FN_LOG_HOTUPDATE_INTERVEL 5
#endif


//#define FN_LOG_USING_ATOM_CFG

namespace FNLog
{
    enum LogPriority
    {
        PRIORITY_TRACE = 0,
        PRIORITY_DEBUG,
        PRIORITY_INFO,
        PRIORITY_WARN,
        PRIORITY_ERROR,
        PRIORITY_ALARM,
        PRIORITY_FATAL,
        PRIORITY_MAX
    };
    enum LogPrefix
    {
        LOG_PREFIX_NULL = 0x0,
        LOG_PREFIX_TIMESTAMP = 0x1,
        LOG_PREFIX_PRIORITY = 0x2,
        LOG_PREFIX_THREAD = 0x4,
        LOG_PREFIX_NAME = 0x8,
        LOG_PREFIX_DESC = 0x10,
        LOG_PREFIX_FILE = 0x20,
        LOG_PREFIX_FUNCTION = 0x40,
        LOG_PREFIX_ALL = 0xff,
        //LOG_PREFIX_DEFAULT = LOG_PREFIX_ALL,
        LOG_PREFIX_DEFAULT = LOG_PREFIX_TIMESTAMP | LOG_PREFIX_PRIORITY | LOG_PREFIX_FILE | LOG_PREFIX_FUNCTION,
    };


    enum LogType
    {
        LOG_TYPE_NULL,
        LOG_TYPE_MAX,
    };

    enum LogState
    {
        MARK_INVALID,
        MARK_HOLD,
        MARK_READY
    };

    struct LogData
    {
    public:
        static const int LOG_SIZE = FN_LOG_MAX_LOG_SIZE;
    public:
        std::atomic_int    data_mark_; //0 invalid, 1 hold, 2 ready
        int     channel_id_;
        int     priority_;
        int     category_; 
        long long     identify_;
        int     code_line_;
        int     code_func_len_;
        int     code_file_len_;
        const char* code_func_;
        const char* code_file_;
        long long timestamp_;        //create timestamp
        int precise_; //create time millionsecond suffix
        unsigned int thread_;
        int prefix_len_;
        int content_len_;
        char content_[LOG_SIZE]; //content
    };


    enum DeviceOutType
    {
        DEVICE_OUT_NULL,
        DEVICE_OUT_SCREEN,
        DEVICE_OUT_FILE,
        DEVICE_OUT_UDP,
        DEVICE_OUT_VIRTUAL,
        DEVICE_OUT_EMPTY,
    };


    enum DeviceConfigEnum
    {
        DEVICE_CFG_ABLE, 
        DEVICE_CFG_PRIORITY,  
        DEVICE_CFG_CATEGORY,  
        DEVICE_CFG_CATEGORY_EXTEND, 
        DEVICE_CFG_CATEGORY_MASK,
        DEVICE_CFG_IDENTIFY,
        DEVICE_CFG_IDENTIFY_EXTEND,
        DEVICE_CFG_IDENTIFY_MASK,
        DEVICE_CFG_FILE_LIMIT_SIZE, 
        DEVICE_CFG_FILE_ROLLBACK,
        DEVICE_CFG_FILE_ROLLDAILY,
        DEVICE_CFG_FILE_ROLLHOURLY,
        DEVICE_CFG_FILE_STUFF_UP,
        DEVICE_CFG_UDP_IP,
        DEVICE_CFG_UDP_PORT,
        DEVICE_CFG_MAX_ID
    };

    enum DeviceLogEnum
    {
        DEVICE_LOG_CUR_FILE_SIZE, 
        DEVICE_LOG_CUR_FILE_CREATE_TIMESTAMP,  
        DEVICE_LOG_CUR_FILE_CREATE_DAY,
        DEVICE_LOG_CUR_FILE_CREATE_HOUR,
        DEVICE_LOG_LAST_TRY_CREATE_TIMESTAMP,
        DEVICE_LOG_LAST_TRY_CREATE_ERROR,
        DEVICE_LOG_LAST_TRY_CREATE_CNT,
        DEVICE_LOG_PRIORITY, //== PRIORITY_TRACE
        DEVICE_LOG_PRIORITY_MAX = DEVICE_LOG_PRIORITY + PRIORITY_MAX,
        DEVICE_LOG_TOTAL_WRITE_LINE,
        DEVICE_LOG_TOTAL_WRITE_BYTE,  
        DEVICE_LOG_MAX_ID
    };




    struct Device
    {
    public:
        static const int MAX_PATH_SYS_LEN = 255;
        static const int MAX_PATH_LEN = 200;
        static const int MAX_LOGGER_NAME_LEN = 50;
        static const int MAX_ROLLBACK_LEN = 4;
        static const int MAX_ROLLBACK_PATHS = 5;
        static_assert(MAX_PATH_LEN + MAX_LOGGER_NAME_LEN + MAX_ROLLBACK_LEN < MAX_PATH_SYS_LEN, "");
        static_assert(LogData::LOG_SIZE > MAX_PATH_SYS_LEN*2, "unsafe size"); // promise format length: date, time, source file path, function length.
        static_assert(MAX_ROLLBACK_PATHS < 10, "");
#ifdef FN_LOG_USING_ATOM_CFG
        using ConfigFields = std::array<std::atomic_llong, DEVICE_CFG_MAX_ID>;
#else
        using ConfigFields = long long[DEVICE_CFG_MAX_ID];
#endif // FN_LOG_USING_ATOM_CFG
        using LogFields = std::array<std::atomic_llong, DEVICE_LOG_MAX_ID>;

    public:
        int device_id_;
        unsigned int out_type_;
        char out_file_[MAX_LOGGER_NAME_LEN];
        char out_path_[MAX_PATH_LEN];
        ConfigFields config_fields_;
        LogFields log_fields_;
    };

   
    enum ChannelType
    {
        CHANNEL_ASYNC,
        CHANNEL_SYNC,
    };

    enum ChannelConfigEnum
    {
        CHANNEL_CFG_PRIORITY, 
        CHANNEL_CFG_CATEGORY,  
        CHANNEL_CFG_CATEGORY_EXTEND, 
        CHANNEL_CFG_CATEGORY_MASK,
        CHANNEL_CFG_IDENTIFY,
        CHANNEL_CFG_IDENTIFY_EXTEND,
        CHANNEL_CFG_IDENTIFY_MASK,
        CHANNEL_CFG_MAX_ID
    };


    enum ChannelLogEnum
    {
        CHANNEL_LOG_WAIT_COUNT,
        CHANNEL_LOG_HOLD,
        CHANNEL_LOG_PUSH,
        CHANNEL_LOG_PRIORITY, //== PRIORITY_TRACE
        CHANNEL_LOG_PRIORITY_MAX = CHANNEL_LOG_PRIORITY + PRIORITY_MAX,

        CHANNEL_LOG_BOUND = CHANNEL_LOG_PRIORITY_MAX + 8, //ull*8 

        CHANNEL_LOG_PROCESSED,
        CHANNEL_LOG_PROCESSED_BYTES,
        CHANNEL_LOG_MAX_PROC_QUE_SIZE,
        CHANNEL_LOG_MAX_DELAY_TIME_S, //second 

        CHANNEL_LOG_MAX_ID
    };

    enum ChannelState
    {
        CHANNEL_STATE_NULL = 0,
        CHANNEL_STATE_RUNNING,
        CHANNEL_STATE_WAITING_FINISH,
        CHANNEL_STATE_FINISH,
    };

    struct RingBuffer
    {
    public:
        static const int BUFFER_LEN = FN_LOG_MAX_LOG_QUEUE_SIZE;
        static_assert(BUFFER_LEN > 10, "ring queue size too little");
    public:
        char chunk_1_[CHUNK_SIZE];
        std::atomic_int write_idx_;
        char chunk_2_[CHUNK_SIZE];
        std::atomic_int hold_idx_;
        char chunk_3_[CHUNK_SIZE];
        std::atomic_int read_idx_;
        char chunk_4_[CHUNK_SIZE];
        std::atomic_int proc_idx_;
        char chunk_5_[CHUNK_SIZE];
        LogData buffer_[BUFFER_LEN];
    };

    struct Channel
    {
    public:
#ifdef FN_LOG_USING_ATOM_CFG
        using ConfigFields = std::array<std::atomic_llong, CHANNEL_CFG_MAX_ID>;
#else
        using ConfigFields = long long[CHANNEL_CFG_MAX_ID];
#endif // FN_LOG_USING_ATOM_CFG

        using LogFields = std::array<std::atomic_llong, CHANNEL_LOG_MAX_ID>;
        static const int MAX_DEVICE_SIZE = FN_LOG_MAX_DEVICE_SIZE;


    public:
        char chunk_1_[CHUNK_SIZE];

        int  channel_id_;
        int  channel_type_;
        unsigned int channel_state_;
        time_t yaml_mtime_;
        time_t last_hot_check_;

        int chunk_;
        int virtual_device_id_;
        int device_size_;
        Device devices_[MAX_DEVICE_SIZE];
        ConfigFields config_fields_;
        LogFields log_fields_;
    };


    enum LoggerState
    {
        LOGGER_STATE_UNINIT = 0,
        LOGGER_STATE_INITING,
        LOGGER_STATE_RUNNING,
        LOGGER_STATE_CLOSING,
    };
    
    struct SHMLogger
    {
        static const int MAX_CHANNEL_SIZE = FN_LOG_MAX_CHANNEL_SIZE;
        using Channels = std::array<Channel, MAX_CHANNEL_SIZE>;
        using RingBuffers = std::array<RingBuffer, MAX_CHANNEL_SIZE>;
        int shm_id_;
        int shm_size_; 
        int channel_size_;
        Channels channels_;
        RingBuffers ring_buffers_;
    };

    template<class Mutex>
    class AutoGuard
    {
    public:
        using mutex_type = Mutex;
        inline explicit AutoGuard(Mutex& mtx, bool noop = false) : mutex_(mtx), noop_(noop) 
        {
            if (!noop_)
            {
                mutex_.lock();
            }
        }

        inline ~AutoGuard() noexcept
        { 
            if (!noop_)
            {
                mutex_.unlock();
            }
        }
        AutoGuard(const AutoGuard&) = delete;
        AutoGuard& operator=(const AutoGuard&) = delete;
    private:
        Mutex& mutex_;
        bool noop_;
    };

    class Logger
    {
    public:
        static const int MAX_CHANNEL_SIZE = SHMLogger::MAX_CHANNEL_SIZE;
        static const int HOTUPDATE_INTERVEL = FN_LOG_HOTUPDATE_INTERVEL;
        static const int MAX_LOGGER_DESC_LEN = 50;
        static const int MAX_LOGGER_NAME_LEN = 250;
        using ReadLocks = std::array<std::mutex, MAX_CHANNEL_SIZE>;
        using ReadGuard = AutoGuard<std::mutex>;

        using AsyncThreads = std::array<std::thread, MAX_CHANNEL_SIZE>;
        using FileHandles = std::array<FileHandler, MAX_CHANNEL_SIZE* Channel::MAX_DEVICE_SIZE>;
        using UDPHandles = std::array<UDPHandler, MAX_CHANNEL_SIZE* Channel::MAX_DEVICE_SIZE>;

    public:
        using StateLock = std::recursive_mutex;
        using StateLockGuard = AutoGuard<StateLock>;

        using ScreenLock = std::mutex;
        using ScreenLockGuard = AutoGuard<ScreenLock>;


    public:
        Logger();
        ~Logger();
        bool hot_update_;
        std::string yaml_path_;
        unsigned int logger_state_;
        StateLock state_lock_;
        char desc_[MAX_LOGGER_DESC_LEN];
        int desc_len_;
        char name_[MAX_LOGGER_NAME_LEN];
        int name_len_;

        long long shm_key_;
        SHMLogger* shm_;

        ReadLocks read_locks_;
        AsyncThreads async_threads;
        ScreenLock screen_lock_;
        FileHandles file_handles_;
        UDPHandles udp_handles_;
    };



    template<class V>
    inline V FN_MIN(V x, V y) 
    {
        return y < x ? y : x;
    }
    template<class V>
    inline V FN_MAX(V x, V y)
    {
        return x < y ? y : x;
    }


#ifdef FN_LOG_USING_ATOM_CFG
    #define AtomicLoadC(m, eid) m.config_fields_[eid].load(std::memory_order_relaxed)
#else
#define AtomicLoadC(m, eid) m.config_fields_[eid]
#endif // FN_LOG_USING_ATOM_CFG


    template <class M>
    inline long long AtomicLoadL(M& m, unsigned eid)
    {
        return m.log_fields_[eid].load(std::memory_order_relaxed);
    }

    template <class M>
    inline void AtomicAddL(M& m, unsigned eid)
    {
        m.log_fields_[eid].fetch_add(1, std::memory_order_relaxed);
    }
    template <class M>
    inline void AtomicAddLV(M& m, unsigned eid, long long v)
    {
        m.log_fields_[eid].fetch_add(v, std::memory_order_relaxed);
    }

    template <class M>
    inline void AtomicStoreL(M& m, unsigned eid, long long v)
    {
        m.log_fields_[eid].store(v, std::memory_order_relaxed);
    }


    enum ErrNo
    {
        E_SUCCESS = 0,

        E_LOGGER_IN_USE,
        E_LOGGER_NOT_RUNNING,

        E_INVALID_CONFIG_PATH,

        E_INVALID_CHANNEL_SIZE,
        E_INVALID_CHANNEL_SYNC,
        E_INVALID_CHANNEL_STATE,
        E_CHANNEL_THREAD_FAILED,
        E_CHANNEL_NOT_SEQUENCE,


        E_INVALID_DEVICE_SIZE,
        E_DEVICE_NOT_SEQUENCE,





        E_SHMGET_PROBE_ERROR,
        E_SHMGET_CREATE_ERROR,
        E_SHMAT_ERROR,
        E_SHM_VERSION_WRONG,

        E_VERSION_MISMATCH,


        E_DISABLE_HOTUPDATE,
        E_NO_CONFIG_PATH,
        E_CONFIG_NO_CHANGE,
        E_OUT_RINGBUFFER,

        E_BASE_ERRNO_MAX

    };


}


#endif
