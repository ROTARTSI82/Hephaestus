//
// Created by 25granty on 11/16/19.
//

#pragma once

#ifndef __HEPHAESTUS_PROFILER_H
#define __HEPHAESTUS_PROFILER_H

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

    struct profile_result {
    public:
        const char *name;
        long long start;
        long long end;
        size_t pid;
        size_t thread;
    };

    class profiler {
    protected:
    public:
        profiler();

        virtual ~profiler();

        profiler(const profiler &other) = delete;

        profiler &operator=(const profiler &other) = delete;

        profiler(profiler &&other) noexcept;

        profiler &operator=(profiler &&other) noexcept;

        void stop();

        void start();

    private:

        friend class profiler_session;

        explicit profiler(const char *name, profiler_session *par);

        const char *name;
        std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
        profiler_session *parent;
        bool stopped;
    };

    class profiler_session {
    public:
        /*
         * This field is to be used as a user-defined session. Usage:
         *  `hp::profiler_session::default_session = new hp::profiler_session(...)`
         */
        static profiler_session *default_session;

        explicit profiler_session(const char *name, const char *output_file = "out.json");

        virtual ~profiler_session();

        profiler_session &operator=(const profiler_session &eq) = delete;

        profiler_session(const profiler_session &cpy) = delete;

        profiler_session();

        profiler_session &operator=(profiler_session &&mv) noexcept;

        profiler_session(profiler_session &&mv) noexcept;

        profiler new_profiler(const char *pname = "Unnamed Profiler");

        profiler *new_heap_profiler(const char *pname = "Unnamed Profiler");

        void close();

        void flush_single();

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

#endif //__HEPHAESTUS_PROFILER_H
