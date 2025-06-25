// -*-c++-*-

/*!
  \file memory_pool.h
  \brief Memory pool utility for efficient allocation
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

#ifndef RCSC_UTIL_MEMORY_POOL_H
#define RCSC_UTIL_MEMORY_POOL_H

#include <vector>
#include <mutex>
#include <memory>
#include <cstddef>
#include <cstdint>

namespace rcsc {

/*!
  \class MemoryPool
  \brief Efficient memory pool for fixed-size allocations
*/
template<typename T, size_t BlockSize = 1024>
class MemoryPool {
private:
    struct Block {
        std::unique_ptr<T[]> data;
        std::vector<bool> used;
        size_t free_count;
        
        Block() : data(std::make_unique<T[]>(BlockSize)), 
                  used(BlockSize, false), 
                  free_count(BlockSize) {}
    };

    mutable std::mutex mutex_;
    std::vector<std::unique_ptr<Block>> blocks_;
    size_t total_allocated_;
    size_t total_capacity_;

public:
    MemoryPool() : total_allocated_(0), total_capacity_(0) {}
    
    ~MemoryPool() = default;
    
    // Disable copy
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;
    
    // Allow move
    MemoryPool(MemoryPool&&) = default;
    MemoryPool& operator=(MemoryPool&&) = default;

    /*!
      \brief Allocate a single object
      \return pointer to allocated object, nullptr if allocation failed
    */
    T* allocate() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Try to find a free slot in existing blocks
        for (auto& block : blocks_) {
            if (block->free_count > 0) {
                for (size_t i = 0; i < BlockSize; ++i) {
                    if (!block->used[i]) {
                        block->used[i] = true;
                        --block->free_count;
                        ++total_allocated_;
                        return &block->data[i];
                    }
                }
            }
        }
        
        // Create new block if needed
        try {
            auto new_block = std::make_unique<Block>();
            new_block->used[0] = true;
            new_block->free_count = BlockSize - 1;
            blocks_.push_back(std::move(new_block));
            total_allocated_++;
            total_capacity_ += BlockSize;
            return &blocks_.back()->data[0];
        } catch (const std::bad_alloc&) {
            return nullptr;
        }
    }

    /*!
      \brief Deallocate an object
      \param ptr pointer to object to deallocate
    */
    void deallocate(T* ptr) {
        if (!ptr) return;
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        for (auto& block : blocks_) {
            if (ptr >= &block->data[0] && ptr <= &block->data[BlockSize - 1]) {
                size_t index = ptr - &block->data[0];
                if (index < BlockSize && block->used[index]) {
                    block->used[index] = false;
                    ++block->free_count;
                    --total_allocated_;
                    return;
                }
            }
        }
    }

    /*!
      \brief Get statistics about the memory pool
      \return pair of (allocated_count, total_capacity)
    */
    std::pair<size_t, size_t> getStats() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return {total_allocated_, total_capacity_};
    }

    /*!
      \brief Clear all allocated memory
    */
    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        blocks_.clear();
        total_allocated_ = 0;
        total_capacity_ = 0;
    }
};

/*!
  \class ObjectPool
  \brief RAII wrapper for memory pool allocations
*/
template<typename T, size_t BlockSize = 1024>
class ObjectPool {
private:
    MemoryPool<T, BlockSize>& pool_;
    T* ptr_;

public:
    ObjectPool(MemoryPool<T, BlockSize>& pool) : pool_(pool), ptr_(pool.allocate()) {}
    
    ~ObjectPool() {
        if (ptr_) {
            pool_.deallocate(ptr_);
        }
    }
    
    // Disable copy
    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;
    
    // Allow move
    ObjectPool(ObjectPool&& other) noexcept : pool_(other.pool_), ptr_(other.ptr_) {
        other.ptr_ = nullptr;
    }
    
    ObjectPool& operator=(ObjectPool&& other) noexcept {
        if (this != &other) {
            if (ptr_) {
                pool_.deallocate(ptr_);
            }
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }
        return *this;
    }

    /*!
      \brief Get the allocated object
      \return pointer to object, nullptr if allocation failed
    */
    T* get() const { return ptr_; }
    
    /*!
      \brief Check if allocation was successful
      \return true if allocation succeeded
    */
    explicit operator bool() const { return ptr_ != nullptr; }
    
    /*!
      \brief Dereference operator
      \return reference to allocated object
    */
    T& operator*() { return *ptr_; }
    
    /*!
      \brief Arrow operator
      \return pointer to allocated object
    */
    T* operator->() { return ptr_; }
};

/*!
  \brief Global memory pools for common types
*/
namespace global_pools {
    extern MemoryPool<char, 4096> string_pool;
    extern MemoryPool<int, 1024> int_pool;
    extern MemoryPool<double, 1024> double_pool;
}

} // namespace rcsc

#endif // RCSC_UTIL_MEMORY_POOL_H 