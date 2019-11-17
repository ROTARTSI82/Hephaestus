//
// Created by 25granty on 11/16/19.
//

#include "hp/profiling.hpp"

namespace hp {

    profiler_session::profiler_session(const char *name, const char *output_file) : name(name), file(output_file),
                                                                                    closed(false),
                                                                                    first_event_written(false) {
        out.open(output_file);

        std::string strname = std::string(name);
        std::replace(strname.begin(), strname.end(), '\"', '\'');

        out << fmt::format(
                R"({{"name": "{0}", "compile-datetime": "{1} at {2}", "current-datetime": "{3}", "traceEvents":[)",
                name, __DATE__, __TIME__, current_datetime());

        out.flush();

        if (!out.is_open()) {
            // LOG
        }
    }

    profiler_session::~profiler_session() {
        flush_all();
        close();
    }

    profiler_session &profiler_session::operator=(profiler_session &&mv) noexcept {
        if (&mv == this) {  // Self assignment
            return *this;
        }

        this->close();  // Close current thingy.
        out = std::move(mv.out);
        closed = mv.closed;
        name = mv.name;
        file = mv.file;
        first_event_written = mv.first_event_written;

        return *this;
    }

    profiler_session::profiler_session(profiler_session &&mv) noexcept {
        out = std::move(mv.out);
        closed = mv.closed;
        name = mv.name;
        file = mv.file;
        first_event_written = mv.first_event_written;
    }

    profiler profiler_session::new_profiler(const char *pname) {
        if (!closed) {
            return profiler(pname, this);
        } else {
            return profiler();
        }
    }

    profiler *profiler_session::new_heap_profiler(const char *pname) {
        if (!closed) {
            return new profiler(pname, this);
        } else {
            return nullptr;
        }
    }

    void profiler_session::close() {
        std::lock_guard<std::mutex> lg(mtx);
        if (!closed) {
            out << "]}";
            out.flush();
            out.close();
            closed = true;
        }
    }

    profiler_session::profiler_session() : closed(true), name("Dummy Session"), file("dummy_file.json"),
                                           first_event_written(true) {}

    void profiler_session::flush_single() {
        std::lock_guard<std::mutex> lg(mtx);
        if (closed || queue.empty()) {
            return;
        }

        profile_result head = queue.front();
        queue.pop();

        if (first_event_written) {
            out << ", ";
        } else {
            first_event_written = true;
        }

        std::string strname = std::string(head.name);
        std::replace(strname.begin(), strname.end(), '"', '\'');

        out << fmt::format(
                R"({{"cat": "function", "dur": {}, "name": "{}", "ph": "X", "pid": {}, "tid": {}, "ts": {}}})",
                head.end - head.start, strname, head.pid, head.thread, head.start);

        out.flush();

    }

    void profiler_session::flush_all() {
        std::lock_guard<std::mutex> lg(mtx);
        if (closed || queue.empty()) {
            return;
        }

        while (!queue.empty()) {
            profile_result head = queue.front();
            queue.pop();

            if (first_event_written) {
                out << ", ";
            } else {
                first_event_written = true;
            }

            std::string strname = std::string(head.name);
            std::replace(strname.begin(), strname.end(), '"', '\'');

            out << fmt::format(
                    R"({{"cat": "function", "dur": {}, "name": "{}", "ph": "X", "pid": {}, "tid": {}, "ts": {}}})",
                    head.end - head.start, strname, head.pid, head.thread, head.start);

            out.flush();
        }
    }

    profiler::~profiler() {
        stop();
    }

    profiler::profiler() : name("Dummy Profile"), parent(nullptr), stopped(true) {}

    profiler::profiler(const profiler &&other) noexcept {
        name = other.name;
        parent = other.parent;
        stopped = other.stopped;
    }

    profiler &profiler::operator=(profiler &&other) noexcept {
        if (&other == this) { // Self-assignment
            return *this;
        }

        this->stop();
        name = other.name;
        parent = other.parent;
        stopped = other.stopped;

        return *this;
    }

    void profiler::stop() {
        if (parent == nullptr) {  // Most likely in this case our profiler was a dummy.
            HP_DEBUG(
                    "Profiler \"{}\" stopped with null parent! (This is not an error in most cases, as destroying dummy profilers would have this effect)",
                    name);
            return;
        }


        std::lock_guard<std::mutex> lg(parent->mtx);
        if (!stopped) {
            stopped = true;
            auto end = std::chrono::high_resolution_clock::now();
            profile_result res = profile_result();
            res.name = name;
            res.start = std::chrono::time_point_cast<std::chrono::microseconds>(start_time).time_since_epoch().count();
            res.end = std::chrono::time_point_cast<std::chrono::microseconds>(end).time_since_epoch().count();
            res.thread = std::hash<std::thread::id>{}(std::this_thread::get_id());
            res.pid = ::getpid();

            parent->queue.push(res);
        }
    }

    void profiler::start() {
        if (parent == nullptr) {  // Cannot start because the profiler has nothing to report to!
            HP_WARN("Attempted to start profiler \"{}\" with a null parent! Ignoring call to profiler::start()!", name);
            return;
        }


        std::lock_guard<std::mutex> lg(parent->mtx);
        if (stopped) {
            start_time = std::chrono::high_resolution_clock::now();
            stopped = false;
        }
    }

    profiler::profiler(const char *name, profiler_session *par) : name(name), parent(par), stopped(true) {}

    profiler_session *profiler_session::default_session = nullptr;
}