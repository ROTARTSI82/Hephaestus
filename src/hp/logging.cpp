//
// Created by 25granty on 11/17/19.
//

#include "hp/logging.hpp"

namespace hp {
    std::string current_datetime() {
        std::ostringstream oss;
        time_t rawtime;
        tm timeinfo{};
        localtime_s(&timeinfo, &rawtime);

        oss << std::put_time(&timeinfo, "%Y.%m.%d %H:%M:%S");
        return oss.str();
    }

#ifdef HP_LOGGING_ENABLED

    static void on_spdlog_err(const std::string &msg) {
        std::cerr << "[** SPDLOG ERROR **]: " << msg << std::endl;
    }

    void init_logging(bool use_single_file, bool use_unique_file) {
        try {
            if (async_logging_enabled) {
                spdlog::init_thread_pool(8192, 1);
            }
            // Sink for outputting to the console
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
//            auto systemd_sink = std::make_shared<spdlog::sinks::systemd_sink_st>();
#ifdef __APPLE__
            auto syslog_sink = std::make_shared<spdlog::sinks::syslog_sink_mt>("ident", LOG_PID, LOG_USER, true);
#endif

#if defined(_WIN32)
            auto msvc_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#endif

            auto now = current_datetime();

            // Create a logger using both sinks, set it as the default logger, and register it.
            std::vector<spdlog::sink_ptr> sink_list{console_sink};

            // Sink for outputting to file
            std::shared_ptr<spdlog::sinks::basic_file_sink_mt> latest_file_sink;
            std::shared_ptr<spdlog::sinks::basic_file_sink_mt> unique_file_sink;
            if (use_single_file) {
                latest_file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("latest.log", true);
                sink_list.emplace_back(latest_file_sink);
            }

            if (use_unique_file) {
                unique_file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/" + now + ".log", true);
                sink_list.emplace_back(unique_file_sink);
            }

#if defined(_WIN32)
            sink_list.emplace_back(msvc_sink);
#endif

#ifdef __APPLE__
            sink_list.emplace_back(syslog_sink);
#endif

            std::shared_ptr<spdlog::logger> logger;
            if (async_logging_enabled) {
                logger = std::make_shared<spdlog::async_logger>("root",
                                                                sink_list.begin(), sink_list.end(),
                                                                spdlog::thread_pool(),
                                                                spdlog::async_overflow_policy::block);
            } else {
                logger = std::make_shared<spdlog::logger>("root", sink_list.begin(), sink_list.end());
            }

            spdlog::register_logger(logger);
            spdlog::set_default_logger(logger);
            spdlog::set_level(spdlog::level::trace);

            spdlog::set_pattern("%v");

            HP_INFO("{:=^150}", " [NEW LOGGING SESSION | " + now + "] ");

            spdlog::set_pattern("%^[%H:%M:%S.%e] [%l] [%P|%t] [%!|%s:%#]: %v%$");

            spdlog::set_error_handler(on_spdlog_err);

            HP_INFO("Started Hephaestus {}! Compiled on {} at {}. The current time is {}", HP_VERSION_STRING, __DATE__,
                    __TIME__, now);
        }
        catch (const spdlog::spdlog_ex &ex) {
            std::cerr << "Log initialization failed: " << ex.what() << std::endl;
        }
    }

#else
    void init_logging(bool use_single_file, bool use_unique_file) {};
#endif

}
