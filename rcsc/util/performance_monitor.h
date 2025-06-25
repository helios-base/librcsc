// -*-c++-*-

/*!
  \file performance_monitor.h
  \brief Performance monitoring utility for librcsc
*/

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

 This code is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 3 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 *EndCopyright:
 */

#ifndef RCSC_UTIL_PERFORMANCE_MONITOR_H
#define RCSC_UTIL_PERFORMANCE_MONITOR_H

#include <chrono>
#include <string>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <vector>
#include <atomic>

namespace rcsc {

/*!
  \class PerformanceMonitor
  \brief High-performance monitoring utility for profiling librcsc operations
*/
class PerformanceMonitor {
public:
    /*!
      \struct TimerData
      \brief Internal timer data structure
    */
    struct TimerData {
        std::chrono::high_resolution_clock::time_point start_time;
        std::chrono::high_resolution_clock::time_point end_time;
        std::atomic<uint64_t> call_count{0};
        std::atomic<uint64_t> total_nanoseconds{0};
        std::atomic<uint64_t> min_nanoseconds{UINT64_MAX};
        std::atomic<uint64_t> max_nanoseconds{0};
        
        TimerData() = default;
    };

    /*!
      \class ScopedTimer
      \brief RAII timer for automatic timing
    */
    class ScopedTimer {
    private:
        PerformanceMonitor& monitor_;
        std::string name_;
        std::chrono::high_resolution_clock::time_point start_time_;
        bool active_;

    public:
        ScopedTimer(PerformanceMonitor& monitor, const std::string& name)
            : monitor_(monitor), name_(name), active_(true) {
            start_time_ = std::chrono::high_resolution_clock::now();
        }

        ~ScopedTimer() {
            if (active_) {
                stop();
            }
        }

        void stop() {
            if (active_) {
                monitor_.stopTimer(name_, start_time_);
                active_ = false;
            }
        }
    };

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::unique_ptr<TimerData>> timers_;
    std::atomic<bool> enabled_{true};

public:
    PerformanceMonitor() = default;
    ~PerformanceMonitor() = default;

    // Disable copy constructor and assignment
    PerformanceMonitor(const PerformanceMonitor&) = delete;
    PerformanceMonitor& operator=(const PerformanceMonitor&) = delete;

    /*!
      \brief Enable or disable performance monitoring
      \param enable true to enable, false to disable
    */
    void setEnabled(bool enable) {
        enabled_.store(enable, std::memory_order_relaxed);
    }

    /*!
      \brief Check if monitoring is enabled
      \return true if enabled, false otherwise
    */
    bool isEnabled() const {
        return enabled_.load(std::memory_order_relaxed);
    }

    /*!
      \brief Start a timer with the given name
      \param name timer name
      \return ScopedTimer object for RAII timing
    */
    ScopedTimer startTimer(const std::string& name) {
        return ScopedTimer(*this, name);
    }

    /*!
      \brief Stop a timer and record the elapsed time
      \param name timer name
      \param start_time start time from high_resolution_clock
    */
    void stopTimer(const std::string& name, 
                   const std::chrono::high_resolution_clock::time_point& start_time) {
        if (!enabled_.load(std::memory_order_relaxed)) {
            return;
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);
        auto nanoseconds = duration.count();

        std::lock_guard<std::mutex> lock(mutex_);
        
        auto& timer_data = timers_[name];
        if (!timer_data) {
            timer_data = std::make_unique<TimerData>();
        }

        timer_data->call_count.fetch_add(1, std::memory_order_relaxed);
        timer_data->total_nanoseconds.fetch_add(nanoseconds, std::memory_order_relaxed);
        
        // Update min/max atomically
        uint64_t current_min = timer_data->min_nanoseconds.load(std::memory_order_relaxed);
        while (nanoseconds < current_min && 
               !timer_data->min_nanoseconds.compare_exchange_weak(current_min, nanoseconds, 
                                                                 std::memory_order_relaxed)) {
            // Retry if CAS failed
        }

        uint64_t current_max = timer_data->max_nanoseconds.load(std::memory_order_relaxed);
        while (nanoseconds > current_max && 
               !timer_data->max_nanoseconds.compare_exchange_weak(current_max, nanoseconds, 
                                                                 std::memory_order_relaxed)) {
            // Retry if CAS failed
        }
    }

    /*!
      \brief Get timer statistics
      \param name timer name
      \return TimerData pointer, nullptr if timer doesn't exist
    */
    const TimerData* getTimerData(const std::string& name) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = timers_.find(name);
        return (it != timers_.end()) ? it->second.get() : nullptr;
    }

    /*!
      \brief Get all timer names
      \return vector of timer names
    */
    std::vector<std::string> getTimerNames() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<std::string> names;
        names.reserve(timers_.size());
        for (const auto& pair : timers_) {
            names.push_back(pair.first);
        }
        return names;
    }

    /*!
      \brief Reset all timers
    */
    void reset() {
        std::lock_guard<std::mutex> lock(mutex_);
        timers_.clear();
    }

    /*!
      \brief Get formatted statistics as string
      \return formatted statistics string
    */
    std::string getStatistics() const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::string result = "Performance Monitor Statistics:\n";
        result += "=====================================\n";
        
        for (const auto& pair : timers_) {
            const auto& name = pair.first;
            const auto& data = pair.second;
            
            uint64_t count = data->call_count.load(std::memory_order_relaxed);
            uint64_t total = data->total_nanoseconds.load(std::memory_order_relaxed);
            uint64_t min_ns = data->min_nanoseconds.load(std::memory_order_relaxed);
            uint64_t max_ns = data->max_nanoseconds.load(std::memory_order_relaxed);
            
            if (count == 0) continue;
            
            double avg_ns = static_cast<double>(total) / count;
            double avg_ms = avg_ns / 1000000.0;
            double min_ms = min_ns / 1000000.0;
            double max_ms = max_ns / 1000000.0;
            
            result += name + ":\n";
            result += "  Calls: " + std::to_string(count) + "\n";
            result += "  Average: " + std::to_string(avg_ms) + " ms\n";
            result += "  Min: " + std::to_string(min_ms) + " ms\n";
            result += "  Max: " + std::to_string(max_ms) + " ms\n";
            result += "  Total: " + std::to_string(total / 1000000.0) + " ms\n\n";
        }
        
        return result;
    }
};

/*!
  \brief Global performance monitor instance
*/
extern PerformanceMonitor g_performance_monitor;

/*!
  \brief Macro for easy performance monitoring
  \param name timer name
*/
#define RCSC_PERF_TIMER(name) \
    auto perf_timer_##name = rcsc::g_performance_monitor.startTimer(#name)

/*!
  \brief Macro for conditional performance monitoring
  \param name timer name
  \param condition condition to enable timing
*/
#define RCSC_PERF_TIMER_IF(name, condition) \
    auto perf_timer_##name = (condition) ? rcsc::g_performance_monitor.startTimer(#name) : \
                                          rcsc::PerformanceMonitor::ScopedTimer(rcsc::g_performance_monitor, "")

} // namespace rcsc

#endif // RCSC_UTIL_PERFORMANCE_MONITOR_H 