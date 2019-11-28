/**
 * @file multithreading.hpp
 * @brief Provide functionality such as threa
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
    extern boost::asio::io_service *io_service;
    extern boost::thread_group *default_threadpool;
    extern boost::asio::io_service::work *io_work;

    void init_threads(unsigned num_threads = 0);

    void quit_threads();
}

#endif //__HEPHAESTUS_MULTITHREADING_HPP
