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
#ifndef _FN_LOG_FILE_H_
#define _FN_LOG_FILE_H_

#include <cstddef>
#include <cstring>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <array>
#include <string>
#include <algorithm>
#include <array>
#include <mutex>
#include <thread>
#include <functional>
#include <regex>
#include <atomic>
#include <cmath>
#include <cfloat>
#include <list>
#include <deque>
#include <queue>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <memory>
#include <atomic>

#ifdef WIN32
#ifndef KEEP_INPUT_QUICK_EDIT
#define KEEP_INPUT_QUICK_EDIT false
#endif

#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <Windows.h>
#include <io.h>
#include <shlwapi.h>
#include <process.h>
#pragma comment(lib, "shlwapi")
#pragma comment(lib, "User32.lib")
#pragma comment(lib,"ws2_32.lib")
#pragma warning(disable:4996)

#else
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/syscall.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#
#endif


#ifdef __APPLE__
#include "TargetConditionals.h"
#include <dispatch/dispatch.h>
#if !TARGET_OS_IPHONE
#define NFLOG_HAVE_LIBPROC
#include <libproc.h>
#endif
#endif


namespace FNLog
{
    static const int CHUNK_SIZE = 128;
    class FileHandler
    {
    public:
        inline FileHandler();
        inline ~FileHandler();
        inline bool is_open();
        inline long open(const char* path, const char* mod, struct stat& file_stat);
        inline void close();
        inline void write(const char* data, size_t len);
        inline void flush();

        inline std::string read_line();
        inline std::string read_content();

        static inline bool is_dir(const std::string & path);
        static inline bool is_file(const std::string & path);
        static inline bool create_dir(const std::string& path);
        static inline std::string process_id();
        static inline std::string process_name();
        static inline bool remove_file(const std::string& path);
        static inline struct tm time_to_tm(time_t t);

        static inline bool rollback(const std::string& path, int depth, int max_depth);
    public:
        char chunk_1_[128];
        FILE* file_;
    };





    long FileHandler::open(const char* path, const char* mod, struct stat& file_stat)
    {
        if (file_ != nullptr)
        {
            fclose(file_);
            file_ = nullptr;
        }
        file_ = fopen(path, mod);
        if (file_)
        {
            if (fstat(fileno(file_), &file_stat) != 0)
            {
                fclose(file_);
                file_ = nullptr;
                return -1;
            }
            return file_stat.st_size;
        }
        return -2;
    }
    void FileHandler::close()
    {
        if (file_ != nullptr)
        {
#if !defined(__APPLE__) && !defined(WIN32) 
            if (file_ != nullptr)
            {
                int fd = fileno(file_);
                fsync(fd);
                posix_fadvise(fd, 0, 0, POSIX_FADV_DONTNEED);
                fsync(fd);
            }
#endif
            fclose(file_);
            file_ = nullptr;
        }
    }

    FileHandler::FileHandler()
    {
        file_ = nullptr;
    }
    FileHandler::~FileHandler()
    {
        close();
    }

    bool FileHandler::is_open()
    {
        return file_ != nullptr;
    }

    void FileHandler::write(const char* data, size_t len)
    {
        if (file_ && len > 0)
        {
            if (fwrite(data, 1, len, file_) != len)
            {
                close();
            }
        }
    }
    void FileHandler::flush()
    {
        if (file_)
        {
            fflush(file_);
        }
    }

    std::string FileHandler::read_line()
    {
        char buf[500] = { 0 };
        if (file_ && fgets(buf, 500, file_) != nullptr)
        {
            return std::string(buf);
        }
        return std::string();
    }
    std::string FileHandler::read_content()
    {
        std::string content;

        if (!file_)
        {
            return content;
        }
        char buf[BUFSIZ];
        size_t ret = 0;
        do
        {
            ret = fread(buf, sizeof(char), BUFSIZ, file_);
            content.append(buf, ret);
        } while (ret == BUFSIZ);

        return content;
    }

    bool FileHandler::is_dir(const std::string& path)
    {
#ifdef WIN32
        return PathIsDirectoryA(path.c_str()) ? true : false;
#else
        DIR* pdir = opendir(path.c_str());
        if (pdir == nullptr)
        {
            return false;
        }
        else
        {
            closedir(pdir);
            pdir = nullptr;
            return true;
        }
#endif
    }

    bool FileHandler::is_file(const std::string& path)
    {
#ifdef WIN32
        return ::_access(path.c_str(), 0) == 0;
#else
        return ::access(path.c_str(), F_OK) == 0;
#endif
    }

    bool FileHandler::create_dir(const std::string& path)
    {
        if (path.length() == 0)
        {
            return true;
        }
        std::string sub;

        std::string::size_type pos = path.find('/');
        while (pos != std::string::npos)
        {
            std::string cur = path.substr(0, pos - 0);
            if (cur.length() > 0 && !is_dir(cur))
            {
                bool ret = false;
#ifdef WIN32
                ret = CreateDirectoryA(cur.c_str(), nullptr) ? true : false;
#else
                ret = (mkdir(cur.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == 0);
#endif
                if (!ret)
                {
                    return false;
                }
            }
            pos = path.find('/', pos + 1);
        }

        return true;
    }


    std::string FileHandler::process_id()
    {
        std::string pid = "0";
        char buf[260] = { 0 };
#ifdef WIN32
        DWORD winPID = GetCurrentProcessId();
        sprintf(buf, "%06u", winPID);
        pid = buf;
#else
        sprintf(buf, "%06d", getpid());
        pid = buf;
#endif
        return pid;
    }

    std::string FileHandler::process_name()
    {
        std::string name = "process";
        char buf[260] = { 0 };
#ifdef WIN32
        if (GetModuleFileNameA(nullptr, buf, 259) > 0)
        {
            name = buf;
        }
        std::string::size_type pos = name.rfind("\\");
        if (pos != std::string::npos)
        {
            name = name.substr(pos + 1, std::string::npos);
        }
        pos = name.rfind(".");
        if (pos != std::string::npos)
        {
            name = name.substr(0, pos - 0);
        }

#elif defined(__APPLE__)
        proc_name(getpid(), buf, 260);
        name = buf;
        return name;;
#else
        sprintf(buf, "/proc/%d/cmdline", (int)getpid());
        FileHandler i;
	struct stat file_stat;
        i.open(buf, "rb", file_stat);
        if (!i.is_open())
        {
            return name;
        }
        name = i.read_line();
        i.close();

        std::string::size_type pos = name.rfind("/");
        if (pos != std::string::npos)
        {
            name = name.substr(pos + 1, std::string::npos);
        }
#endif

        return name;
    }

    bool FileHandler::remove_file(const std::string & path)
    {
        return ::remove(path.c_str()) == 0;
    }

    struct tm FileHandler::time_to_tm(time_t t)
    {
#ifdef WIN32
#if _MSC_VER < 1400 //VS2003
        return *localtime(&t);
#else //vs2005->vs2013->
        struct tm tt = { 0 };
        localtime_s(&tt, &t);
        return tt;
#endif
#else //linux
        struct tm tt = { 0 };
        localtime_r(&t, &tt);
        return tt;
#endif
    }



    bool FileHandler::rollback(const std::string& path, int depth, int max_depth)
    {
        if (!is_file(path))
        {
            return true;
        }
        if (depth > max_depth)
        {
            return remove_file(path);
        }
        std::string next_path = path;
        size_t pos = path.find_last_not_of("0123456789");
        if (pos != std::string::npos && path.at(pos) == '.')
        {
            next_path = path.substr(0, pos - 0);
        }
        next_path += ".";
        next_path += std::to_string(depth);
        rollback(next_path, depth + 1, max_depth);
        int ret = ::rename(path.c_str(), next_path.c_str());
        (void)ret;
        return true;
    }

    inline int short_path(const char* path, int len)
    {
        int count = 3;
        if (path == nullptr || len <= 0)
        {
            return 0;
        }
        const char* last = path + len;
        while (last-- != path)
        {
            if (*last == '/' || *last == '\\')
            {
                if (--count <= 0)
                {
                    return (int)(last - path + 1);
                }
            }
        }
        return 0;
    }

}





class UDPHandler
{
public:
#ifndef WIN32
    using SOCKET = int;
    static const int INVALID_SOCKET = -1;
#endif 

public:
    UDPHandler()
    {
        handler_ = INVALID_SOCKET;
    }
    ~UDPHandler()
    {
        if (handler_ != INVALID_SOCKET)
        {
            close();
        }
    }

    bool is_open()
    {
        return handler_ != INVALID_SOCKET;
    }

    void open()
    {
        handler_ = socket(AF_INET, SOCK_DGRAM, 0);
    }

    void close()
    {
        if (handler_ != INVALID_SOCKET)
        {
#ifndef WIN32
            ::close(handler_);
#else
            closesocket(handler_);
#endif 
            handler_ = INVALID_SOCKET;
        }
    }

    void write(unsigned int ip, unsigned short port, const char* data, int len)
    {
        if (handler_ == INVALID_SOCKET)
        {
            return;
        }

        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = port;
        addr.sin_addr.s_addr = ip;
        int ret = sendto(handler_, data, len, 0, (struct sockaddr*) &addr, sizeof(addr));
        (void)ret;
    }
 
public:
    char chunk_1_[128];
    SOCKET handler_;
};


#endif
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


#ifndef FN_LOG_MAX_CHANNEL_SIZE
#define FN_LOG_MAX_CHANNEL_SIZE 2
#endif

#ifndef FN_LOG_MAX_LOG_SIZE
#define FN_LOG_MAX_LOG_SIZE 1000
#endif

#ifndef FN_LOG_MAX_LOG_QUEUE_SIZE //the size need big than push log thread count
#define FN_LOG_MAX_LOG_QUEUE_SIZE 10000
#endif


#ifndef FN_LOG_HOTUPDATE_INTERVEL
#define FN_LOG_HOTUPDATE_INTERVEL 5
#endif

#ifndef FN_LOG_USE_SHM
#define FN_LOG_USE_SHM 0
#endif 

#ifndef FN_LOG_SHM_KEY
#define FN_LOG_SHM_KEY 0x9110
#endif 


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
        LOG_PREFIX_FILE = 0x8,
        LOG_PREFIX_FUNCTION = 0x10,
        LOG_PREFIX_ALL = 0xff
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
        int    channel_id_;
        int    priority_;
        int    category_;
        long long timestamp_;        //create timestamp
        int precise_; //create time millionsecond suffix
        unsigned int thread_;
        int content_len_;
        char content_[LOG_SIZE]; //content
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
        DEVICE_CFG_PRIORITY,  
        DEVICE_CFG_CATEGORY,  
        DEVICE_CFG_CATEGORY_EXTEND, 
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
        DEVICE_LOG_LAST_TRY_CREATE_ERROR,
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
        static_assert(LogData::LOG_SIZE > MAX_PATH_SYS_LEN*2, "unsafe size"); // promise format length: date, time, source file path, function length.
        static_assert(MAX_ROLLBACK_PATHS < 10, "");
        using ConfigFields = std::array<std::atomic_llong, DEVICE_CFG_MAX_ID>;
        using LogFields = std::array<std::atomic_llong, DEVICE_LOG_MAX_ID>;

    public:
        int device_id_;
        unsigned int out_type_;
        char out_file_[MAX_NAME_LEN];
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
        CHANNEL_CFG_MAX_ID
    };


    enum ChannelLogEnum
    {
        CHANNEL_LOG_HOLD,
        CHANNEL_LOG_PUSH,
        CHANNEL_LOG_PROCESSED = CHANNEL_LOG_PUSH + 8,
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
        using ConfigFields = std::array<std::atomic_llong, CHANNEL_CFG_MAX_ID>;
        using LogFields = std::array<std::atomic_llong, CHANNEL_LOG_MAX_ID>;
        static const int MAX_DEVICE_SIZE = 20;


    public:
        char chunk_1_[CHUNK_SIZE];

        int  channel_id_;
        int  channel_type_;
        unsigned int channel_state_;
        time_t yaml_mtime_;
        time_t last_hot_check_;

        int chunk_;
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
        StateLock state_lock;

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

    template <class M>
    inline long long AtomicLoadC(M& m, unsigned eid)
    {
        return m.config_fields_[eid].load(std::memory_order_relaxed);
    }

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
}


#endif
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
#ifndef _FN_LOG_PARSE_H_
#define _FN_LOG_PARSE_H_



namespace FNLog
{
    enum ParseErrorCode
    {
        PEC_NONE, 
        PEC_ERROR,
        PEC_ILLEGAL_CHARACTER,
        PEC_ILLEGAL_KEY,
        PEC_NOT_CLOSURE,
        PEC_ILLEGAL_ADDR_IP,
        PEC_ILLEGAL_ADDR_PORT,

        PEC_UNDEFINED_DEVICE_KEY,
        PEC_UNDEFINED_DEVICE_TYPE,
        PEC_UNDEFINED_CHANNEL_KEY,
        PEC_UNDEFINED_GLOBAL_KEY,

        PEC_DEVICE_NOT_ARRAY,
        PEC_DEVICE_INDEX_OUT_MAX,
        PEC_DEVICE_INDEX_NOT_SEQUENCE,
 

        PEC_CHANNEL_NOT_ARRAY,
        PEC_CHANNEL_INDEX_OUT_MAX,
        PEC_CHANNEL_INDEX_NOT_SEQUENCE,
        PEC_NO_ANY_CHANNEL,
    };

    enum LineType
    {
        LINE_NULL,
        LINE_ARRAY,
        LINE_BLANK,
        LINE_EOF,
    };
    enum BlockType
    {
        BLOCK_BLANK,
        BLOCK_PRE_KEY,
        BLOCK_KEY,
        BLOCK_PRE_SEP,
        BLOCK_PRE_VAL,
        BLOCK_VAL,
        BLOCK_CLEAN,
    };
    enum ReseveKey
    {
        RK_NULL,
        RK_CHANNEL,
        RK_DEVICE,
        RK_SYNC,
        RK_DISABLE,
        RK_HOT_UPDATE,
        RK_PRIORITY,
        RK_CATEGORY,
        RK_CATEGORY_EXTEND,
        RK_OUT_TYPE,
        RK_FILE,
        RK_PATH,
        RK_LIMIT_SIZE,
        RK_ROLLBACK,
        RK_UDP_ADDR,
    };

#if __GNUG__ && __GNUC__ >= 5
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess"
#endif


    inline ReseveKey ParseReserve(const char* begin, const char* end)
    {
        if (end - begin < 2)
        {
            return RK_NULL;
        }
        switch (*begin)
        {
        case 'c':
            if (*(begin + 1) == 'h')
            {
                return RK_CHANNEL;
            }
            else if (*(begin + 1) == 'a')
            {
                if (end - begin > (int)sizeof("category") - 1)
                {
                    return RK_CATEGORY_EXTEND;
                }
                else
                {
                    return RK_CATEGORY;
                }
            }
            break;
        case 'd':
            if (*(begin+1) == 'e')
            {
                return RK_DEVICE;
            }
            else if (*(begin + 1) == 'i')
            {
                return RK_DISABLE;
            }
            break;
        case  'f':
            return RK_FILE;
        case 'h':
            return RK_HOT_UPDATE;
        case 'l':
            return RK_LIMIT_SIZE;
        case 'p':
            if (*(begin + 1) == 'r')
            {
                return RK_PRIORITY;
            }
            else if (*(begin + 1) == 'a')
            {
                return RK_PATH;
            }
            break;
        case 'r':
            return RK_ROLLBACK;
        case 'o':
            return RK_OUT_TYPE;
        case 's':
            return RK_SYNC;
        case 'u':
            return RK_UDP_ADDR;
        default:
            break;
        }
        return RK_NULL;
    }

    inline LogPriority ParsePriority(const char* begin, const char* end)
    {
        if (end <= begin)
        {
            return PRIORITY_TRACE;
        }
        switch (*begin)
        {
        case 't':case 'T':
        case 'n':case 'N':
            return PRIORITY_TRACE;
        case 'd':case 'D':
            return PRIORITY_DEBUG;
        case 'i':case 'I':
            return PRIORITY_INFO;
        case 'w':case 'W':
            return PRIORITY_WARN;
        case 'e':case 'E':
            return PRIORITY_ERROR;
        case 'a':case 'A':
            return PRIORITY_ALARM;
        case 'f':case 'F':
            return PRIORITY_FATAL;
        }
        return PRIORITY_TRACE;
    }

    inline bool ParseBool(const char* begin, const char* end)
    {
        if (end <= begin)
        {
            return false;
        }
        if (*begin == '0' || *begin == 'f')
        {
            return false;
        }
        return true;
    }

    inline ChannelType ParseChannelType(const char* begin, const char* end)
    {
        if (end <= begin || *begin != 's')
        {
            return CHANNEL_ASYNC;
        }
        return CHANNEL_SYNC;
    }
    
    inline DeviceOutType ParseOutType(const char* begin, const char* end)
    {
        if (end <= begin)
        {
            return DEVICE_OUT_NULL;
        }
        switch (*begin)
        {
        case 'f': case 'F':
            return DEVICE_OUT_FILE;
        case 'n': case 'N':
            return DEVICE_OUT_NULL;
        case 'u': case 'U':
            return DEVICE_OUT_UDP;
        case 's':case 'S':
            return DEVICE_OUT_SCREEN;
        }
        return DEVICE_OUT_NULL;
    }

    inline void ParseAddres(const char* begin, const char* end, std::atomic_llong& ip, std::atomic_llong& port)
    {
        ip = 0;
        port = 0;
        if (end <= begin)
        {
            return;
        }
        const char* ip_begin = begin;
        while ((*ip_begin < '1' || *ip_begin > '9') && ip_begin != end)
        {
            ip_begin++;
        }
        const char* ip_end = ip_begin;
        while (((*ip_end >= '0' && *ip_end <= '9') || *ip_end == '.' ) && ip_end != end)
        {
            ip_end++;
        }
        if (ip_end <= ip_begin)
        {
            return;
        }

        const char* port_begin = ip_end;
        while ((*port_begin < '1' || *port_begin > '9') && port_begin != end)
        {
            port_begin++;
        }
        if (end <= port_begin)
        {
            return;
        }
        std::string str(ip_begin, ip_end - ip_begin);
        ip = inet_addr(str.c_str());
        str.assign(port_begin, end - port_begin);
        port = htons(atoi(str.c_str()));
        return;
    }

    struct Line
    {
        int blank_;
        int line_type_;
        int block_type_;
        int key_;
        const char* key_begin_;
        const char* key_end_;
        const char* val_begin_;
        const char* val_end_;
    };

    struct LexState
    {
        int line_number_;
        const char* first_;
        const char* current_;
        const char* end_;
        Line line_;
        SHMLogger::Channels channels_;
        int channel_size_;
        bool hot_update_;
    };

    inline void InitState(LexState& state)
    {
        //static_assert(std::is_trivial<LexState>::value, "");
        memset(&state, 0, sizeof(state));
    }

    inline int Lex(LexState& ls)
    {
        memset(&ls.line_, 0, sizeof(ls.line_));
        while (true)
        {
            char ch = *ls.current_++;
            if (ls.line_.block_type_ == BLOCK_CLEAN && ch != '\0' && ch != '\r' && ch != '\n')
            {
                continue;
            }

            //preprocess
            if (ls.line_.block_type_ == BLOCK_KEY && (ch < 'a' || ch > 'z') && ch != '_')
            {
                ls.line_.block_type_ = BLOCK_PRE_SEP;
                ls.line_.key_end_ = ls.current_ - 1;
                ls.line_.key_ = ParseReserve(ls.line_.key_begin_, ls.line_.key_end_);
                if (ls.line_.key_ == RK_NULL)
                {
                    return PEC_ILLEGAL_KEY;
                }
            }
            if (ls.line_.block_type_ == BLOCK_VAL)
            {
                switch (ch)
                {
                case '\0': case'\n':case '\r': case '#': case '\"':
                    ls.line_.block_type_ = BLOCK_CLEAN;
                    ls.line_.val_end_ = ls.current_ - 1;
                    break;
                }
            }

            //end of line check
            switch (ch)
            {
            case '\0': case '\n':case '\r': case '#':
                if (ls.line_.block_type_ == BLOCK_BLANK)
                {
                    ls.line_.block_type_ = BLOCK_CLEAN;
                    ls.line_.line_type_ = LINE_BLANK;
                }
                if (ls.line_.block_type_ != BLOCK_CLEAN)
                {
                    return PEC_NOT_CLOSURE;
                }
                break;
            }

            //process
            switch (ch)
            {
            case ' ': case '\f': case '\t': case '\v': case '\"':
                if (ls.line_.block_type_ == BLOCK_BLANK)
                {
                    ls.line_.blank_++;
                    break;
                }
                break;
            case '\n':case '\r':
                ls.line_number_++;
                if ((*ls.current_ == '\r' || *ls.current_ == '\n') && *ls.current_ != ch)
                {
                    ls.current_++; //skip '\n\r' or '\r\n'
                }
                return PEC_NONE;
            case '\0':
                if (ls.line_.line_type_ != LINE_BLANK)
                {
                    ls.current_--;
                    return PEC_NONE;
                }
                else
                {
                    ls.line_.line_type_ = LINE_EOF;
                    return PEC_NONE;
                }
                
            case '-':
                if (ls.line_.block_type_ == BLOCK_BLANK)
                {
                    ls.line_.block_type_ = BLOCK_PRE_KEY;
                    ls.line_.line_type_ = LINE_ARRAY;
                    break;
                }
                else if (ls.line_.block_type_ != BLOCK_VAL)
                {
                    return PEC_ILLEGAL_CHARACTER;
                }
                break;
            case ':':
                if (ls.line_.block_type_ == BLOCK_PRE_SEP)
                {
                    ls.line_.block_type_ = BLOCK_PRE_VAL;
                    break;
                }
                else if (ls.line_.block_type_ != BLOCK_VAL)
                {
                    return PEC_ILLEGAL_CHARACTER;
                }
                break;
            default:
                if ((ch >= 'a' && ch <= 'z')
                    || (ch >= 'A' && ch <= 'Z')
                    || (ch >= '0' && ch <= '9')
                    || ch == '_' || ch == '-' || ch == ':' || ch == '/' || ch == '.' || ch == '$' || ch == '~')
                {
                    switch (ls.line_.block_type_)
                    {
                    case BLOCK_CLEAN: case BLOCK_KEY: case BLOCK_VAL:
                        break;
                    case BLOCK_BLANK: case BLOCK_PRE_KEY:
                        ls.line_.block_type_ = BLOCK_KEY;
                        ls.line_.key_begin_ = ls.current_ - 1;
                        break;
                    case BLOCK_PRE_VAL:
                        ls.line_.block_type_ = BLOCK_VAL;
                        ls.line_.val_begin_ = ls.current_ - 1;
                        break;
                    default:
                        return PEC_ILLEGAL_CHARACTER;
                    }
                    break;
                }
                else if (ls.line_.block_type_ != BLOCK_CLEAN)
                {
                    return PEC_ILLEGAL_CHARACTER;
                }
            }
        }
        return PEC_ERROR;
    }

    inline int ParseDevice(LexState& ls, Device& device, int indent)
    {
        do
        {
            const char* current = ls.current_;
            int ret = Lex(ls);
            if (ret != PEC_NONE)
            {
                ls.current_ = current;
                ls.line_number_--;
                return ret;
            }
            if (ls.line_.line_type_ == LINE_EOF)
            {
                return ret;
            }
            if (ls.line_.line_type_ == LINE_BLANK)
            {
                continue;
            }
            
            if (ls.line_.blank_ <= indent)
            {
                ls.current_ = current;
                ls.line_number_--;
                ls.line_.line_type_ = LINE_BLANK;
                return 0;
            }

            switch (ls.line_.key_)
            {
            case RK_OUT_TYPE:
                device.out_type_ = ParseOutType(ls.line_.val_begin_, ls.line_.val_end_);
                if (device.out_type_ == DEVICE_OUT_NULL)
                {
                    return PEC_UNDEFINED_DEVICE_TYPE;
                }
                break;
            case RK_DISABLE:
                device.config_fields_[DEVICE_CFG_ABLE] = !ParseBool(ls.line_.val_begin_, ls.line_.val_end_); //"disable"
                break;
            case RK_PRIORITY:
                device.config_fields_[DEVICE_CFG_PRIORITY] = ParsePriority(ls.line_.val_begin_, ls.line_.val_end_);
                break;
            case RK_CATEGORY:
                device.config_fields_[DEVICE_CFG_CATEGORY] = atoll(ls.line_.val_begin_);
                break;
            case RK_CATEGORY_EXTEND:
                device.config_fields_[DEVICE_CFG_CATEGORY_EXTEND] = atoll(ls.line_.val_begin_);
                break;
            case RK_LIMIT_SIZE:
                device.config_fields_[DEVICE_CFG_FILE_LIMIT_SIZE] = atoll(ls.line_.val_begin_) * 1000*1000;
                break;
            case RK_ROLLBACK:
                device.config_fields_[DEVICE_CFG_FILE_ROLLBACK] = atoll(ls.line_.val_begin_);
                break;
            case RK_PATH:
                if (ls.line_.val_end_ - ls.line_.val_begin_ < Device::MAX_PATH_LEN - 1
                    && ls.line_.val_end_ - ls.line_.val_begin_ >= 1)
                {
                    memcpy(device.out_path_, ls.line_.val_begin_, ls.line_.val_end_ - ls.line_.val_begin_);
                    device.out_path_[ls.line_.val_end_ - ls.line_.val_begin_] = '\0';
                }
                break;
            case RK_FILE:
                if (ls.line_.val_end_ - ls.line_.val_begin_ < Device::MAX_NAME_LEN - 1
                    && ls.line_.val_end_ - ls.line_.val_begin_ >= 1)
                {
                    memcpy(device.out_file_, ls.line_.val_begin_, ls.line_.val_end_ - ls.line_.val_begin_);
                    device.out_file_[ls.line_.val_end_ - ls.line_.val_begin_] = '\0';
                }
                break;
            case RK_UDP_ADDR:
                ParseAddres(ls.line_.val_begin_, ls.line_.val_end_, device.config_fields_[DEVICE_CFG_UDP_IP], device.config_fields_[DEVICE_CFG_UDP_PORT]);
                if (device.config_fields_[DEVICE_CFG_UDP_IP] == 0)
                {
                    return PEC_ILLEGAL_ADDR_IP;
                }
                if (device.config_fields_[DEVICE_CFG_UDP_PORT] == 0)
                {
                    return PEC_ILLEGAL_ADDR_PORT;
                }
                break;
            default:
                return PEC_UNDEFINED_DEVICE_KEY;
            }
        } while (ls.line_.line_type_ != LINE_EOF);
        return 0;
    }
    inline int ParseChannel(LexState& ls, Channel& channel, int indent)
    {
        do
        {
            const char* current = ls.current_;
            int ret = Lex(ls);
            if (ret != PEC_NONE)
            {
                ls.current_ = current;
                ls.line_number_--;
                return ret;
            }
            if (ls.line_.line_type_ == LINE_EOF)
            {
                return ret;
            }
            if (ls.line_.line_type_ == LINE_BLANK)
            {
                continue;
            }

            if (ls.line_.blank_ <= indent)
            {
                ls.current_ = current;
                ls.line_number_--;
                ls.line_.line_type_ = LINE_BLANK;
                return 0;
            }

            switch (ls.line_.key_)
            {
            case RK_SYNC:
                channel.channel_type_ = ParseChannelType(ls.line_.val_begin_, ls.line_.val_end_);
                break;
            case RK_PRIORITY:
                channel.config_fields_[CHANNEL_CFG_PRIORITY] = ParsePriority(ls.line_.val_begin_, ls.line_.val_end_);
                break;
            case RK_CATEGORY:
                channel.config_fields_[CHANNEL_CFG_CATEGORY] = atoi(ls.line_.val_begin_);
                break;            
            case RK_CATEGORY_EXTEND:
                channel.config_fields_[CHANNEL_CFG_CATEGORY_EXTEND] = atoi(ls.line_.val_begin_);
                break;
            case RK_DEVICE:
                if (ls.line_.line_type_ != LINE_ARRAY)
                {
                    return PEC_DEVICE_NOT_ARRAY;
                }
                else
                {
                    int device_id = atoi(ls.line_.val_begin_);
                    if (channel.device_size_ >= Channel::MAX_DEVICE_SIZE)
                    {
                        return PEC_DEVICE_INDEX_OUT_MAX;
                    }
                    if (device_id != channel.device_size_)
                    {
                        return PEC_DEVICE_INDEX_NOT_SEQUENCE;
                    }
                    Device& device = channel.devices_[channel.device_size_++];
                    memset(&device, 0, sizeof(device));
                    device.device_id_ = device_id;
                    ret = ParseDevice(ls, device, ls.line_.blank_);
                    if (ret != PEC_NONE || ls.line_.line_type_ == LINE_EOF)
                    {
                        return ret;
                    }
                }
                break;
            default:
                return PEC_UNDEFINED_CHANNEL_KEY;
            }

        } while (ls.line_.line_type_ != LINE_EOF);
        return 0;
    }
    inline int ParseLogger(LexState& ls, const std::string& text)
    {
        //UTF8 BOM 
        const char* first = &text[0];
        if (text.size() >= 3)
        {
            if ((unsigned char)text[0] == 0xEF && (unsigned char)text[1] == 0xBB && (unsigned char)text[2] == 0xBF)
            {
                first += 3;
            }
        }
        ls.first_ = first;
        ls.end_ = first + text.length();
        memset(&ls.channels_, 0, sizeof(ls.channels_));
        ls.channel_size_ = 0;
        ls.hot_update_ = false;
        ls.current_ = ls.first_;
        ls.line_.line_type_ = LINE_NULL;
        ls.line_number_ = 1;
        do
        {
            const char* current = ls.current_;
            int ret = Lex(ls);
            if (ret != PEC_NONE)
            {
                ls.current_ = current;
                ls.line_number_--;
                return ret;
            }
            if (ls.line_.line_type_ == LINE_EOF)
            {
                break;
            }
            if (ls.line_.line_type_ == LINE_BLANK)
            {
                continue;
            }

            switch (ls.line_.key_)
            {
            case RK_HOT_UPDATE:
                ls.hot_update_ = ParseBool(ls.line_.val_begin_, ls.line_.val_end_);//"disable"
                break;
            case RK_CHANNEL:
                if (ls.line_.line_type_ != LINE_ARRAY)
                {
                    return PEC_CHANNEL_NOT_ARRAY;
                }
                else
                {
                    int channel_id = atoi(ls.line_.val_begin_);
                    if (ls.channel_size_ >= Logger::MAX_CHANNEL_SIZE)
                    {
                        return PEC_CHANNEL_INDEX_OUT_MAX;
                    }
                    if (ls.channel_size_ != channel_id)
                    {
                        return PEC_CHANNEL_INDEX_NOT_SEQUENCE;
                    }
                    Channel& channel = ls.channels_[ls.channel_size_++];
                    memset(&channel, 0, sizeof(channel));
                    channel.channel_id_ = channel_id;
                    ret = ParseChannel(ls, channel, ls.line_.blank_);
                    if (ret != 0)
                    {
                        return ret;
                    }
                }
                break;
            default:
                return PEC_UNDEFINED_GLOBAL_KEY;
            }
        } while (ls.line_.line_type_ != LINE_EOF);
        if (ls.channel_size_ == 0)
        {
            return PEC_NO_ANY_CHANNEL;
        }
        return PEC_NONE;
    }

#if __GNUG__ && __GNUC__ >= 5
#pragma GCC diagnostic pop
#endif

}


#endif
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
#ifndef _FN_LOG_FMT_H_
#define _FN_LOG_FMT_H_



namespace FNLog
{


#ifndef WIN32
    struct PriorityRender
    {
        const char* const priority_name_;
        int priority_len_;
        const char* const scolor_;
    };
    static const PriorityRender PRIORITY_RENDER[] =
    {
        {"[TRACE]", sizeof("[TRACE]") - 1, "\e[0m",                     },
        {"[DEBUG]", sizeof("[DEBUG]") - 1, "\e[0m",                     },
        {"[INFO ]", sizeof("[INFO ]") - 1, "\e[34m\e[1m",/*hight blue*/},
        {"[WARN ]", sizeof("[WARN ]") - 1, "\e[33m", /*yellow*/         },
        {"[ERROR]", sizeof("[ERROR]") - 1, "\e[31m", /*red*/            },
        {"[ALARM]", sizeof("[ALARM]") - 1, "\e[32m", /*green*/          },
        {"[FATAL]", sizeof("[FATAL]") - 1, "\e[35m",                    },
    };
#else
    struct PriorityRender
    {
        const char* const priority_name_;
        int priority_len_;
        const WORD color_;
    };
    static const PriorityRender PRIORITY_RENDER[] =
    {
        {"[TRACE]", sizeof("[TRACE]") - 1,  FOREGROUND_INTENSITY,               },
        {"[DEBUG]", sizeof("[DEBUG]") - 1,  FOREGROUND_INTENSITY,               },
        {"[INFO ]", sizeof("[INFO ]") - 1,  FOREGROUND_BLUE | FOREGROUND_GREEN, },
        {"[WARN ]", sizeof("[WARN ]") - 1,  FOREGROUND_GREEN | FOREGROUND_RED,  },
        {"[ERROR]", sizeof("[ERROR]") - 1,  FOREGROUND_RED,                     },
        {"[ALARM]", sizeof("[ALARM]") - 1,  FOREGROUND_GREEN,                   },
        {"[FATAL]", sizeof("[FATAL]") - 1,  FOREGROUND_RED | FOREGROUND_BLUE,   },
    };
#endif



    static_assert(PRIORITY_TRACE == 0, "");
    static_assert(sizeof(PRIORITY_RENDER) / sizeof(PriorityRender) == PRIORITY_MAX, "");

    
    template<int WIDE>
    int write_bin_unsafe(char* dst, unsigned long long number);
    template<int WIDE>
    int write_dec_unsafe(char* dst, unsigned long long number);
    template<int WIDE>
    int write_hex_unsafe(char* dst, unsigned long long number);

    template<int WIDE>
    int write_bin_unsafe(char* dst, long long number)
    {
        return write_bin_unsafe<WIDE>(dst, (unsigned long long) number);
    }
    template<int WIDE>
    int write_hex_unsafe(char* dst, long long number)
    {
        return write_hex_unsafe<WIDE>(dst, (unsigned long long) number);
    }
    template<int WIDE>
    int write_dec_unsafe(char* dst, long long number)
    {
        if (number < 0)
        {
            *dst = '-';
            number = -number;
            return 1 + write_dec_unsafe<WIDE - 1>(dst + 1, (unsigned long long) number);
        }
        return write_dec_unsafe<WIDE>(dst, (unsigned long long) number);
    }


    template<int WIDE>
    int write_dec_unsafe(char* dst, unsigned long long number)
    {
        static const char* dec_lut =
            "00010203040506070809"
            "10111213141516171819"
            "20212223242526272829"
            "30313233343536373839"
            "40414243444546474849"
            "50515253545556575859"
            "60616263646566676869"
            "70717273747576777879"
            "80818283848586878889"
            "90919293949596979899";

        static const int buf_len = 30;
        char buf[buf_len];
        int write_index = buf_len;
        unsigned long long m1 = 0;
        unsigned long long m2 = 0;
        do
        {
            m1 = number / 100;
            m2 = number % 100;
            m2 += m2;
            number = m1;
            *(buf + write_index - 1) = dec_lut[m2 + 1];
            *(buf + write_index - 2) = dec_lut[m2];
            write_index -= 2;
        } while (number);
        if (buf[write_index] == '0')
        {
            write_index++;
        }
        while (buf_len - write_index < WIDE)
        {
            write_index--;
            buf[write_index] = '0';
        }
        memcpy(dst, buf + write_index, buf_len - write_index);
        return buf_len - write_index;
    }


    template<int WIDE>
    int write_hex_unsafe(char* dst, unsigned long long number)
    {
        static const char* lut =
            "0123456789ABCDEFGHI";
        static const char* hex_lut =
            "000102030405060708090A0B0C0D0E0F"
            "101112131415161718191A1B1C1D1E1F"
            "202122232425262728292A2B2C2D2E2F"
            "303132333435363738393A3B3C3D3E3F"
            "404142434445464748494A4B4C4D4E4F"
            "505152535455565758595A5B5C5D5E5F"
            "606162636465666768696A6B6C6D6E6F"
            "707172737475767778797A7B7C7D7E7F"
            "808182838485868788898A8B8C8D8E8F"
            "909192939495969798999A9B9C9D9E9F"
            "A0A1A2A3A4A5A6A7A8A9AAABACADAEAF"
            "B0B1B2B3B4B5B6B7B8B9BABBBCBDBEBF"
            "C0C1C2C3C4C5C6C7C8C9CACBCCCDCECF"
            "D0D1D2D3D4D5D6D7D8D9DADBDCDDDEDF"
            "E0E1E2E3E4E5E6E7E8E9EAEBECEDEEEF"
            "F0F1F2F3F4F5F6F7F8F9FAFBFCFDFEFF";



        int real_wide = 0;
#ifndef WIN32
        real_wide = sizeof(number) * 8 - __builtin_clzll(number);
#else
        unsigned long win_index = 0;
        if (_BitScanReverse(&win_index, (unsigned long)(number >> 32)))
        {
            real_wide = win_index + 1 + 32;
        }
        else if(_BitScanReverse(&win_index, (unsigned long)(number & 0xffffffff)))
        {
            real_wide = win_index + 1;
        }
#endif 
        switch (real_wide)
        {
        case  1:case  2:case  3:case  4:real_wide = 1; break;
        case  5:case  6:case  7:case  8:real_wide = 2; break;
        case 9: case 10:case 11:case 12:real_wide = 3; break;
        case 13:case 14:case 15:case 16:real_wide = 4; break;
        case 17:case 18:case 19:case 20:real_wide = 5; break;
        case 21:case 22:case 23:case 24:real_wide = 6; break;
        case 25:case 26:case 27:case 28:real_wide = 7; break;
        case 29:case 30:case 31:case 32:real_wide = 8; break;
        case 33:case 34:case 35:case 36:real_wide = 9; break;
        case 37:case 38:case 39:case 40:real_wide = 10; break;
        case 41:case 42:case 43:case 44:real_wide = 11; break;
        case 45:case 46:case 47:case 48:real_wide = 12; break;
        case 49:case 50:case 51:case 52:real_wide = 13; break;
        case 53:case 54:case 55:case 56:real_wide = 14; break;
        case 57:case 58:case 59:case 60:real_wide = 15; break;
        case 61:case 62:case 63:case 64:real_wide = 16; break;
        }
        if (real_wide < WIDE)
        {
            real_wide = WIDE;
        }
        unsigned long long cur_wide = real_wide;
        while (number && cur_wide >= 2)
        {
            const unsigned long long m2 = (unsigned long long)((number % 256) * 2);
            number /= 256;
            *(dst + cur_wide - 1) = hex_lut[m2 + 1];
            *(dst + cur_wide - 2) = hex_lut[m2];
            cur_wide -= 2;
        } 
        if (number)
        {
            *dst = lut[number % 16];
            cur_wide --;
        }
        while (cur_wide-- != 0)
        {
            *(dst + cur_wide) = '0';
        }
        return real_wide;
    }


    template<int WIDE>
    int write_bin_unsafe(char* dst, unsigned long long number)
    {
        static const char* lut =
            "0123456789abcdefghijk";

        int real_wide = 0;
#ifndef WIN32
        real_wide = sizeof(number) * 8 - __builtin_clzll(number);
#else
        unsigned long win_index = 0;
        _BitScanReverse64(&win_index, number);
        real_wide = (int)win_index + 1;
#endif 
        if (real_wide < WIDE)
        {
            real_wide = WIDE;
        }
        unsigned long long cur_wide = real_wide;
        do
        {
            const unsigned long long m2 = number & 1;
            number >>= 1;
            *(dst + cur_wide - 1) = lut[m2];
            cur_wide--;
        } while (number);
        return real_wide;
    }

    inline int write_double_unsafe(char* dst, double number)
    {
        int fp_class = std::fpclassify(number);
        switch (fp_class)
        {
        case FP_NAN:
            memcpy(dst, "nan", 3);
            return 3;
        case FP_INFINITE:
            memcpy(dst, "inf", 3);
            return 3;
        }


        double fabst = std::fabs(number);

        
        if (fabst < 0.0001 || fabst > 0xFFFFFFFFFFFFFFFULL)
        {
            char * buf = gcvt(number, 16, dst);
            (void)buf;
            return (int)strlen(dst);
        }

        if (number < 0.0)
        {
            double intpart = 0;
            unsigned long long fractpart = (unsigned long long)(modf(fabst, &intpart) * 10000);
            *dst = '-';
            int writed_len = 1 + write_dec_unsafe<0>(dst + 1, (unsigned long long)intpart);
            if (fractpart > 0)
            {
                *(dst + writed_len) = '.';
                return writed_len + 1 + write_dec_unsafe<4>(dst + writed_len + 1, (unsigned long long)fractpart);
            }
            return writed_len;
        }

        double intpart = 0;
        unsigned long long fractpart = (unsigned long long)(modf(fabst, &intpart) * 10000);
        int writed_len = write_dec_unsafe<0>(dst, (unsigned long long)intpart);
        if (fractpart > 0)
        {
            *(dst + writed_len) = '.';
            writed_len++;
            int wide = 4;
            int fp = (int)fractpart;
            while (fp % 10 == 0 && wide > 1)
            {
                wide--;
                fp /= 10;
            }
            writed_len += wide;
            fp = (int)fractpart;
            switch (wide)
            {
            case 1:
                *(dst + writed_len - 1) = "0123456789"[fp / 1000];
                break;
            case 2:
                *(dst + writed_len - 2) = "0123456789"[fp / 1000];
                *(dst + writed_len - 1) = "0123456789"[fp / 100 % 10];
                break;
            case 3:
                *(dst + writed_len - 3) = "0123456789"[fp / 1000];
                *(dst + writed_len - 2) = "0123456789"[fp / 100 % 10];
                *(dst + writed_len - 1) = "0123456789"[fp / 10 % 10];
                break;
            case 4:
                *(dst + writed_len - 4) = "0123456789"[fp / 1000];
                *(dst + writed_len - 3) = "0123456789"[fp / 100 % 10];
                *(dst + writed_len - 2) = "0123456789"[fp / 10 % 10];
                *(dst + writed_len - 1) = "0123456789"[fp % 10];
                break;
            default:
                break;
            }
        }
        return writed_len;
    }

    inline int write_float_unsafe(char* dst, float number)
    {
        if (std::isnan(number))
        {
            memcpy(dst, "nan", 3);
            return 3;
        }
        else if (std::isinf(number))
        {
            memcpy(dst, "inf", 3);
            return 3;
        }

        double fabst = std::fabs(number);


        if (fabst < 0.0001 || fabst > 0xFFFFFFFULL)
        {
            char* buf = gcvt(number, 7, dst);
            (void)buf;
            return (int)strlen(dst);
        }

        if (number < 0.0)
        {
            double intpart = 0;
            unsigned long long fractpart = (unsigned long long)(modf(fabst, &intpart) * 10000);
            *dst = '-';
            int writed_len = 1 + write_dec_unsafe<0>(dst + 1, (unsigned long long)intpart);
            if (fractpart > 0)
            {
                *(dst + writed_len) = '.';
                return writed_len + 1 + write_dec_unsafe<4>(dst + writed_len + 1, (unsigned long long)fractpart);
            }
            return writed_len;
        }

        double intpart = 0;
        unsigned long long fractpart = (unsigned long long)(modf(fabst, &intpart) * 10000);
        int writed_len = write_dec_unsafe<0>(dst, (unsigned long long)intpart);
        if (fractpart > 0)
        {
            *(dst + writed_len) = '.';
            writed_len++;
            int wide = 4;
            int fp = (int)fractpart;
            while (fp %10 == 0 && wide > 1)
            {
                wide--;
                fp /= 10;
            }
            writed_len += wide;
            fp = (int)fractpart;
            switch (wide)
            {
            case 1:
                *(dst + writed_len - 1) = "0123456789"[fp / 1000];
                break;
            case 2:
                *(dst + writed_len - 2) = "0123456789"[fp / 1000];
                *(dst + writed_len - 1) = "0123456789"[fp / 100 % 10];
                break;
            case 3:
                *(dst + writed_len - 3) = "0123456789"[fp / 1000];
                *(dst + writed_len - 2) = "0123456789"[fp / 100 % 10];
                *(dst + writed_len - 1) = "0123456789"[fp / 10 % 10];
                break;
            case 4:
                *(dst + writed_len - 4) = "0123456789"[fp / 1000];
                *(dst + writed_len - 3) = "0123456789"[fp / 100 % 10];
                *(dst + writed_len - 2) = "0123456789"[fp / 10 % 10];
                *(dst + writed_len - 1) = "0123456789"[fp % 10];
                break;
            default:
                break;
            }
        }
        return writed_len;
    }

    inline int write_date_unsafe(char* dst, long long timestamp, unsigned int precise)
    {
        static thread_local tm cache_date = { 0 };
        static thread_local long long cache_timestamp = 0;
        static const char date_fmt[] = "[20190412 13:05:35.417]";
        long long day_second = timestamp - cache_timestamp;
        if (day_second < 0 || day_second >= 24*60*60)
        {
            cache_date = FileHandler::time_to_tm((time_t)timestamp);
            struct tm daytm = cache_date;
            daytm.tm_hour = 0;
            daytm.tm_min = 0;
            daytm.tm_sec = 0;
            cache_timestamp = mktime(&daytm);
            day_second = timestamp - cache_timestamp;
        }
        int write_bytes = 0;

        *(dst + write_bytes++) = '[';

        write_bytes += write_dec_unsafe<4>(dst + write_bytes, (unsigned long long)cache_date.tm_year + 1900);
        write_bytes += write_dec_unsafe<2>(dst + write_bytes, (unsigned long long)cache_date.tm_mon + 1);
        write_bytes += write_dec_unsafe<2>(dst + write_bytes, (unsigned long long)cache_date.tm_mday);

        *(dst + write_bytes++) = ' ';

        write_bytes += write_dec_unsafe<2>(dst + write_bytes, (unsigned long long)day_second/3600);
        *(dst + write_bytes++) = ':';
        day_second %= 3600;
        write_bytes += write_dec_unsafe<2>(dst + write_bytes, (unsigned long long)day_second / 60);
        *(dst + write_bytes++) = ':';
        day_second %= 60;
        write_bytes += write_dec_unsafe<2>(dst + write_bytes, (unsigned long long)day_second);

        *(dst + write_bytes++) = '.';
        if (precise >= 1000)
        {
            precise = 999;
        }
        write_bytes += write_dec_unsafe<3>(dst + write_bytes, (unsigned long long)precise);

        *(dst + write_bytes++) = ']';

        if (write_bytes != sizeof(date_fmt) - 1)
        {
            return 0;
        }

        return write_bytes;
    }

    inline int write_log_priority_unsafe(char* dst, int priority)
    {
        priority = priority % PRIORITY_MAX;
        memcpy(dst, PRIORITY_RENDER[priority].priority_name_, PRIORITY_RENDER[priority].priority_len_);
        return PRIORITY_RENDER[priority].priority_len_;
    }

    inline int write_log_thread_unsafe(char* dst, unsigned int thread_id)
    {
        int write_bytes = 0;
        *(dst + write_bytes) = ' ';
        write_bytes++;
        *(dst + write_bytes) = '[';
        write_bytes++;
        write_bytes += write_dec_unsafe<0>(dst + write_bytes, (unsigned long long) thread_id);
        *(dst + write_bytes) = ']';
        write_bytes++;
        return write_bytes;
    }

    inline int write_pointer_unsafe(char* dst, const void* ptr)
    {
        if (ptr == nullptr)
        {
            memcpy(dst, "null", 4);
            return 4;
        }
        int write_bytes = 2;
        memcpy(dst, "0x", 2);
        write_bytes += write_hex_unsafe<0>(dst + write_bytes, (unsigned long long) ptr);
        return write_bytes;
    }

}


#endif
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

#include <sstream>

namespace FNLog
{
#if __GNUG__ && __GNUC__ >= 5
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess"
#endif
    

    inline int InitFromYMAL(Logger& logger, const std::string& text, const std::string& path)
    {
        Logger::StateLockGuard state_guard(logger.state_lock);
        if (logger.logger_state_ != LOGGER_STATE_UNINIT)
        {
            printf("init from ymal:<%s> text error\n", path.c_str());
            return -1;
        }
        if (logger.shm_ == nullptr)
        {
            printf("%s", "init from ymal text error. no shm.\n");
            return -2;
        }
        std::unique_ptr<LexState> ls(new LexState);
        int ret = ParseLogger(*ls, text);
        if (ret != PEC_NONE)
        {
            std::stringstream os;
            os << "load has error:<" << ret << "> in line:[" << ls->line_number_ << "], line type:" << ls->line_.line_type_;
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

        logger.yaml_path_ = path;
        logger.hot_update_ = ls->hot_update_;
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
            printf("start error 2");
            return -2;
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
            printf("ymal:<%s> open file error\n", path.c_str());
            return -1;
        }
        int ret = InitFromYMAL(logger, config.read_content(), path);
        if (ret != 0)
        {
            printf("ymal:<%s> has parse/init error\n", path.c_str());
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
            return -5;
        }
        if (file_stat.st_mtime == dst_chl.yaml_mtime_)
        {
            return -6;
        }

        Logger::StateLockGuard state_guard(logger.state_lock);
        if (logger.logger_state_ != LOGGER_STATE_RUNNING)
        {
            return -7;
        }

        dst_chl.yaml_mtime_ = file_stat.st_mtime;

        std::unique_ptr<LexState> ls(new LexState);
        static_assert(std::is_same<decltype(logger.shm_->channels_), decltype(ls->channels_)>::value, "");
        //static_assert(std::is_trivial<decltype(logger.shm_->channels_)>::value, "");

        std::string text = config.read_content();
        int ret = ParseLogger(*ls, text);
        if (ret != PEC_NONE)
        {
            return ret;
        }
        logger.hot_update_ = ls->hot_update_;

        static_assert(std::is_same<decltype(logger.shm_->channels_[channel_id].config_fields_), decltype(ls->channels_[channel_id].config_fields_)>::value, "");
        


        Channel& src_chl = ls->channels_[channel_id];
        if (dst_chl.channel_id_ != src_chl.channel_id_ || src_chl.channel_id_ != channel_id)
        {
            return - 7;
        }
        for (int field_id = 0; field_id < CHANNEL_CFG_MAX_ID; field_id++)
        {
            //this is multi-thread safe op. 
            dst_chl.config_fields_[field_id] = src_chl.config_fields_[field_id].load();
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

#if __GNUG__ && __GNUC__ >= 5
#pragma GCC diagnostic pop
#endif
}


#endif
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
#ifndef _FN_LOG_OUT_FILE_DEVICE_H_
#define _FN_LOG_OUT_FILE_DEVICE_H_


namespace FNLog
{

    //support
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

    inline void OpenFileDevice(Logger & logger, Channel & channel, Device & device, FileHandler & writer, LogData & log)
    {
        bool sameday = true;
        if (log.timestamp_ < AtomicLoadL(device, DEVICE_LOG_CUR_FILE_CREATE_DAY)
            || log.timestamp_ >= AtomicLoadL(device, DEVICE_LOG_CUR_FILE_CREATE_DAY) + 24 * 3600)
        {
            sameday = false;
        }

        bool file_over = false;
        if (AtomicLoadC(device, DEVICE_CFG_FILE_LIMIT_SIZE) > 0 && AtomicLoadC(device, DEVICE_CFG_FILE_ROLLBACK) > 0
            && AtomicLoadL(device, DEVICE_LOG_CUR_FILE_SIZE) + log.content_len_ > AtomicLoadC(device, DEVICE_CFG_FILE_LIMIT_SIZE))
        {
            file_over = true;
        }

        if (!sameday || file_over)
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
        tm t = FileHandler::time_to_tm(log.timestamp_);
        if (true) //process day time   
        {
            tm day = t;
            day.tm_hour = 0;
            day.tm_min = 0;
            day.tm_sec = 0;
            create_day = mktime(&day);
        }

        std::string name = MakeFileName(device.out_file_, channel.channel_id_, device.device_id_, t);

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
            AtomicStoreL(device, DEVICE_LOG_LAST_TRY_CREATE_ERROR, 1);
            AtomicStoreL(device, DEVICE_LOG_LAST_TRY_CREATE_TIMESTAMP, log.timestamp_);
            return;
        }

        if (AtomicLoadC(device, DEVICE_CFG_FILE_ROLLBACK) > 0 || AtomicLoadC(device, DEVICE_CFG_FILE_LIMIT_SIZE) > 0)
        {
            //when no rollback but has limit size. need try rollback once.
            long long limit_roll = device.config_fields_[DEVICE_CFG_FILE_ROLLBACK];
            limit_roll = limit_roll > 0 ? limit_roll : 1;
            FileHandler::rollback(path, 1, (int)limit_roll);
        }

        struct stat file_stat;
        long writed_byte = writer.open(path.c_str(), "ab", file_stat);
        if (!writer.is_open())
        {
            AtomicStoreL(device, DEVICE_LOG_LAST_TRY_CREATE_ERROR, 2);
            AtomicStoreL(device, DEVICE_LOG_LAST_TRY_CREATE_TIMESTAMP, log.timestamp_);
            return;
        }
        AtomicStoreL(device, DEVICE_LOG_LAST_TRY_CREATE_ERROR, 0);
        AtomicStoreL(device, DEVICE_LOG_LAST_TRY_CREATE_TIMESTAMP, 0);
        AtomicStoreL(device, DEVICE_LOG_CUR_FILE_CREATE_TIMESTAMP, log.timestamp_);
        AtomicStoreL(device, DEVICE_LOG_CUR_FILE_CREATE_DAY, create_day);
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
    }


}


#endif
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
#ifndef _FN_LOG_OUT_UDP_DEVICE_H_
#define _FN_LOG_OUT_UDP_DEVICE_H_


namespace FNLog
{

    inline void EnterProcOutUDPDevice(Logger& logger, int channel_id, int device_id, LogData& log)
    {
        auto& udp = logger.udp_handles_[channel_id * Channel::MAX_DEVICE_SIZE + device_id];
        if (!udp.is_open())
        {
            udp.open();
        }
        if (!udp.is_open())
        {
            return;
        }
        Device& device = logger.shm_->channels_[channel_id].devices_[device_id];
        long long ip = AtomicLoadC(device, DEVICE_CFG_UDP_IP);
        long long port = AtomicLoadC(device, DEVICE_CFG_UDP_PORT);
        udp.write((unsigned long)ip, (unsigned short)port, log.content_, log.content_len_);
        AtomicAddL(device, DEVICE_LOG_TOTAL_WRITE_LINE);
        AtomicAddLV(device, DEVICE_LOG_TOTAL_WRITE_BYTE, log.content_len_);
    }
}


#endif
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
#ifndef _FN_LOG_OUT_SCREEN_DEVICE_H_
#define _FN_LOG_OUT_SCREEN_DEVICE_H_


namespace FNLog
{

    inline void EnterProcOutScreenDevice(Logger& logger, int channel_id, int device_id, LogData& log)
    {
        Logger::ScreenLockGuard l(logger.screen_lock_);
        Device& device = logger.shm_->channels_[channel_id].devices_[device_id];
        AtomicAddL(device, DEVICE_LOG_TOTAL_WRITE_LINE);
        AtomicAddLV(device, DEVICE_LOG_TOTAL_WRITE_BYTE, log.content_len_);
        int priority = log.priority_;
        if (log.priority_ < PRIORITY_INFO)
        {
            printf("%s", log.content_);
            return;
        }
        if (priority >= PRIORITY_MAX)
        {
            priority = PRIORITY_ALARM;
        }
#ifndef WIN32
        printf("%s%s\e[0m", PRIORITY_RENDER[priority].scolor_, log.content_);
#else

        HANDLE sc_handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
        if (sc_handle == INVALID_HANDLE_VALUE)
        {
            printf("%s", log.content_);
            return;
        }
        CONSOLE_SCREEN_BUFFER_INFO old_info;
        if (!GetConsoleScreenBufferInfo(sc_handle, &old_info))
        {
            printf("%s", log.content_);
            return;
        }
        else
        {
            SetConsoleTextAttribute(sc_handle, PRIORITY_RENDER[priority].color_);
            printf("%s", log.content_);
            SetConsoleTextAttribute(sc_handle, old_info.wAttributes);
        }
#endif
    }



}


#endif
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


namespace FNLog
{
    inline void EnterProcDevice(Logger& logger, int channel_id, int device_id, LogData & log)
    {
        Channel& channel = logger.shm_->channels_[channel_id];
        Device& device = channel.devices_[device_id];
        //async promise only single thread proc. needn't lock.
        Logger::ReadGuard rg(logger.read_locks_[channel_id], channel.channel_type_ == CHANNEL_ASYNC);
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
            if (!AtomicLoadC(device, DEVICE_CFG_ABLE))
            {
                continue;
            }
            if (log.priority_ < AtomicLoadC(device, DEVICE_CFG_PRIORITY))
            {
                continue;
            }
            if (AtomicLoadC(device, DEVICE_CFG_CATEGORY) > 0)
            {
                if (log.category_ < AtomicLoadC(device, DEVICE_CFG_CATEGORY)
                    || log.category_ > AtomicLoadC(device, DEVICE_CFG_CATEGORY) + AtomicLoadC(device, DEVICE_CFG_CATEGORY_EXTEND))
                {
                    continue;
                }
            }
            EnterProcDevice(logger, channel.channel_id_, device_id, log);
        }
    }
    
 
    inline void EnterProcChannel(Logger& logger, int channel_id)
    {
        Channel& channel = logger.shm_->channels_[channel_id];
        RingBuffer& ring_buffer = logger.shm_->ring_buffers_[channel_id];
        do
        {
            int local_write_count = 0;
            do
            {
                int old_idx = ring_buffer.proc_idx_.load(std::memory_order_acquire);
                int next_idx = (old_idx + 1) % RingBuffer::BUFFER_LEN;
                if (old_idx == ring_buffer.write_idx_.load(std::memory_order_acquire))
                {
                    //empty branch    
                    break;
                }

                //set proc index  
                if (!ring_buffer.proc_idx_.compare_exchange_strong(old_idx, next_idx))
                {
                    //only one thread get log. this branch will not hit.   
                    break;
                }
                auto& cur_log = ring_buffer.buffer_[old_idx];
                DispatchLog(logger, channel, cur_log);
                cur_log.data_mark_ = 0;
                AtomicAddL(channel, CHANNEL_LOG_PROCESSED);
                local_write_count ++;


                do
                {
                    //set read index to proc index  
                    old_idx = ring_buffer.read_idx_.load(std::memory_order_acquire);
                    next_idx = (old_idx + 1) % RingBuffer::BUFFER_LEN;
                    if (old_idx == ring_buffer.proc_idx_.load(std::memory_order_acquire))
                    {
                        break;
                    }
                    if (ring_buffer.buffer_[old_idx].data_mark_.load(std::memory_order_acquire) != MARK_INVALID)
                    {
                        break;
                    }
                    ring_buffer.read_idx_.compare_exchange_strong(old_idx, next_idx);
                } while (true);  

                //if want the high log security can reduce this threshold or enable shared memory queue.  
                if (local_write_count > 10000)
                {
                    local_write_count = 0;
                    for (int i = 0; i < channel.device_size_; i++)
                    {
                        if (channel.devices_[i].out_type_ == DEVICE_OUT_FILE)
                        {
                            logger.file_handles_[channel_id * Channel::MAX_DEVICE_SIZE + i].flush();
                        }
                    }
                }
            } while (true);  


            if (channel.channel_state_ == CHANNEL_STATE_NULL)
            {
                channel.channel_state_ = CHANNEL_STATE_RUNNING;
            }

            if (local_write_count)
            {
                for (int i = 0; i < channel.device_size_; i++)
                {
                    if (channel.devices_[i].out_type_ == DEVICE_OUT_FILE)
                    {
                        logger.file_handles_[channel_id * Channel::MAX_DEVICE_SIZE + i].flush();
                    }
                }
            }
            HotUpdateLogger(logger, channel.channel_id_);
            if (channel.channel_type_ == CHANNEL_ASYNC)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            
        } while (channel.channel_type_ == CHANNEL_ASYNC 
            && (channel.channel_state_ == CHANNEL_STATE_RUNNING || ring_buffer.write_idx_ != ring_buffer.read_idx_));

        if (channel.channel_type_ == CHANNEL_ASYNC)
        {
            channel.channel_state_ = CHANNEL_STATE_FINISH;
        }
    }
    
    

    inline void InitLogData(Logger& logger, LogData& log, int channel_id, int priority, int category, unsigned int prefix)
    {
        log.channel_id_ = channel_id;
        log.priority_ = priority;
        log.category_ = category;
        log.content_len_ = 0;
        log.content_[log.content_len_] = '\0';

#ifdef WIN32
        FILETIME ft;
        GetSystemTimeAsFileTime(&ft);
        unsigned long long now = ft.dwHighDateTime;
        now <<= 32;
        now |= ft.dwLowDateTime;
        now /= 10;
        now -= 11644473600000000ULL;
        now /= 1000;
        log.timestamp_ = now / 1000;
        log.precise_ = (unsigned int)(now % 1000);
#else
        struct timeval tm;
        gettimeofday(&tm, nullptr);
        log.timestamp_ = tm.tv_sec;
        log.precise_ = tm.tv_usec / 1000;
#endif
        log.thread_ = 0;
        if (prefix == LOG_PREFIX_NULL)
        {
            return;
        }

#ifdef WIN32
        static thread_local unsigned int therad_id = GetCurrentThreadId();
        log.thread_ = therad_id;
#elif defined(__APPLE__)
        unsigned long long tid = 0;
        pthread_threadid_np(nullptr, &tid);
        log.thread_ = (unsigned int)tid;
#else
        static thread_local unsigned int therad_id = (unsigned int)syscall(SYS_gettid);
        log.thread_ = therad_id;
#endif
        if (prefix & LOG_PREFIX_TIMESTAMP)
        {
            log.content_len_ += write_date_unsafe(log.content_ + log.content_len_, log.timestamp_, log.precise_);
        }
        if (prefix & LOG_PREFIX_PRIORITY)
        {
            log.content_len_ += write_log_priority_unsafe(log.content_ + log.content_len_, log.priority_);
        }
        if (prefix & LOG_PREFIX_THREAD)
        {
            log.content_len_ += write_log_thread_unsafe(log.content_ + log.content_len_, log.thread_);
        }
        log.content_[log.content_len_] = '\0';
        return;
    }

    inline int HoldChannel(Logger& logger, int channel_id, int priority, int category)
    {
        if (channel_id >= logger.shm_->channel_size_ || channel_id < 0)
        {
            return -1;
        }
        if (logger.logger_state_ != LOGGER_STATE_RUNNING)
        {
            return -2;
        }
        Channel& channel = logger.shm_->channels_[channel_id];
        RingBuffer& ring_buffer = logger.shm_->ring_buffers_[channel_id];
        if (channel.channel_state_ != CHANNEL_STATE_RUNNING)
        {
            return -3;
        }
        if (priority < AtomicLoadC(channel, CHANNEL_CFG_PRIORITY))
        {
            return -4;
        }
        if (AtomicLoadC(channel, CHANNEL_CFG_CATEGORY) > 0)
        {
            if (category < AtomicLoadC(channel, CHANNEL_CFG_CATEGORY)
                || category > AtomicLoadC(channel, CHANNEL_CFG_CATEGORY) + AtomicLoadC(channel, CHANNEL_CFG_CATEGORY_EXTEND))
            {
                return -5;
            }
        }
        bool need_write = false;

        for (int i = 0; i < logger.shm_->channels_[channel_id].device_size_; i++)
        {
            if (logger.shm_->channels_[channel_id].devices_[i].config_fields_[FNLog::DEVICE_CFG_ABLE] && priority >= logger.shm_->channels_[channel_id].devices_[i].config_fields_[FNLog::DEVICE_CFG_PRIORITY])
            {
                need_write = true;
                break;
            }
        }
        if (!need_write)
        {
            return -6;
        }



        int state = 0;
        do
        {
            if (state > 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            state++;

            for (int i = 0; i < FN_MAX(RingBuffer::BUFFER_LEN, 10); i++)
            {
                if (channel.channel_state_ != CHANNEL_STATE_RUNNING)
                {
                    break;
                }
                int old_idx = ring_buffer.hold_idx_.load(std::memory_order_acquire);
                int hold_idx = (old_idx + 1) % RingBuffer::BUFFER_LEN;
                if (hold_idx == ring_buffer.read_idx_.load(std::memory_order_acquire))
                {
                    break;
                }
                if (ring_buffer.hold_idx_.compare_exchange_strong(old_idx, hold_idx))
                {
                    AtomicAddL(channel, CHANNEL_LOG_HOLD);
                    ring_buffer.buffer_[old_idx].data_mark_.store(MARK_HOLD, std::memory_order_release);
                    return old_idx;
                }
                continue;
            }
            if (channel.channel_state_ != CHANNEL_STATE_RUNNING)
            {
                break;
            }
        } while (true);
        return -10;
    }

    inline int PushChannel(Logger& logger, int channel_id, int hold_idx)
    {
        if (channel_id >= logger.shm_->channel_size_ || channel_id < 0)
        {
            return -1;
        }
        if (hold_idx >= RingBuffer::BUFFER_LEN || hold_idx < 0)
        {
            return -2;
        }
        Channel& channel = logger.shm_->channels_[channel_id];
        RingBuffer& ring_buffer = logger.shm_->ring_buffers_[channel_id];
        if (channel.channel_state_ != CHANNEL_STATE_RUNNING)
        {
            return -1;
        }

        LogData& log = ring_buffer.buffer_[hold_idx];
        log.content_len_ = FN_MIN(log.content_len_, LogData::LOG_SIZE - 2);
        log.content_[log.content_len_++] = '\n';
        log.content_[log.content_len_] = '\0';

        log.data_mark_ = 2;


        do
        {
            int old_idx = ring_buffer.write_idx_.load(std::memory_order_acquire);
            int next_idx = (old_idx + 1) % RingBuffer::BUFFER_LEN;
            if (old_idx == ring_buffer.hold_idx_.load(std::memory_order_acquire))
            {
                break;
            }
            if (ring_buffer.buffer_[old_idx].data_mark_.load(std::memory_order_acquire) != 2)
            {
                break;
            }
            if (ring_buffer.write_idx_.compare_exchange_strong(old_idx, next_idx))
            {
                AtomicAddL(channel, CHANNEL_LOG_PUSH);
            }
        } while (channel.channel_state_ == CHANNEL_STATE_RUNNING);

        if (channel.channel_type_ == CHANNEL_SYNC && channel.channel_state_ == CHANNEL_STATE_RUNNING)
        {
            EnterProcChannel(logger, channel_id); //no affect channel.single_thread_write_
        }
        return 0;
    }
}


#endif
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


namespace FNLog
{

#if __GNUG__ && __GNUC__ >= 5
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

    inline bool FastCheckPriorityPass(Logger& logger, int channel_id, int priority, int category)
    {
        if (logger.shm_->channel_size_ <= channel_id || priority < logger.shm_->channels_[channel_id].config_fields_[FNLog::CHANNEL_CFG_PRIORITY])
        {
            return true;
        }
        for (int i = 0; i < logger.shm_->channels_[channel_id].device_size_; i++)
        {
            if (logger.shm_->channels_[channel_id].devices_[i].config_fields_[FNLog::DEVICE_CFG_ABLE] && priority >= logger.shm_->channels_[channel_id].devices_[i].config_fields_[FNLog::DEVICE_CFG_PRIORITY])
            {
                return false;
            }
        }
        return true;
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

#if __GNUG__ && __GNUC__ >= 5
#pragma GCC diagnostic pop
#endif

}


#endif
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
#ifndef _FN_LOG_STREAM_H_
#define _FN_LOG_STREAM_H_


namespace FNLog
{
    class LogStream
    {
    public:
        static const int MAX_CONTAINER_DEPTH = 5;
    public:
        LogStream(const LogStream& other) = delete;
        LogStream(LogStream&& other) noexcept
        {
            logger_ = other.logger_;
            log_data_ = other.log_data_;
            hold_idx_ = other.hold_idx_;
            other.logger_ = nullptr;
            other.log_data_ = nullptr;
            other.hold_idx_ = -1;
        }

        explicit LogStream(Logger& logger, int channel_id, int priority, int category, 
            const char * const file_name, int file_name_len, int line,
            const char * const func_name, int func_name_len, unsigned int prefix)
        {
            logger_ = nullptr;
            log_data_ = nullptr;
            int hold_idx = HoldChannel(logger, channel_id, priority, category);
            if (hold_idx < 0)
            {
                return;
            }

            try
            {
                InitLogData(logger, logger.shm_->ring_buffers_[channel_id].buffer_[hold_idx], channel_id, priority, category, prefix);
            }
            catch (const std::exception&)
            {
                printf("%s", "alloc log error. no more memory.");
                return;
            }
            logger_ = &logger;
            log_data_ = &logger.shm_->ring_buffers_[channel_id].buffer_[hold_idx];
            hold_idx_ = hold_idx;
            if (prefix == LOG_PREFIX_NULL)
            {
                return;
            }
            if (prefix & LOG_PREFIX_FILE)
            {
                write_char_unsafe(' ');
                if (file_name && file_name_len > 0)
                {
                    int jump_bytes = short_path(file_name, file_name_len);
                    write_buffer_unsafe(file_name + jump_bytes, file_name_len - jump_bytes);
                }
                else
                {
                    write_buffer_unsafe("nofile", 6);
                }
                write_char_unsafe(':');
                write_char_unsafe('<');
                *this << (unsigned long long)line;
                write_char_unsafe('>');
                write_char_unsafe(' ');
            }
            if (prefix & LOG_PREFIX_FUNCTION)
            {
                if (func_name && func_name_len > 0)
                {
                    write_buffer_unsafe(func_name, func_name_len);
                }
                else
                {
                    write_buffer_unsafe("null", 4);
                }
                write_char_unsafe(' ');
            }
        }
        
        ~LogStream()
        {
            if (log_data_) 
            {
                PushLog(*logger_, log_data_->channel_id_, hold_idx_);
                hold_idx_ = -1;
                log_data_ = nullptr;
                logger_ = nullptr;
            }
        }
        
        LogStream& set_category(int category) { if (log_data_) log_data_->category_ = category;  return *this; }
        LogStream& write_char_unsafe(char ch)
        {
            log_data_->content_[log_data_->content_len_] = ch;
            log_data_->content_len_++;
            return *this;
        }
        LogStream& write_buffer_unsafe(const char* src, int src_len)
        {
            memcpy(log_data_->content_ + log_data_->content_len_, src, src_len);
            log_data_->content_len_ += src_len;
            return *this;
        }

        LogStream& write_buffer(const char* src, int src_len)
        {
            if (log_data_ && src && src_len > 0 && log_data_->content_len_ < LogData::LOG_SIZE)
            {
                src_len = FN_MIN(src_len, LogData::LOG_SIZE - log_data_->content_len_);
                memcpy(log_data_->content_ + log_data_->content_len_, src, src_len);
                log_data_->content_len_ += src_len;
            }
            return *this;
        }


        template<size_t Wide>
        LogStream& write_longlong(long long n)
        {
            if (log_data_ && log_data_->content_len_ + 30 <= LogData::LOG_SIZE)
            {
                log_data_->content_len_ += write_dec_unsafe<Wide>(log_data_->content_ + log_data_->content_len_, n);
            }
            return *this;
        }
        template<size_t Wide>
        LogStream& write_ulonglong(unsigned long long n)
        {
            if (log_data_ && log_data_->content_len_ + 30 <= LogData::LOG_SIZE)
            {
                log_data_->content_len_ += write_dec_unsafe<Wide>(log_data_->content_ + log_data_->content_len_, n);
            }
            return *this;
        }

        template<size_t Wide, class N>
        LogStream& write_number(N n)
        {
            if (std::is_signed<N>::value)
            {
                return write_longlong<Wide>((long long)n);
            }
            return write_ulonglong<Wide>((unsigned long long)n);
        }

        LogStream& write_pointer(const void* ptr)
        {
            if (log_data_ && log_data_->content_len_ + 30 <= LogData::LOG_SIZE)
            {
                log_data_->content_len_ += FNLog::write_pointer_unsafe(log_data_->content_ + log_data_->content_len_, ptr);
            }
            return *this;
        }

        LogStream& write_binary(const char* dst, int len)
        {
            if (!log_data_)
            {
                return *this;
            }
            write_buffer("\r\n\t[", sizeof("\r\n\t[")-1);
            for (int i = 0; i < (len / 32) + 1; i++)
            {
                write_buffer("\r\n\t[", sizeof("\r\n\t[") - 1);
                *this << (void*)(dst + (size_t)i * 32);
                write_buffer(": ", sizeof(": ") - 1);
                for (int j = i * 32; j < (i + 1) * 32 && j < len; j++)
                {
                    if (isprint((unsigned char)dst[j]))
                    {
                        write_buffer(" ", sizeof(" ") - 1);
                        write_buffer(dst + j, 1);
                        write_buffer(" ", sizeof(" ") - 1);
                    }
                    else
                    {
                        write_buffer(" . ", sizeof(" . ") - 1);
                    }
                }
                write_buffer("\r\n\t[", sizeof("\r\n\t[") - 1);
                if (log_data_->content_len_ + sizeof(void*) <= LogData::LOG_SIZE)
                {
                    write_pointer(dst + (size_t)i * 32);
                }
                write_buffer(": ", sizeof(": ") - 1);
                for (int j = i * 32; j < (i + 1) * 32 && j < len; j++)
                {
                    if (log_data_->content_len_ + 30 >= LogData::LOG_SIZE)
                    {
                        break;
                    }
                    log_data_->content_len_ += FNLog::write_hex_unsafe<2>(log_data_->content_ + log_data_->content_len_,
                        (unsigned long long)(unsigned char)dst[j]);
                    write_buffer(" ", sizeof(" ") - 1);
                }
            }
            write_buffer("\r\n\t]\r\n\t", sizeof("\r\n\t]\r\n\t") - 1);
            return *this;
        }

        LogStream& operator <<(const char* cstr)
        {
            if (log_data_)
            {
                if (cstr)
                {
                    write_buffer(cstr, (int)strlen(cstr));
                }
                else
                {
                    write_buffer("null", 4);
                }
            }
            return *this;
        }
        LogStream& operator <<(const void* ptr)
        {
            write_pointer(ptr);
            return *this;
        }
        
        LogStream& operator <<(std::nullptr_t) 
        {
            return write_pointer(nullptr);
        }

        LogStream& operator << (char ch) { return write_buffer(&ch, 1);}
        LogStream & operator << (unsigned char ch) { return (*this << (unsigned long long)ch); }

        LogStream& operator << (bool val) { return (val ? write_buffer("true", 4) : write_buffer("false", 5)); }

        LogStream & operator << (short val) { return (*this << (long long)val); }
        LogStream & operator << (unsigned short val) { return (*this << (unsigned long long)val); }
        LogStream & operator << (int val) { return (*this << (long long)val); }
        LogStream & operator << (unsigned int val) { return (*this << (unsigned long long)val); }
        LogStream & operator << (long val) { return (*this << (long long)val); }
        LogStream & operator << (unsigned long val) { return (*this << (unsigned long long)val); }
        
        LogStream& operator << (long long integer){ return write_longlong<0>(integer);}

        LogStream& operator << (unsigned long long integer){return write_ulonglong<0>(integer);}

        LogStream& operator << (float f)
        {
            if (log_data_ && log_data_->content_len_ + 30 <= LogData::LOG_SIZE)
            {
                log_data_->content_len_ += write_float_unsafe(log_data_->content_ + log_data_->content_len_, f);
            }
            return *this;
        }
        LogStream& operator << (double df)
        {
            if (log_data_ && log_data_->content_len_ + 30 <= LogData::LOG_SIZE)
            {
                log_data_->content_len_ += write_double_unsafe(log_data_->content_ + log_data_->content_len_, df);
            }
            return *this;
        }


        template<class _Ty1, class _Ty2>
        inline LogStream & operator <<(const std::pair<_Ty1, _Ty2> & val){ return *this << '<' <<val.first << ':' << val.second << '>'; }

        template<class Container>
        LogStream& write_container(const Container& container, const char* name, int len)
        {
            write_buffer(name, len);
            write_buffer("(", 1);
            *this << container.size();
            write_buffer(")[", 2);
            int input_count = 0;
            for (auto iter = container.begin(); iter != container.end(); iter++)
            {
                if (input_count >= MAX_CONTAINER_DEPTH)
                {
                    *this << "..., ";
                    break;
                }
                if(input_count > 0)
                {
                    *this << ", ";
                }
                *this << *iter;
                input_count++;
            }
            return *this << "]";
        }

        template<class _Elem, class _Alloc>
        LogStream & operator <<(const std::vector<_Elem, _Alloc> & val) { return write_container(val, "vector:", sizeof("vector:") - 1);}
        template<class _Elem, class _Alloc>
        LogStream & operator <<(const std::list<_Elem, _Alloc> & val) { return write_container(val, "list:", sizeof("list:") - 1);}
        template<class _Elem, class _Alloc>
        LogStream & operator <<(const std::deque<_Elem, _Alloc> & val) { return write_container(val, "deque:", sizeof("deque:") - 1);}
        template<class _Key, class _Tp, class _Compare, class _Allocator>
        LogStream & operator <<(const std::map<_Key, _Tp, _Compare, _Allocator> & val) { return write_container(val, "map:", sizeof("map:") - 1);}
        template<class _Key, class _Compare, class _Allocator>
        LogStream & operator <<(const std::set<_Key, _Compare, _Allocator> & val) { return write_container(val, "set:", sizeof("set:") - 1);}
        template<class _Key, class _Tp, class _Hash, class _Compare, class _Allocator>
        LogStream& operator <<(const std::unordered_map<_Key, _Tp, _Hash, _Compare, _Allocator>& val)
        {return write_container(val, "unordered_map:", sizeof("unordered_map:") - 1);}
        template<class _Key, class _Hash, class _Compare, class _Allocator>
        LogStream& operator <<(const std::unordered_set<_Key, _Hash, _Compare, _Allocator> & val)
        {return write_container(val, "unordered_set:", sizeof("unordered_set:") - 1);}
        template<class _Traits, class _Allocator>
        LogStream & operator <<(const std::basic_string<char, _Traits, _Allocator> & str) { return write_buffer(str.c_str(), (int)str.length());}
        
    public:
        LogData * log_data_ = nullptr;
        Logger* logger_ = nullptr;
        int hold_idx_ = -1;//ring buffer  
    };
}


#endif
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
#ifndef _FN_LOG_MACRO_H_
#define _FN_LOG_MACRO_H_


namespace FNLog
{

    inline Logger& GetDefaultLogger()
    {
        static Logger logger;
        return logger;
    }

    inline int LoadAndStartDefaultLogger(const std::string& config_path)
    {
        int ret = LoadAndStartLogger(GetDefaultLogger(), config_path);
        if (ret != 0)
        {
            printf("load auto start default logger error. ret:<%d>.\n", ret);
            return ret;
        }
        return 0;
    }

    inline int FastStartDefaultLogger(const std::string& config_text)
    {
        int ret = ParseAndStartLogger(GetDefaultLogger(), config_text);
        if (ret != 0)
        {
            printf("fast start default logger error. ret:<%d>.\n", ret);
            return ret;
        }
        return 0;
    }


    inline int FastStartDefaultLogger()
    {
        static const std::string default_config_text =
            R"----(
 # default channel 0
   # write full log to pname.log 
   # write error log to pname_error.log 
   # view  info log to screen 
 # sync channel 1 
   # write full log to pname.log
   # write info log to pname_info.log
   # view  info log to screen 

 - channel: 0
    sync: async
    -device: 0
        disable: false
        out_type: file
        file: "$PNAME"
        rollback: 4
        limit_size: 100 m #only support M byte
    -device: 1
        disable: false
        out_type: file
        priority: error
        file: "$PNAME_error"
        rollback: 4
        limit_size: 100 m #only support M byte
    -device:2
        disable: false
        out_type: screen
        priority: info
 - channel: 1
    sync: sync
    -device: 0
        disable: false
        out_type: file
        file: "$PNAME_sync"
        rollback: 4
        limit_size: 100 m #only support M byte
    -device: 1
        disable: false
        out_type: file
        priority: info
        file: "$PNAME_sync_info"
        rollback: 4
        limit_size: 100 m #only support M byte
    -device:2
        disable: false
        out_type: screen
        priority: info 
)----";
        return FastStartDefaultLogger(default_config_text);
    }

    inline int FastStartDebugLogger()
    {
        int ret = FastStartDefaultLogger();
        if (ret == 0)
        {
            BatchSetDeviceConfig(GetDefaultLogger(), DEVICE_OUT_FILE, DEVICE_CFG_PRIORITY, PRIORITY_TRACE);
            SetDeviceConfig(GetDefaultLogger(), 0, 1, DEVICE_CFG_PRIORITY, PRIORITY_ERROR); //error file is still error file    
            BatchSetDeviceConfig(GetDefaultLogger(), DEVICE_OUT_SCREEN, DEVICE_CFG_PRIORITY, PRIORITY_DEBUG);
        }
        return ret;
    }
}

//--------------------BASE STREAM MACRO ---------------------------

#define LOG_STREAM_ORIGIN(logger, channel, priority, category, prefix) \
FNLog::LogStream(logger, channel, priority, category, \
__FILE__, sizeof(__FILE__) - 1, \
__LINE__, __FUNCTION__, sizeof(__FUNCTION__) -1, prefix)

#define LOG_STREAM_DEFAULT_LOGGER(channel, priority, category, prefix) \
    LOG_STREAM_ORIGIN(FNLog::GetDefaultLogger(), channel, priority, category, prefix)

#define LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel, priority, category) \
    LOG_STREAM_DEFAULT_LOGGER(channel, priority, category, FNLog::LOG_PREFIX_ALL)


//--------------------CPP STREAM STYLE FORMAT ---------------------------
#define LogTraceStream(channel_id, category) LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_TRACE, category)
#define LogDebugStream(channel_id, category) LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_DEBUG, category)
#define LogInfoStream(channel_id,  category) LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_INFO,  category)
#define LogWarnStream(channel_id,  category) LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_WARN,  category)
#define LogErrorStream(channel_id, category) LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_ERROR, category)
#define LogAlarmStream(channel_id, category) LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_ALARM, category)
#define LogFatalStream(channel_id, category) LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_FATAL, category)

#define LogTrace() LogTraceStream(0, 0)
#define LogDebug() LogDebugStream(0, 0)
#define LogInfo()  LogInfoStream(0, 0)
#define LogWarn()  LogWarnStream(0, 0)
#define LogError() LogErrorStream(0, 0)
#define LogAlarm() LogAlarmStream(0, 0)
#define LogFatal() LogFatalStream(0, 0)


//--------------------CPP TEMPLATE STYLE FORMAT ---------------------------
inline FNLog::LogStream& LogTemplatePack(FNLog::LogStream&& ls)
{
    return ls;
}
template<typename ... Args>
FNLog::LogStream& LogTemplatePack(FNLog::LogStream&& ls, Args&& ... args)
{
    char buff[] = { (ls << args, '\0') ... };
    (void)buff;
    return ls;
}

#define LogTracePack(channel_id, category, ...)  LogTemplatePack(LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_TRACE, category), ##__VA_ARGS__)
#define LogDebugPack(channel_id, category, ...)  LogTemplatePack(LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_DEBUG, category), ##__VA_ARGS__)
#define LogInfoPack(channel_id,  category, ...)  LogTemplatePack(LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_INFO,  category), ##__VA_ARGS__)
#define LogWarnPack(channel_id,  category, ...)  LogTemplatePack(LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_WARN,  category), ##__VA_ARGS__)
#define LogErrorPack(channel_id, category, ...)  LogTemplatePack(LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_ERROR, category), ##__VA_ARGS__)
#define LogAlarmPack(channel_id, category, ...)  LogTemplatePack(LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_ALARM, category), ##__VA_ARGS__)
#define LogFatalPack(channel_id, category, ...)  LogTemplatePack(LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_FATAL, category), ##__VA_ARGS__)

#define PackTrace(...) LogTracePack(0, 0, ##__VA_ARGS__)
#define PackDebug(...) LogDebugPack(0, 0, ##__VA_ARGS__)
#define PackInfo( ...) LogInfoPack( 0, 0, ##__VA_ARGS__)
#define PackWarn( ...) LogWarnPack( 0, 0, ##__VA_ARGS__)
#define PackError(...) LogErrorPack(0, 0, ##__VA_ARGS__)
#define PackAlarm(...) LogAlarmPack(0, 0, ##__VA_ARGS__)
#define PackFatal(...) LogFatalPack(0, 0, ##__VA_ARGS__)


//--------------------CPP MACRO STREAM STYLE FORMAT ---------------------------

#define LOG_TRACE(channel_id, category, log) LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_TRACE, category) << log
#define LOG_DEBUG(channel_id, category, log) LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_DEBUG, category) << log
#define LOG_INFO(channel_id,  category, log) LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_INFO,  category) << log
#define LOG_WARN(channel_id,  category, log) LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_WARN,  category) << log
#define LOG_ERROR(channel_id, category, log) LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_ERROR, category) << log
#define LOG_ALARM(channel_id, category, log) LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_ALARM, category) << log
#define LOG_FATAL(channel_id, category, log) LOG_STREAM_DEFAULT_LOGGER_WITH_PREFIX(channel_id, FNLog::PRIORITY_FATAL, category) << log

#define LOGT(log) LOG_TRACE(0, 0, log)
#define LOGD(log) LOG_DEBUG(0, 0, log)
#define LOGI(log) LOG_INFO(0, 0, log)
#define LOGW(log) LOG_WARN(0, 0, log)
#define LOGE(log) LOG_ERROR(0, 0, log)
#define LOGA(log) LOG_ALARM(0, 0, log)
#define LOGF(log) LOG_FATAL(0, 0, log)


//--------------------C STYLE FORMAT ---------------------------
#ifdef WIN32
#define LOG_FORMAT(channel_id, priority, category, prefix, logformat, ...) \
do{ \
    if (FNLog::FastCheckPriorityPass(FNLog::GetDefaultLogger(), channel_id, priority, category))  \
    { \
        break;   \
    } \
    FNLog::LogStream __log_stream(LOG_STREAM_DEFAULT_LOGGER(channel_id, priority, category, prefix));\
    if (__log_stream.log_data_)\
    {\
        int __log_len = _snprintf_s(__log_stream.log_data_ ->content_ + __log_stream.log_data_ ->content_len_, FNLog::LogData::LOG_SIZE - __log_stream.log_data_ ->content_len_, _TRUNCATE, logformat, ##__VA_ARGS__); \
        if (__log_len < 0) __log_len = 0; \
        __log_stream.log_data_ ->content_len_ += __log_len; \
    }\
} while (0)
#else
#define LOG_FORMAT(channel_id, priority, category, prefix, logformat, ...) \
do{ \
    if (FNLog::FastCheckPriorityPass(FNLog::GetDefaultLogger(), channel_id, priority, category))  \
    { \
        break;   \
    } \
    FNLog::LogStream __log_stream(LOG_STREAM_DEFAULT_LOGGER(channel_id, priority, category, prefix));\
    if (__log_stream.log_data_)\
    {\
        int __log_len = snprintf(__log_stream.log_data_ ->content_ + __log_stream.log_data_ ->content_len_, FNLog::LogData::LOG_SIZE - __log_stream.log_data_ ->content_len_, logformat, ##__VA_ARGS__); \
        if (__log_len < 0) __log_len = 0; \
        __log_stream.log_data_ ->content_len_ += __log_len; \
    }\
} while (0)
#endif

#define LOGFMT_TRACE(channel_id, category, fmt, ...)  LOG_FORMAT(channel_id, FNLog::PRIORITY_TRACE, category, FNLog::LOG_PREFIX_ALL, fmt, ##__VA_ARGS__)
#define LOGFMT_DEBUG(channel_id, category, fmt, ...)  LOG_FORMAT(channel_id, FNLog::PRIORITY_DEBUG, category, FNLog::LOG_PREFIX_ALL, fmt, ##__VA_ARGS__)
#define LOGFMT_INFO( channel_id, category, fmt, ...)  LOG_FORMAT(channel_id, FNLog::PRIORITY_INFO,  category, FNLog::LOG_PREFIX_ALL, fmt, ##__VA_ARGS__)
#define LOGFMT_WARN( channel_id, category, fmt, ...)  LOG_FORMAT(channel_id, FNLog::PRIORITY_WARN,  category, FNLog::LOG_PREFIX_ALL, fmt, ##__VA_ARGS__)
#define LOGFMT_ERROR(channel_id, category, fmt, ...)  LOG_FORMAT(channel_id, FNLog::PRIORITY_ERROR, category, FNLog::LOG_PREFIX_ALL, fmt, ##__VA_ARGS__)
#define LOGFMT_ALARM(channel_id, category, fmt, ...)  LOG_FORMAT(channel_id, FNLog::PRIORITY_ALARM, category, FNLog::LOG_PREFIX_ALL, fmt, ##__VA_ARGS__)
#define LOGFMT_FATAL(channel_id, category, fmt, ...)  LOG_FORMAT(channel_id, FNLog::PRIORITY_FATAL, category, FNLog::LOG_PREFIX_ALL, fmt, ##__VA_ARGS__)
#define LOGFMTT(fmt, ...) LOGFMT_TRACE(0, 0, fmt,  ##__VA_ARGS__)
#define LOGFMTD(fmt, ...) LOGFMT_DEBUG(0, 0, fmt,  ##__VA_ARGS__)
#define LOGFMTI(fmt, ...) LOGFMT_INFO( 0, 0, fmt,  ##__VA_ARGS__)
#define LOGFMTW(fmt, ...) LOGFMT_WARN( 0, 0, fmt,  ##__VA_ARGS__)
#define LOGFMTE(fmt, ...) LOGFMT_ERROR(0, 0, fmt,  ##__VA_ARGS__)
#define LOGFMTA(fmt, ...) LOGFMT_ALARM(0, 0, fmt,  ##__VA_ARGS__)
#define LOGFMTF(fmt, ...) LOGFMT_FATAL(0, 0, fmt,  ##__VA_ARGS__)


#endif
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
#ifndef _FN_LOG_LOG_H_
#define _FN_LOG_LOG_H_


namespace FNLog   
{

    //inline void EnableAllChannel(Logger& logger, bool enable);
    inline void EnableAllFileDevice(Logger& logger, bool enable) { BatchSetDeviceConfig(logger, DEVICE_OUT_FILE, DEVICE_CFG_ABLE, enable); }
    inline void EnableAllScreenDevice(Logger& logger, bool enable) { BatchSetDeviceConfig(logger, DEVICE_OUT_SCREEN, DEVICE_CFG_ABLE, enable); }
    inline void EnableAllUDPDevice(Logger& logger, bool enable) { BatchSetDeviceConfig(logger, DEVICE_OUT_UDP, DEVICE_CFG_ABLE, enable); }

    inline void SetAllChannelPriority(Logger& logger, LogPriority priority) { BatchSetChannelConfig(logger, CHANNEL_CFG_PRIORITY, priority); }
    inline void SetAllFilePriority(Logger& logger, LogPriority priority) { BatchSetDeviceConfig(logger, DEVICE_OUT_FILE, DEVICE_CFG_PRIORITY, priority); }
    inline void SetAllScreenPriority(Logger& logger, LogPriority priority) { BatchSetDeviceConfig(logger, DEVICE_OUT_SCREEN, DEVICE_CFG_PRIORITY, priority); }
    inline void SetAllUDPPriority(Logger& logger, LogPriority priority) { BatchSetDeviceConfig(logger, DEVICE_OUT_UDP, DEVICE_CFG_PRIORITY, priority); }

#define BatchSetChannelCategoryMacro(begin, count) \
    BatchSetChannelConfig(logger, CHANNEL_CFG_CATEGORY, begin);\
    BatchSetChannelConfig(logger, CHANNEL_CFG_CATEGORY_EXTEND, count);
#define BatchSetDeviceCategoryMacro(out_type, begin, count) \
    BatchSetDeviceConfig(logger, out_type, DEVICE_CFG_CATEGORY, begin); \
    BatchSetDeviceConfig(logger, out_type, DEVICE_CFG_CATEGORY_EXTEND, count);


    inline void SetAllChannelCategory(Logger& logger, int begin, int count) { BatchSetChannelCategoryMacro(begin, count);}
    inline void SetAllFilePriority(Logger& logger, int begin, int count) { BatchSetDeviceCategoryMacro(DEVICE_OUT_FILE, begin, count); }
    inline void SetAllScreenCategory(Logger& logger, int begin, int count) { BatchSetDeviceCategoryMacro(DEVICE_OUT_SCREEN, begin, count); }
    inline void SetAllUDPCategory(Logger& logger, int begin, int count) { BatchSetDeviceCategoryMacro(DEVICE_OUT_UDP, begin, count); }

    inline void SetAllFileLimitSize(Logger& logger, int limit) { BatchSetDeviceConfig(logger, DEVICE_OUT_FILE, DEVICE_CFG_FILE_LIMIT_SIZE, limit); }
    inline void SetAllFileRollbackCount(Logger& logger, int rb_count) { BatchSetDeviceConfig(logger, DEVICE_OUT_FILE, DEVICE_CFG_FILE_ROLLBACK, rb_count); }

}

#endif
