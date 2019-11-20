//
// Created by 25granty on 11/17/19.
//

#pragma once

#ifndef __HEPHAESTUS_LOGGING_HPP
#define __HEPHAESTUS_LOGGING_HPP

#include "config.hpp"
#include "hp.hpp"

#include "boost/current_function.hpp"

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/fmt/fmt.h" // Included so any file that includes this file gets access to `fmt::format()`

#include <iostream>
#include <iomanip>
#include <sstream>

namespace hp {
    struct code_location {  // Struct containing info from macros.
        const char *file;
        const char *func;
        unsigned line;

        code_location(const char *file, const char *func, unsigned line) : file(file), func(func), line(line) {}
    };

    std::string current_datetime();

    void init_logging(bool use_single_file = true);
}

#define HP_GET_CODE_LOC ::hp::code_location(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__)

#ifdef HP_LOGGING_ENABLED

#define HP_TRACE SPDLOG_TRACE
#define HP_DEBUG SPDLOG_DEBUG
#define HP_INFO SPDLOG_INFO
#define HP_WARN SPDLOG_WARN
#define HP_FATAL SPDLOG_CRITICAL
#define HP_CRITICAL SPDLOG_CRITICAL

#elif

#define HP_TRACE(...)
#define HP_DEBUG(...)
#define HP_INFO(...)
#define HP_WARN(...)
#define HP_FATAL(...)
#define HP_CRITICAL(...)

#endif


#endif // __HEPHAESTUS_LOGGING_HPP
