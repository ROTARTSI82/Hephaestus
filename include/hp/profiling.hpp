/**
 * @file profiling.hpp
 * @brief Offers profiling functionality for timing operations.
 */

#pragma once

#ifndef __HEPHAESTUS_PROFILING_HPP
/**
 * @def __HEPHAESTUS_PROFILING_HPP
 * @brief This macro is defined if `profiling.hpp` has been included.
 */
#define __HEPHAESTUS_PROFILING_HPP

#include <chrono>
#include <queue>
#include <fstream>
#include <thread>
#include <iostream>
#include <zconf.h>

#include "boost/current_function.hpp"

#include "config.hpp"
#include "logging.hpp"

namespace hp {
    class profiler_session;

    /**
     * @struct profile_result
     * @brief Includes details of a profile result, such as start and end times.
     */
    struct profile_result {
    public:
        /**
         * @var const char *name
         * @brief The name of the profile this was generated from.
         */
        const char *name;

        /**
         * @var long long start
         * @brief The start time of the profile in microseconds since the epoch.
         */
        long long start;

        /**
         * @var long long end
         * @brief The end time of the profile in microseconds since the epoch.
         */
        long long end;

        /**
         * @var size_t pid
         * @brief Process id of the process the profile was taken on.
         */
        size_t pid;

        /**
         * @var size_t thread
         * @brief Thread id of the thread the profile was taken on.
         */
        size_t thread;
    };

    /**
     * @class profiler
     * @brief Object for timing operations.
     * @note Timing needs to be explicitly started with the `hp::profiler::start()` function, but timing is automatically
     *       stopped (if active) on the destruction of the profiler object. This object can be started and stopped multiple
     *       times with calls to `start()` and `stop()`.
     */
    class profiler {
    protected:
    public:
        /**
         * @fn profiler()
         * @brief Default constructor; builds a dummy profiler object.
         */
        profiler();

        /**
         * @fn virtual ~profiler()
         * @brief Destructor of profiler object, stops timing if active.
         */
        virtual ~profiler();

        profiler(const profiler &other) = delete;

        profiler &operator=(const profiler &other) = delete;

        /**
         * @fn profiler(profiler &&other) noexcept
         * @brief Move copy constructor of profilers.
         * @param other The profiler to move. (RHS of the move)
         */
        profiler(profiler &&other) noexcept;

        /**
         * @fn profiler &operator=(profiler &&other) noexcept
         * @param other The profiler to move. (RHS of the move)
         * @return Returns a reference `*this`.
         */
        profiler &operator=(profiler &&other) noexcept;

        /**
         * @fn void stop()
         * @brief Stops the timing if timing is active. Otherwise invocations are quietly ignored.
         * @details It is safe to start, stop, restart, and stop this object as many times as you'd like.
         */
        void stop();

        /**
         * @fn void start()
         * @brief Start timing if timing is not active. Otherwise invocations are quietly ignored.
         * @details It is safe to start, stop, restart, and stop this object as many times as you'd like.
         * @note This function needs to be explicitly called for timing to start! Timing is NOT automatically started when constructed!
         */
        void start();

    private:

        friend class profiler_session;

        explicit profiler(const char *name, profiler_session *par);

        const char *name;
        std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
        profiler_session *parent;
        bool stopped;
    };

    /**
     * @class profiler_session
     * @brief Profiler sessions represent files that reports are written to.
     * @details You can utilize discrete profiler sessions to output different profiles to different files.
     */
    class profiler_session {
    public:
        /**
         * @var static profiler_session *default_session
         * @brief A user-defined pointer to the session to be used by `HP_START_PROFILER`
         * @details The user should assign this variable before using `HP_START_PROFILER`.
         *          Example usage: `hp::profiler_session::default_session = new profiler_session("Profiling Session");`
         */
        static profiler_session *default_session;

        /**
         * @fn explicit profiler_session(const char *name, const char *output_file = "out.json")
         * @brief Constructor for profiler sessions.
         * @param name Name of the session. No use other than identification
         * @param output_file File to write the report to.
         */
        explicit profiler_session(const char *name, const char *output_file = "out.json");

        /**
         * @fn virtual ~profiler_session()
         * @brief Destructor of profiler session, writes and closes the report.
         * @details Simply calls `flush_all()` and then `close()`
         */
        virtual ~profiler_session();

        profiler_session &operator=(const profiler_session &eq) = delete;

        profiler_session(const profiler_session &cpy) = delete;

        profiler_session();

        profiler_session &operator=(profiler_session &&mv) noexcept;

        profiler_session(profiler_session &&mv) noexcept;

        profiler new_profiler(const char *pname = "Unnamed Profiler");

        profiler *new_heap_profiler(const char *pname = "Unnamed Profiler");

        /**
         * @fn void close()
         * @brief Closes the file.
         * @note This function does not finish writing queued reports, so reports in queue WILL NOT be written!
         *       Best practice is to call `hp::profiler_session::flush_all()` before calling this function.
         */
        void close();

        /**
         * @fn void flush_single()
         * @brief Write
         */
        void flush_single();

        /**
         * @fn void flush_all()
         * @brief Writes all queued profile reports to the file.
         * @note You MUST call this function before calling `close()`, or reports in queue will NOT be written!
         */
        void flush_all();

    private:

        friend class profiler;

        std::mutex mtx;
        std::queue<profile_result> queue;
        const char *file;
        const char *name;
        std::ofstream out;
        bool closed;

        bool first_event_written;
    };
}

#ifdef HP_PROFILING_ENABLED
#define HP_START_PROFILER ::hp::profiler __hp_tmp_profiler_obj; if (::hp::profiler_session::default_session != nullptr) {__hp_tmp_profiler_obj = ::std::move(::hp::profiler_session::default_session->new_profiler(BOOST_CURRENT_FUNCTION)); __hp_tmp_profiler_obj.start();};
#else
#define HP_START_PROFILER
#endif

#endif //__HEPHAESTUS_PROFILING_HPP
