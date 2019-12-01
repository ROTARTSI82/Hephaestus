//
// Created by 25granty on 11/27/19.
//

#include "hp/multithreading.hpp"
#include "hp/logging.hpp"

namespace hp {
    boost::asio::io_service *io_service = nullptr;
    static boost::thread_group *default_threadpool = nullptr;
    static boost::asio::io_service::work *io_work = nullptr;

    void init_threads(unsigned num_threads) {
        if (io_service != nullptr || default_threadpool != nullptr || io_work != nullptr) {
            HP_WARN("Init threads called with thread pool active! Ignoring invocation!");
            return;
        }
        num_threads = num_threads == 0 ? (int) (std::thread::hardware_concurrency() *
                                                2)  // 2x hardware threads would work best?
                                       : num_threads;
        num_threads = num_threads == 0 ? 8 : num_threads; // Default to 8 concurrent threads

        HP_INFO("Started thread pool with {} threads!", num_threads);

        io_service = new boost::asio::io_service;
        default_threadpool = new boost::thread_group;
        io_work = new boost::asio::io_service::work(*io_service);

        for (unsigned i = 0; i < num_threads; i++) {
            default_threadpool->create_thread(boost::bind(&boost::asio::io_service::run, io_service));
        }
    }

    void quit_threads() {
        if (io_service == nullptr || default_threadpool == nullptr || io_work == nullptr) {
            HP_WARN("Quit threads called with thread pool stopped! Ignoring invocation!");
            return;
        }

        io_service->stop();
        default_threadpool->join_all();

        delete io_work;
        delete default_threadpool;
        delete io_service;

        io_work = nullptr;
        default_threadpool = nullptr;
        io_service = nullptr;
    }
}
