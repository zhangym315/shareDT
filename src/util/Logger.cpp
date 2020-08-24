
#if defined(WIN32) || defined(_WIN32)
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif

#include "Logger.h"

#include <stdarg.h>
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>

const char* Priority_To_String[] =
{
    "DEBUG",
    "STATE",
    "INFO",
    "WARNING",
    "ERROR",
    "INVALID"
};

void LoggerStream::_finish()
{
    LOGGER.log2 (_p, "%s", _logp.c_str());
}

LoggerStream::~LoggerStream ()
{
    _finish();
}

Logger::Logger()
    : _shutdown(false)
{
    _thread = std::thread(&Logger::run, this);
}

Logger& Logger::instance()
{
    static Logger s_logger;
    return s_logger;
}

Logger::~Logger()
{
    _shutdown = true;
    _cond.notify_all();

    _thread.join();
}

const void Logger::setLogFile(const char *pathname)
{
    _ofs.open(pathname, std::ios_base::app);
    if (_ofs.fail())
    {
        std::cerr << "Failed to open logfile." << std::endl;
    }
}

void Logger::log(Priority priority, const char* __file, const char* __func, int __line, const char *fmt, ...)
{
    char buf[2048] = {0};

    sprintf(buf, "[%s][%s:%s:%d] ", Priority_To_String[priority],  __file, __func, __line);
    va_list args;
    va_start(args, fmt);
    vsprintf(buf + strlen(buf), fmt, args);
    va_end(args);

    String entry(buf);
    std::unique_lock<std::mutex> lock(_mutex);
    _queue.push(std::move(entry));
    lock.unlock();

    _cond.notify_all();
}

void Logger::log2(Priority priority, const char *fmt, ...)
{
    char buf[4096] = { 0 };

    if(priority != LOG_NOPREFIX)
    {
        sprintf(buf, "[%s] [%s] ", Timestamp::localtime().c_str(), Priority_To_String[priority]);
    }
    va_list args;
    va_start(args, fmt);
    vsprintf(buf + strlen(buf), fmt, args);
    va_end(args);

    String entry(buf);
    std::unique_lock<std::mutex> lock(_mutex);
    _queue.push(std::move(entry));
    lock.unlock();

    _cond.notify_all();
}

void Logger::vaPrint(Priority priority, const char * fmt, va_list args) {
    char buf[40960] = { 0 };

    if(priority != LOG_NOPREFIX)
    {
        sprintf(buf, "[%s] [%s] ", Timestamp::localtime().c_str(), Priority_To_String[priority]);
    }

    vsprintf(buf + strlen(buf), fmt, args);
    String entry(buf);
    std::unique_lock<std::mutex> lock(_mutex);
    _queue.push(std::move(entry));
    lock.unlock();

    _cond.notify_all();
}

void Logger::run()
{
    std::unique_lock<std::mutex> lock(_mutex);

    /* write out log event it is shutting down */
    while(!_shutdown ||  !_queue.empty()) {
        if(!_queue.empty())
        {
            if(_ofs.is_open() && (!_shutdown))
            {
                _ofs << _queue.front () << std::endl;
                _ofs.flush();
            }
            else
                std::cout << _queue.front() << std::endl;
            _queue.pop();
        }
        else
        {
            _cond.wait(lock);
        }
    }
}

LoggerStream Logger::debug() { return LoggerStream(LOG_DEBUG);}
LoggerStream Logger::state() { return LoggerStream(LOG_STATE);}
LoggerStream Logger::info()  { return LoggerStream(LOG_INFO);}
LoggerStream Logger::warn()  { return LoggerStream(LOG_WARNING);}
LoggerStream Logger::error() { return LoggerStream(LOG_ERROR);}
LoggerStream Logger::noPre() { return LoggerStream(LOG_NOPREFIX);}

void Logger::debug(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vaPrint(LOG_DEBUG, fmt, args);
    va_end(args);
}

void Logger::state(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vaPrint(LOG_STATE, fmt, args);
    va_end(args);
}

void Logger::info(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vaPrint(LOG_INFO, fmt, args);
    va_end(args);
}

void Logger::warn(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vaPrint(LOG_WARNING, fmt, args);
    va_end(args);
}

void Logger::error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vaPrint(LOG_ERROR, fmt, args);
    va_end(args);
}

void Logger::noPre(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vaPrint(LOG_NOPREFIX, fmt, args);
    va_end(args);
}

String Timestamp::localtime()
{
    std::ostringstream stream;
    auto now = std::chrono::system_clock::now();
    time_t tt = std::chrono::system_clock::to_time_t(now);
    auto seconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
    auto fraction = now - seconds;
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(fraction);

#if defined(WIN32) || defined(_WIN32)
    struct tm tm;
    localtime_s(&tm, &tt);
    stream << std::put_time(&tm, "%F %T.");
#else
    char buffer[200] = {0};
    String timeString;
    std::strftime(buffer, 200, "%F %T.", std::localtime(&tt));
    stream << buffer;
#endif

    stream << std::setfill('0') << std::setw(3) << milliseconds.count();
    return stream.str();
}
