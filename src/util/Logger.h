#pragma once

#include "TypeDef.h"
#include <queue>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <fstream>
#include <cstring>
#include <iostream>
#include <sstream>

#define LOGGER Logger::instance()


#ifdef _DEBUG
#define LOG_DEBUG(fmt, ...) Logger::instance().log(LOG_DEBUG, __FILE__, __FUNCTION__,__LINE__, fmt, ##__VA_ARGS__)
#else
#define LOG_DEBUG(fmt, ...)
#endif
#define LOG_INFO(fmt, ...) Logger::instance().log2(LOG_INFO, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) Logger::instance().log(LOG_ERROR, __FILE__, __FUNCTION__,__LINE__, fmt, ##__VA_ARGS__)


class Timestamp
{
  public:
    Timestamp()
        : _beginTimePoint(std::chrono::high_resolution_clock::now())
    { }

    void reset()
    {
        _beginTimePoint = std::chrono::high_resolution_clock::now();
    }

    int64_t elapsed()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - _beginTimePoint).count();
    }

    static std::string localtime();

  private:
    std::chrono::time_point<std::chrono::high_resolution_clock> _beginTimePoint;
};

enum Priority {
    LOG_DEBUG, LOG_STATE, LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_INVALID, LOG_NOPREFIX
};

class LoggerStream;
class Logger
{
  public:
    Logger &operator=(const Logger &) = delete;
    static Logger& instance();
    ~Logger();

    const void setLogFile(const char *pathname);
    void log(Priority priority, const char* __file, const char* __func, int __line, const char *fmt, ...);
    void log2(Priority priority, const char *fmt, ...);

    LoggerStream debug();
    LoggerStream state();
    LoggerStream info() ;
    LoggerStream warn() ;
    LoggerStream error();
    LoggerStream noPre();

    void debug(const char *fmt, ...);
    void state(const char *fmt, ...);
    void info (const char *fmt, ...);
    void warn (const char *fmt, ...);
    void error(const char *fmt, ...);
    void noPre(const char *fmt, ...);   /* without prefix */

  private:
    Logger();
    void run();
    void vaPrint(Priority priority, const char * fmt, va_list args);

    std::atomic<bool> _shutdown;
    std::thread _thread;
    std::mutex _mutex;
    std::condition_variable _cond;
    std::queue<std::string> _queue;
    std::ofstream _ofs;
};

class LoggerStream {
  public:
    ~LoggerStream ();

    LoggerStream& operator << (const std::string & val)
    {
        _logp.append(val);
        return *this;
    }

    LoggerStream& operator << (const char * val)
    {
        _logp.append(val);
        return *this;
    }

    LoggerStream& operator << (char * val)
    {
        _logp.append(val);
        return *this;
    }

    template<typename  T>
    LoggerStream& operator << (T val)
    {
        try{
            _logp.append(std::to_string(val));
        } catch (...) {
            // no to_string defined, return INVALID log
            LOGGER.log2(LOG_ERROR, "%s not typename defined", typeid(T).name());
            LoggerStream * ls = new LoggerStream(LOG_INVALID);
            return *ls;
        }
        return *this;
    }

  private:
    LoggerStream() { }
    LoggerStream (Priority p) : _p(p) { }
    void _finish();

    std::string _logp;
    Priority _p;

    friend Logger;
};
