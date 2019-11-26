/**
 * @file logging.hpp
 * @brief This file defines logging functionality.
 * @details This file also includes `"spdlog/fmt/fmt.h"` which enables anyone who includes this file to access `fmt::fmt()`.
 *          See https://fmt.dev/latest/index.html.
 */

#pragma once

#ifndef __HEPHAESTUS_LOGGING_HPP
/**
 * @def __HEPHAESTUS_LOGGING_HPP
 * @brief This macro is defined if `logging.hpp` is included.
 */
#define __HEPHAESTUS_LOGGING_HPP

#include "config.hpp"
#include "hp.hpp"

#include "boost/current_function.hpp"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/async.h"
#include "spdlog/fmt/fmt.h"

#include "spdlog/sinks/msvc_sink.h"
#include "spdlog/sinks/syslog_sink.h"

#include <iostream>
#include <iomanip>
#include <sstream>

namespace hp {
    /**
     * @struct code_location
     * @brief A struct containing filename, funciton name, and line number.
     * @details Useful for passing code locations around for logging purposes. The `HP_GET_CODE_LOC` macro automatically constructs
     *          a `code_location` struct containing the appropriate info.
     */
    struct code_location {  // Struct containing info from macros.
        /**
         * @var const char *file
         * @brief The name of the file this code location is in
         * @details Usually contains contents of `__FILE__` macro.
         */
        const char *file;

        /**
         * @var const char *func
         * @brief The name of the function this code location is in.
         * @details Usually contains contents of `BOOST_CURRENT_FUNCTION` macro, which aliases to the compiler/platform-specific function name macro.
         */
        const char *func;

        /**
         * @var unsigned line
         * @brief The line number this code location is on.
         * @details Usually contains contents of `__LINE__` macro.
         */
        unsigned line;


        /**
         * @fn code_location(const char *file, const char *func, unsigned line)
         *
         * @brief Constructor for code locations.
         * @details Can automatically built by the `HP_GET_CODE_LOC` macro.
         *
         * @param file The filename, see `const char *file` member. Expects `__FILE__`
         * @param func The function name, see `const char *func` member. Expects `BOOST_CURRENT_FUNCTION`
         * @param line The line number, see `unsigned line` member. Expects `__LINE__`
         */
        code_location(const char *file, const char *func, unsigned line) : file(file), func(func), line(line) {}
    };

    /**
     * @fn std::string current_datetime()
     *
     * @brief Queries the current datetime as a string.
     *
     * @return Returns datetime in the format of `"{YEAR}.{MONTH}.{DAY} {HOUR}:{MINUTE}:{SECOND}"`
     */
    std::string current_datetime();

    /**
     * @fn void init_logging(bool use_single_file = true, bool use_unique_file = false)
     *
     * @brief Initialize Hephaestus logging (which uses SPDLOG).
     * @details Call this function before logging anything, as any logs before a call to this function is lost.
     *          This function initializes SPDLOG, so see https://github.com/gabime/spdlog.
     *
     * @param use_single_file If true, `latest.log` would be created and logged to.
     * @param use_unique_file If true, a logfile with the current datetime as a name under directory `logs` would be created and logged to.
     *                        If the directory `logs` doesn't exist, it would be treated as if this parameter was set to false.
     */
    void init_logging(bool use_single_file = true, bool use_unique_file = false);
}

/**
 * @def HP_GET_CODE_LOC
 * @brief Builds the appropriate `hp::code_location` struct for where it is invoked.
 * @details See the documentation for `hp::code_location`.
 */
#define HP_GET_CODE_LOC ::hp::code_location(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__)

#ifdef HP_LOGGING_ENABLED

/**
 * @def HP_TRACE
 * @brief Logging macro aliases to `SPDLOG_TRACE`. See the [spdlog documentation](https://github.com/gabime/spdlog/wiki/1.-QuickStart#logging-macros).
 * @details This macro would be disabled (ie. `#define HP_TRACE(...)`) if `HP_LOGGING_ENABLED` is not defined.
 *          Example usage: `HP_TRACE("Logging message! Values: {}, {}, {}", 2, 3.1415, "Test");`
 */
#define HP_TRACE SPDLOG_TRACE

/**
 * @def HP_DEBUG
 * @brief Logging macro aliases to `SPDLOG_DEBUG`. See the [spdlog documentation](https://github.com/gabime/spdlog/wiki/1.-QuickStart#logging-macros).
 * @details This macro would be disabled (ie. `#define HP_DEBUG(...)`) if `HP_LOGGING_ENABLED` is not defined.
 *          Example usage: `HP_DEBUG("Logging message! Values: {}, {}, {}", 2, 3.1415, "Test");`
 */
#define HP_DEBUG SPDLOG_DEBUG

/**
 * @def HP_INFO
 * @brief Logging macro aliases to `SPDLOG_INFO`. See the [spdlog documentation](https://github.com/gabime/spdlog/wiki/1.-QuickStart#logging-macros).
 * @details This macro would be disabled (ie. `#define HP_INFO(...)`) if `HP_LOGGING_ENABLED` is not defined.
 *          Example usage: `HP_INFO("Logging message! Values: {}, {}, {}", 2, 3.1415, "Test");`
 */
#define HP_INFO SPDLOG_INFO

/**
 * @def HP_WARN
 * @brief Logging macro aliases to `SPDLOG_WARN`. See the [spdlog documentation](https://github.com/gabime/spdlog/wiki/1.-QuickStart#logging-macros).
 * @details This macro would be disabled (ie. `#define HP_WARN(...)`) if `HP_LOGGING_ENABLED` is not defined.
 *          Example usage: `HP_WARN("Logging message! Values: {}, {}, {}", 2, 3.1415, "Test");`
 */
#define HP_WARN SPDLOG_WARN

/**
 * @def HP_FATAL
 * @brief Logging macro aliases to `SPDLOG_CRITICAL`. See the [spdlog documentation](https://github.com/gabime/spdlog/wiki/1.-QuickStart#logging-macros).
 * @details This macro would be disabled (ie. `#define HP_FATAL(...)`) if `HP_LOGGING_ENABLED` is not defined.
 *          Example usage: `HP_FATAL("Logging message! Values: {}, {}, {}", 2, 3.1415, "Test");`
 */
#define HP_FATAL SPDLOG_CRITICAL

/**
 * @def HP_CRITICAL
 * @brief Logging macro aliases to `SPDLOG_CRITICAL`. See the [spdlog documentation](https://github.com/gabime/spdlog/wiki/1.-QuickStart#logging-macros).
 * @details This macro would be disabled (ie. `#define HP_CRITICAL(...)`) if `HP_LOGGING_ENABLED` is not defined.
 *          Example usage: `HP_CRITICAL("Logging message! Values: {}, {}, {}", 2, 3.1415, "Test");`
 */
#define HP_CRITICAL SPDLOG_CRITICAL

#elif

/**
 * @def HP_TRACE(...)
 * @brief Logging macro aliases to `SPDLOG_TRACE`. See the [spdlog documentation](https://github.com/gabime/spdlog/wiki/1.-QuickStart#logging-macros).
 * @details This macro would be disabled (ie. `#define HP_TRACE(...)`) if `HP_LOGGING_ENABLED` is not defined.
 *          Example usage: `HP_TRACE("Logging message! Values: {}, {}, {}", 2, 3.1415, "Test");`
 */
#define HP_TRACE(...)

/**
 * @def HP_DEBUG(...)
 * @brief Logging macro aliases to `SPDLOG_DEBUG`. See the [spdlog documentation](https://github.com/gabime/spdlog/wiki/1.-QuickStart#logging-macros).
 * @details This macro would be disabled (ie. `#define HP_DEBUG(...)`) if `HP_LOGGING_ENABLED` is not defined.
 *          Example usage: `HP_DEBUG("Logging message! Values: {}, {}, {}", 2, 3.1415, "Test");`
 */
#define HP_DEBUG(...)

/**
 * @def HP_INFO(...)
 * @brief Logging macro aliases to `SPDLOG_INFO`. See the [spdlog documentation](https://github.com/gabime/spdlog/wiki/1.-QuickStart#logging-macros).
 * @details This macro would be disabled (ie. `#define HP_INFO(...)`) if `HP_LOGGING_ENABLED` is not defined.
 *          Example usage: `HP_INFO("Logging message! Values: {}, {}, {}", 2, 3.1415, "Test");`
 */
#define HP_INFO(...)

/**
 * @def HP_WARN(...)
 * @brief Logging macro aliases to `SPDLOG_WARN`. See the [spdlog documentation](https://github.com/gabime/spdlog/wiki/1.-QuickStart#logging-macros).
 * @details This macro would be disabled (ie. `#define HP_WARN(...)`) if `HP_LOGGING_ENABLED` is not defined.
 *          Example usage: `HP_WARN("Logging message! Values: {}, {}, {}", 2, 3.1415, "Test");`
 */
#define HP_WARN(...)

/**
 * @def HP_FATAL(...)
 * @brief Logging macro aliases to `SPDLOG_CRITICAL`. See the [spdlog documentation](https://github.com/gabime/spdlog/wiki/1.-QuickStart#logging-macros).
 * @details This macro would be disabled (ie. `#define HP_FATAL(...)`) if `HP_LOGGING_ENABLED` is not defined.
 *          Example usage: `HP_FATAL("Logging message! Values: {}, {}, {}", 2, 3.1415, "Test");`
 */
#define HP_FATAL(...)

/**
 * @def HP_CRITICAL(...)
 * @brief Logging macro aliases to `SPDLOG_CRITICAL`. See the [spdlog documentation](https://github.com/gabime/spdlog/wiki/1.-QuickStart#logging-macros).
 * @details This macro would be disabled (ie. `#define HP_CRITICAL(...)`) if `HP_LOGGING_ENABLED` is not defined.
 *          Example usage: `HP_CRITICAL("Logging message! Values: {}, {}, {}", 2, 3.1415, "Test");`
 */
#define HP_CRITICAL(...)

#endif


#endif // __HEPHAESTUS_LOGGING_HPP
