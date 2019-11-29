/**
 * @file multithreading.hpp
 * @brief Provide thread pool functionality using boost.
 */

#pragma once

#ifndef __HEPHAESTUS_MULTITHREADING_HPP
/**
 * @def __HEPHAESTUS_MULTITHREADING_HPP
 * @brief This macro is defined if `multithreading.hpp` has been included.
 */
#define __HEPHAESTUS_MULTITHREADING_HPP

#include <boost/asio/io_service.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>

namespace hp {
    /**
     * @var extern boost::asio::io_service *io_service
     * @brief Interface for submitting asynchronous tasks.
     * @details Once the default thread pool has been constructed, you can submit a task using
     *           `"io_service->post"`. For example, `"io_service->post(boost::bind(printf, "Test"));"` would queue `"printf("Test")"` to be called by the threadpool. See boost documentation for more details.
     */
    extern boost::asio::io_service *io_service;

    /**
     * @var extern boost::thread_group *default_threadpool
     * @brief *IMPLEMENTATION DETAIL* The actual thread pool object. For internal use.
     * @warning DO NOT MODIFY OR USE THIS OBJECT! THIS IS PURELY FOR THE IMPLEMENTATION!
     */
    extern boost::thread_group *default_threadpool;

    /**
     * @var extern boost::asio::io_service::work *io_work
     * @brief *IMPLEMENTATION DETAIL* The controller object for `io_service`. For internal use.
     * @warning DO NOT MODIFY OR USE THIS OBJECT! THIS IS PURELY FOR THE IMPLEMENTATION!
     */
    extern boost::asio::io_service::work *io_work;

    /**
     * @fn void init_threads(unsigned num_threads = 0)
     * @brief Initialize/Construct the default thread pool.
     * @details Best practice is to call `init_threads` at program startup and `quit_threads` at exit.
     * @param num_threads The number of threads to assign to the thread pool. If set to `0`, a reasonable number
     *                    of threads would be automatically selected depending on hardware. If this automatic
     *                    selection fails, this value would default to `8`.
     */
    void init_threads(unsigned num_threads = 0);

    /**
     * @fn void quit_threads()
     * @brief Stops the default thread pool, joining all threads.
     * @details Best practice is to call `init_threads` at program startup and `quit_threads` at exit.
     */
    void quit_threads();
}

#endif //__HEPHAESTUS_MULTITHREADING_HPP
