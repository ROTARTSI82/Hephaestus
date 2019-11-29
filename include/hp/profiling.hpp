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

        /**
         * @fn profiler(const profiler &other) = delete
         * @note The copy constructor is deleted. You *MUST* use the `profiler(profiler &&) noexcept` move
         *       constructor instead.
         */
        profiler(const profiler &other) = delete;

        /**
         * @fn profiler &operator=(const profiler &other) = delete
         * @note The copy assignment operator is deleted. You *MUST* use the `profiler &operator=(profiler &&) noexcept`
         *       move assignment operator instead.
         */
        profiler &operator=(const profiler &other) = delete;

        /**
         * @fn profiler(profiler &&other) noexcept
         * @brief Standard move constructor.
         * @param other The profiler to move. (RHS of the move)
         */
        profiler(profiler &&other) noexcept;

        /**
         * @fn profiler &operator=(profiler &&other) noexcept
         * @brief Standard move assignment operator.
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

        explicit profiler(const char *name, profiler_session *par); ///< @private

        const char *name; ///< @private
        std::chrono::time_point<std::chrono::high_resolution_clock> start_time; ///< @private
        profiler_session *parent; ///< @private
        bool stopped; ///< @private
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
         *          Example Usage: `hp::profiler_session::default_session = new profiler_session("Profiling Session");` would set the default profiler session for use in `HP_START_PROFILER`
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

        /**
         * @fn profiler_session &operator=(const profiler_session &eq) = delete;
         * @note The standard copy assignment operator is deleted. You *MUST* use the
         *       `profiler_session &operator=(profiler_session &&) noexcept` move assignment operator.
         */
        profiler_session &operator=(const profiler_session &eq) = delete;

        /**
         * @fn profiler_session(const profiler_session &cpy) = delete
         * @note The standard copy constructor is deleted. You *MUST* use the
         *       `profiler_session(profiler_session &&) noexcept` standard move constructor.
         */
        profiler_session(const profiler_session &cpy) = delete;

        /**
         * @fn profiler_session()
         * @brief Standard default constructor. Initializes with dummy values.
         */
        profiler_session();

        /**
         * @fn profiler_session &operator=(profiler_session &&mv) noexcept
         * @brief Standard move assignment operator.
         * @param mv RHS of the move assignment
         * @return Forwards the `mv` parameter.
         */
        profiler_session &operator=(profiler_session &&mv) noexcept;

        /**
         * @fn profiler_session(profiler_session &&mv) noexcept
         * @brief Standard move constructor.
         * @param mv RHS of the move construction.
         */
        profiler_session(profiler_session &&mv) noexcept;

        /**
         * @fn profiler new_profiler(const char *pname = "Unnamed Profiler")
         * @brief Construct and retrieve a new `hp::profiler`.
         * @param pname Name of the profiler.
         * @return Returns the newly constructed profiler.
         */
        profiler new_profiler(const char *pname = "Unnamed Profiler");

        /**
         * @fn profiler *new_dynamic_profiler(const char *pname = "Unnamed Profiler")
         * @brief Construct and retrieve a new dynamically allocated `hp::profiler`. (Allocated with `new`)
         * @param pname Name of the profiler.
         * @return Pointer to the newly constructed profiler.
         */
        profiler *new_dynamic_profiler(const char *pname = "Unnamed Profiler");

        /**
         * @fn void close()
         * @brief Closes the file.
         * @note This function does not finish writing queued reports, so reports in queue WILL NOT be written!
         *       Best practice is to call `hp::profiler_session::flush_all()` before calling this function.
         */
        void close();

        /**
         * @fn void flush_single()
         * @brief Write a SINGLE profile report to the file
         * @note See `void flush_all()` if you wish to write ALL queued profile reports.
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

        std::mutex mtx; ///< @private
        std::queue<profile_result> queue; ///< @private
        const char *file; ///< @private
        const char *name; ///< @private
        std::ofstream out; ///< @private
        bool closed; ///< @private

        bool first_event_written; ///< @private
    };
}

#ifdef HP_PROFILING_ENABLED
/**
 * @def HP_START_PROFILER
 * @brief Create a profiler from the default session and start timing. Will stop when scope exits.
 * @details Useful for timing function calls and sections of code. You can simply create a scope and put
 *          `HP_START_PROFILER` at the beginning to time the execution time of the code within that scope.
 */
#define HP_START_PROFILER ::hp::profiler __hp_tmp_profiler_obj; if (::hp::profiler_session::default_session != nullptr) {__hp_tmp_profiler_obj = ::std::move(::hp::profiler_session::default_session->new_profiler(BOOST_CURRENT_FUNCTION)); __hp_tmp_profiler_obj.start();};
#else

/**
 * @def HP_START_PROFILER
 * @brief Create a profiler from the default session and start timing. Will stop when scope exits.
 * @details Useful for timing function calls and sections of code. You can simply create a scope and put
 *          `HP_START_PROFILER` at the beginning to time the execution time of the code within that scope.
 */
#define HP_START_PROFILER
#endif

#endif //__HEPHAESTUS_PROFILING_HPP
