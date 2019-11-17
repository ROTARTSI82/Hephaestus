//
// Created by 25granty on 11/17/19.
//

#pragma once

#ifndef __HEPHAESTUS_LOGGING_HPP
#define __HEPHAESTUS_LOGGING_HPP

#include "hp/config.hpp"
#include "hp/hp.hpp"

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/fmt/fmt.h" // Included so any file that includes this file gets access to `fmt::format()`

#include <iostream>
#include <iomanip>
#include <sstream>

namespace hp {
    std::string current_datetime();

    void init_logging(bool use_single_file = true);
}

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
