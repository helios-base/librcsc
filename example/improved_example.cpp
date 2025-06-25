// -*-c++-*-

/*!
  \file improved_example.cpp
  \brief Example demonstrating improved librcsc functionality
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

/////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <rcsc/common/soccer_agent.h>
#include <rcsc/common/logger.h>
#include <rcsc/gz/gzcompressor.h>
#include <rcsc/util/performance_monitor.h>
#include <rcsc/util/memory_pool.h>
#include <rcsc/geom/vector_2d.h>
#include <rcsc/geom/angle_deg.h>

#include <iostream>
#include <thread>
#include <vector>
#include <memory>
#include <chrono>

using namespace rcsc;

/*!
  \class ImprovedPlayer
  \brief Example player demonstrating improved librcsc features
*/
class ImprovedPlayer : public SoccerAgent {
private:
    // Performance monitoring
    PerformanceMonitor::ScopedTimer* perf_timer_;
    
    // Memory pool for efficient allocation
    ObjectPool<Vector2D> position_pool_;
    
    // Thread-safe data
    std::mutex data_mutex_;
    std::vector<Vector2D> recent_positions_;

public:
    ImprovedPlayer() 
        : SoccerAgent(),
          perf_timer_(nullptr),
          position_pool_(global_pools::double_pool) {
        
        // Enable performance monitoring
        g_performance_monitor.setEnabled(true);
        
        std::cout << "ImprovedPlayer initialized with enhanced features" << std::endl;
    }
    
    ~ImprovedPlayer() {
        if (perf_timer_) {
            delete perf_timer_;
        }
    }

    bool initImpl(CmdLineParser& cmd_parser) override {
        // Start performance monitoring for initialization
        RCSC_PERF_TIMER(player_init);
        
        std::cout << "Initializing improved player..." << std::endl;
        
        // Example of using memory pool
        if (auto pos = ObjectPool<Vector2D>(global_pools::double_pool)) {
            *pos = Vector2D(0.0, 0.0);
            std::cout << "Allocated position from memory pool: " << pos->x << ", " << pos->y << std::endl;
        }
        
        return true;
    }

    void actionImpl() override {
        // Performance monitoring for action execution
        RCSC_PERF_TIMER(player_action);
        
        // Thread-safe data access
        {
            std::lock_guard<std::mutex> lock(data_mutex_);
            if (world().self().posValid()) {
                recent_positions_.push_back(world().self().pos());
                if (recent_positions_.size() > 10) {
                    recent_positions_.erase(recent_positions_.begin());
                }
            }
        }
        
        // Example of compression usage
        std::string test_data = "This is a test message for compression";
        std::string compressed;
        
        {
            RCSC_PERF_TIMER(compression_test);
            GZCompressor compressor(6);
            int result = compressor.compress(test_data.c_str(), test_data.length(), compressed);
            
            if (result == Z_OK) {
                std::cout << "Compression successful: " << test_data.length() 
                          << " -> " << compressed.length() << " bytes" << std::endl;
            }
        }
        
        // Basic soccer logic
        if (world().self().isKickable()) {
            // Kick the ball
            doKick();
        } else {
            // Move to ball
            doMove();
        }
    }
    
    void finalizeImpl() override {
        // Print performance statistics
        std::cout << "\n=== Performance Statistics ===" << std::endl;
        std::cout << g_performance_monitor.getStatistics() << std::endl;
        
        // Print memory pool statistics
        auto [allocated, capacity] = global_pools::double_pool.getStats();
        std::cout << "Memory Pool Stats: " << allocated << "/" << capacity << " objects allocated" << std::endl;
    }

private:
    void doKick() {
        RCSC_PERF_TIMER(kick_action);
        
        // Simple kick logic
        Vector2D ball_pos = world().ball().pos();
        Vector2D goal_pos(52.5, 0.0); // Opponent goal
        
        Vector2D kick_direction = (goal_pos - ball_pos).normalize();
        double kick_power = 100.0;
        
        // Execute kick
        doKick(kick_power, kick_direction);
    }
    
    void doMove() {
        RCSC_PERF_TIMER(move_action);
        
        // Simple move logic
        Vector2D ball_pos = world().ball().pos();
        Vector2D my_pos = world().self().pos();
        
        Vector2D move_direction = (ball_pos - my_pos).normalize();
        double move_power = 100.0;
        
        // Execute move
        doMove(move_power, move_direction);
    }
};

/*!
  \brief Main function demonstrating improved features
*/
int main(int argc, char* argv[]) {
    std::cout << "=== Librcsc Improved Functionality Demo ===" << std::endl;
    std::cout << "Features demonstrated:" << std::endl;
    std::cout << "1. Thread-safe logging" << std::endl;
    std::cout << "2. Performance monitoring" << std::endl;
    std::cout << "3. Memory pool optimization" << std::endl;
    std::cout << "4. Improved compression with error handling" << std::endl;
    std::cout << "5. Modern C++17 features" << std::endl;
    std::cout << "===========================================" << std::endl;
    
    // Test thread-safe logging
    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([i]() {
            for (int j = 0; j < 10; ++j) {
                dlog.addText(Logger::WORLD, "Thread %d: Message %d", i, j);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Test compression with error handling
    try {
        GZCompressor compressor(6);
        std::string test_data = "This is a test message for the improved compression system";
        std::string compressed, decompressed;
        
        int compress_result = compressor.compress(test_data.c_str(), test_data.length(), compressed);
        if (compress_result == Z_OK) {
            std::cout << "Compression successful!" << std::endl;
            
            GZDecompressor decompressor;
            int decompress_result = decompressor.decompress(compressed.c_str(), compressed.length(), decompressed);
            if (decompress_result == Z_OK && decompressed == test_data) {
                std::cout << "Decompression successful! Data integrity verified." << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Compression error: " << e.what() << std::endl;
    }
    
    // Test memory pool
    std::cout << "\nTesting memory pool..." << std::endl;
    {
        auto obj1 = ObjectPool<int>(global_pools::int_pool);
        auto obj2 = ObjectPool<int>(global_pools::int_pool);
        
        if (obj1 && obj2) {
            *obj1 = 42;
            *obj2 = 100;
            std::cout << "Memory pool objects: " << *obj1 << ", " << *obj2 << std::endl;
        }
    }
    
    // Create and run improved player
    ImprovedPlayer player;
    
    // Simulate some game cycles
    for (int cycle = 0; cycle < 10; ++cycle) {
        std::cout << "Game cycle " << cycle << std::endl;
        player.actionImpl();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    player.finalizeImpl();
    
    std::cout << "\n=== Demo completed successfully ===" << std::endl;
    return 0;
} 