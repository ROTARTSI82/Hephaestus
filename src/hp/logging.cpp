//
// Created by 25granty on 11/17/19.
//

#include "hp/logging.hpp"

namespace hp {
    std::string current_datetime() {
        std::ostringstream oss;
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        oss << std::put_time(&tm, "%Y.%m.%d %H:%M:%S");
        return oss.str();
    }

#ifdef HP_LOGGING_ENABLED

    static void on_spdlog_err(const std::string &msg) {
        std::cerr << "[** SPDLOG ERROR **]: " << msg << std::endl;
    }

    void init_logging(bool use_single_file) {
        try {
            // Sink for outputting to the console
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
//            console_sink->set_level(spdlog::level::trace);

            auto now = current_datetime();

            // Sink for outputting to file
            std::shared_ptr<spdlog::sinks::basic_file_sink_mt> file_sink;
            if (!use_single_file) {
                file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/" + now + ".log", true);
            } else {
                file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("latest.log", true);
            }
//            file_sink->set_level(spdlog::level::trace);

            // Create a logger using both sinks, set it as the default logger, and register it.
            std::shared_ptr<spdlog::logger> logger = std::make_shared<spdlog::logger>("root",
                                                                                      spdlog::sinks_init_list(
                                                                                              {console_sink,
                                                                                               file_sink}));
//            logger->set_level(spdlog::level::trace);

            spdlog::register_logger(logger);
            spdlog::set_default_logger(logger);
            spdlog::set_level(spdlog::level::trace);

            spdlog::set_pattern("%v");

            HP_INFO("{:=^150}", " [NEW LOGGING SESSION | " + now + "] ");

            spdlog::set_pattern("%^[%Y.%m.%d] [%H:%M:%S.%F] [%n:%l] [%s:%#]: %v%$");

            spdlog::set_error_handler(on_spdlog_err);

            HP_INFO("Started Hephaestus {}! Compiled on {} at {}. The current time is {}", HP_VERSION_STRING, __DATE__,
                    __TIME__, now);
        }
        catch (const spdlog::spdlog_ex &ex) {
            std::cerr << "Log initialization failed: " << ex.what() << std::endl;
        }
    }

#elif
    void init_logging(bool x) {};
#endif

}
